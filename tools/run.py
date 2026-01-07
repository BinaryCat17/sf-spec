#!/usr/bin/env python3
import os
import sys
import subprocess
import argparse
import pathlib

def find_tool(build_root, tool_name):
    # Common locations in workspace build
    search_paths = [
        "workspace/x64-linux/sf-compiler/sfc",
        "workspace/x64-linux/sf-samples/apps/sf-runner",
        "workspace/x64-linux/sf-samples/apps/sf-window"
    ]
    for sp in search_paths:
        path = os.path.join(build_root, sp, tool_name)
        if os.path.exists(path) and os.access(path, os.X_OK):
            return path
    
    # Fallback search
    for root, dirs, files in os.walk(build_root):
        if tool_name in files:
            path = os.path.join(root, tool_name)
            if os.access(path, os.X_OK):
                return path
    return None

def main():
    parser = argparse.ArgumentParser(description="SionFlow Run Utility")
    parser.add_argument("input", help="Path to .json graph or .mfapp manifest")
    parser.add_argument("--mode", choices=["headless", "window"], default="headless", help="Execution mode")
    parser.add_argument("--frames", type=int, default=1, help="Number of frames (for headless)")
    parser.add_argument("--build-root", default=os.environ.get("SIONFLOW_BUILD_ROOT"), help="Path to build root")
    
    args, unknown = parser.parse_known_args()

    if not args.build_root:
        print("Error: SIONFLOW_BUILD_ROOT not set and --build-root not provided.")
        sys.exit(1)

    sfc = find_tool(args.build_root, "sfc")
    sf_runner = find_tool(args.build_root, "sf-runner")
    sf_window = find_tool(args.build_root, "sf-window")

    if not sfc:
        print(f"Error: 'sfc' not found in {args.build_root}")
        sys.exit(1)

    # Prepare temp directory in build root
    tmp_dir = os.path.join(args.build_root, "tmp_sfc")
    os.makedirs(tmp_dir, exist_ok=True)
    
    input_path = pathlib.Path(args.input)
    output_sfc = os.path.join(tmp_dir, input_path.stem + ".sfc")

    # 1. Compile
    print(f"-- Compiling {args.input} -> {output_sfc}")
    try:
        subprocess.run([sfc, str(input_path), output_sfc], check=True)
    except subprocess.CalledProcessError:
        print("Error: Compilation failed.")
        sys.exit(1)

    # 2. Run
    if args.mode == "headless":
        if not sf_runner:
            print("Error: 'sf-runner' not found.")
            sys.exit(1)
        print(f"-- Running Headless ({args.frames} frames)")
        cmd = [sf_runner, output_sfc, "--headless", "--frames", str(args.frames)] + unknown
        subprocess.run(cmd)
    else:
        if not sf_window:
            print("Error: 'sf-window' not found.")
            sys.exit(1)
        print("-- Running Windowed")
        cmd = [sf_window, output_sfc] + unknown
        subprocess.run(cmd)

if __name__ == "__main__":
    main()
