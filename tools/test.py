#!/usr/bin/env python3
import subprocess
import sys
import os
import argparse
import tempfile
import json
import re
import time
from pathlib import Path

def find_tools(build_root):
    if not build_root:
        return None, None
    sfc = os.path.join(build_root, "workspace/x64-linux/sf-compiler/sfc/sfc")
    runner = os.path.join(build_root, "workspace/x64-linux/sf-samples/apps/sf-runner/sf-runner")
    return sfc if os.path.exists(sfc) else None, runner if os.path.exists(runner) else None

def parse_output_values(output):
    pattern = re.compile(r"'(\w+)'\s+Shape:.*?:\s+{(.*?)}")
    results = {}
    final_state_parts = output.split("--- Final State ---")
    if len(final_state_parts) < 2: return results
    
    for match in pattern.finditer(final_state_parts[-1]):
        name = match.group(1)
        vals_str = match.group(2).replace(" ", "")
        try:
            results[name] = [float(v) for v in vals_str.split(",") if v]
        except ValueError: continue
    return results

def run_single_test(sfc, runner, input_path, expect_fail=False, timeout=5):
    frames = 1
    expectations = {}
    if input_path.endswith(".mfapp"):
        try:
            with open(input_path, 'r') as f:
                config = json.load(f)
                test_cfg = config.get("test", {})
                frames = test_cfg.get("frames", 1)
                expectations = test_cfg.get("expectations", {})
        except: pass

    with tempfile.NamedTemporaryFile(suffix=".sfc", delete=False) as tmp:
        sfc_path = tmp.name

    try:
        # 1. Compile
        ret = subprocess.run([sfc, input_path, sfc_path], capture_output=True, text=True, timeout=timeout)
        if ret.returncode != 0:
            return (expect_fail, ret.stdout + ret.stderr)
        
        # 2. Run
        ret = subprocess.run([runner, sfc_path, "--headless", "--frames", str(frames)], capture_output=True, text=True, timeout=timeout)
        if ret.returncode != 0:
            return (expect_fail, ret.stdout + ret.stderr)
        
        if expect_fail:
            return (False, "Test was expected to fail but succeeded.")

        # 3. Verify
        if expectations:
            actual = parse_output_values(ret.stdout)
            for name, expected_vals in expectations.items():
                if name not in actual: return (False, f"Resource '{name}' not found")
                if len(actual[name]) != len(expected_vals): return (False, f"Size mismatch for {name}")
                for i in range(len(expected_vals)):
                    if abs(actual[name][i] - expected_vals[i]) > 0.001:
                        return (False, f"Value mismatch at {name}[{i}]: expected {expected_vals[i]}, got {actual[name][i]}")
        
        return (True, "Passed")
    except subprocess.TimeoutExpired:
        return (False, "Timed out")
    finally:
        if os.path.exists(sfc_path): os.remove(sfc_path)

def main():
    parser = argparse.ArgumentParser(description="SionFlow Unified Test Tool")
    parser.add_argument("input", nargs="?", help="Path to a single test (.json or .mfapp)")
    parser.add_argument("--build-root", default=os.environ.get("SIONFLOW_BUILD_ROOT"), help="Path to build root")
    parser.add_argument("--dir", default="sf-samples/tests", help="Directory to crawl for tests")
    parser.add_argument("--timeout", type=int, default=5, help="Timeout per test")
    parser.add_argument("--expect-fail", action="store_true", help="Expect the single test to fail")
    args = parser.parse_args()

    sfc, runner = find_tools(args.build_root)
    if not sfc or not runner:
        print(f"Error: Tools not found in {args.build_root}")
        sys.exit(1)

    if args.input:
        success, msg = run_single_test(sfc, runner, args.input, args.expect_fail, args.timeout)
        print(msg)
        sys.exit(0 if success else 1)

    # Crawl mode
    all_mfapps = list(Path(args.dir).rglob("*.mfapp"))
    test_files = [str(p) for p in all_mfapps]
    
    # Only add .json files if they are not in a directory containing an .mfapp
    # and not in a subdirectory of such a directory.
    mfapp_dirs = [str(p.parent) for p in all_mfapps]
    
    for path in Path(args.dir).rglob("*.json"):
        path_str = str(path)
        is_part_of_mfapp = False
        for md in mfapp_dirs:
            if path_str.startswith(md):
                is_part_of_mfapp = True
                break
        
        if not is_part_of_mfapp:
            test_files.append(path_str)
    
    test_files.sort()
    
    import concurrent.futures
    results = {"pass": 0, "fail": 0}
    failed_details = []

    # Prepare temp directory in build root
    tmp_dir = os.path.join(args.build_root, "tmp_test") if args.build_root else tempfile.gettempdir()
    os.makedirs(tmp_dir, exist_ok=True)

    # Color support check
    use_color = sys.stdout.isatty() and os.environ.get("TERM") != "dumb" and os.environ.get("NO_COLOR") is None
    C_PASS = "\033[92mPASS\033[0m" if use_color else "PASS"
    C_FAIL = "\033[91mFAIL\033[0m" if use_color else "FAIL"

    print(f"Found {len(test_files)} tests. Running in parallel...\n")
    
    def task(test_path):
        rel = os.path.relpath(test_path, args.dir)
        is_neg = "negative" in test_path or os.path.basename(test_path).startswith("fail_")
        
        # We need a unique temp file for each parallel task
        fd, sfc_path = tempfile.mkstemp(suffix=".sfc", dir=tmp_dir)
        os.close(fd)

        try:
            ok, msg = run_single_test_custom(sfc, runner, test_path, sfc_path, is_neg, args.timeout)
            return rel, ok, msg
        finally:
            if os.path.exists(sfc_path): os.remove(sfc_path)

    # Refactor run_single_test to accept sfc_path
    def run_single_test_custom(sfc, runner, input_path, sfc_path, expect_fail=False, timeout=5):
        frames = 1
        expectations = {}
        if input_path.endswith(".mfapp"):
            try:
                with open(input_path, 'r') as f:
                    config = json.load(f)
                    test_cfg = config.get("test", {})
                    frames = test_cfg.get("frames", 1)
                    expectations = test_cfg.get("expectations", {})
            except: pass

        cmd_compile = [sfc, input_path, sfc_path]
        cmd_run = [runner, sfc_path, "--headless", "--frames", str(frames)]

        try:
            # 1. Compile
            ret = subprocess.run(cmd_compile, capture_output=True, text=True, timeout=timeout)
            if ret.returncode != 0:
                return (expect_fail, f"Compilation Failed\nCommand: {' '.join(cmd_compile)}\n\n{ret.stdout}{ret.stderr}")
            
            # 2. Run
            try:
                ret = subprocess.run(cmd_run, capture_output=True, text=True, timeout=timeout)
                if ret.returncode != 0:
                    return (expect_fail, f"Runtime Failed (Exit {ret.returncode})\nCommand: {' '.join(cmd_run)}\n\n{ret.stdout}{ret.stderr}")
            except subprocess.TimeoutExpired as e:
                out = (e.stdout.decode() if e.stdout else "") + (e.stderr.decode() if e.stderr else "")
                return (False, f"Runtime Timed Out after {timeout}s\nCommand: {' '.join(cmd_run)}\n\nPartial Output:\n{out}")
            
            if expect_fail:
                return (False, "Test was expected to fail but succeeded.")

            # 3. Verify
            if expectations:
                actual = parse_output_values(ret.stdout)
                for name, expected_vals in expectations.items():
                    if name not in actual: return (False, f"Resource '{name}' not found")
                    if len(actual[name]) != len(expected_vals): return (False, f"Size mismatch for {name}")
                    for i in range(len(expected_vals)):
                        if abs(actual[name][i] - expected_vals[i]) > 0.001:
                            return (False, f"Value mismatch at {name}[{i}]: expected {expected_vals[i]}, got {actual[name][i]}")
            
            return (True, "Passed")
        except subprocess.TimeoutExpired:
            return (False, f"Compilation Timed Out\nCommand: {' '.join(cmd_compile)}")

    with concurrent.futures.ThreadPoolExecutor() as executor:
        futures = [executor.submit(task, tf) for tf in test_files]
        for future in concurrent.futures.as_completed(futures):
            rel, ok, msg = future.result()
            status = C_PASS if ok else C_FAIL
            print(f"[{status}] {rel}")
            if ok:
                results["pass"] += 1
            else:
                results["fail"] += 1
                failed_details.append((rel, msg))

    print(f"\nRESULTS: {results['pass']} Passed, {results['fail']} Failed")
    if failed_details:
        failed_details.sort() # Keep report stable
        for name, err in failed_details: print(f"\n--- {name} ---\n{err}")
        sys.exit(1)

if __name__ == "__main__":
    main()