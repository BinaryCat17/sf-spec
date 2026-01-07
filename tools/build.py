#!/usr/bin/env python3
import os
import subprocess
import sys
import argparse

# Configuration
MODULES = ["sf-spec", "sf-compiler", "sf-backend-cpu", "sf-runtime", "sf-samples"]
PRESET = "x64-linux"

# Dependency Graph: module -> list of modules it depends on
DEPENDENCIES = {
    "sf-spec": [],
    "sf-compiler": ["sf-spec"],
    "sf-backend-cpu": ["sf-spec"],
    "sf-runtime": ["sf-spec"],
    "sf-samples": ["sf-spec", "sf-compiler", "sf-backend-cpu", "sf-runtime"]
}

# Key files to 'touch' to force a relink/rebuild in dependent modules
KEY_FILES = {
    "sf-spec": "base/src/sf_platform.c",
    "sf-compiler": "compiler/src/sf_compiler.c",
    "sf-backend-cpu": "backend_cpu/src/sf_backend_cpu.c",
    "sf-runtime": "engine/src/sf_engine.c",
    "sf-samples": "apps/sf-runner/src/main.c"
}

def get_root():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    return os.path.abspath(os.path.join(script_dir, "..", ".."))

def run_command(cmd, cwd):
    print(f"Executing: {" ".join(cmd)} in {cwd}")
    output = []
    try:
        process = subprocess.Popen(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        for line in process.stdout:
            print(line, end="")
            output.append(line)
        process.wait()
        return process.returncode, "".join(output)
    except KeyboardInterrupt:
        print("\nBuild interrupted by user.")
        sys.exit(1)

def main():
    parser = argparse.ArgumentParser(description="SionFlow Workspace Build Orchestrator")
    parser.add_argument("module", nargs="?", help="Specific module to build")
    parser.add_argument("--config", action="store_true", help="Force re-configure")
    parser.add_argument("--clean", action="store_true", help="Remove build directory")
    args = parser.parse_args()

    root = get_root()
    build_root = os.environ.get("SIONFLOW_BUILD_ROOT")
    
    if not build_root:
        print("Error: SIONFLOW_BUILD_ROOT environment variable is not set.")
        sys.exit(1)

    dirty_modules = set()
    if args.module:
        target_modules = [args.module]
    else:
        target_modules = MODULES

    for mod in target_modules:
        mod_path = os.path.join(root, mod)
        binary_dir = os.path.join(build_root, mod, PRESET)
        
        if not os.path.exists(mod_path):
            continue

        # If a dependency was rebuilt, we should force a rebuild here
        is_dependency_dirty = False
        for dep in DEPENDENCIES.get(mod, []):
            if dep in dirty_modules:
                is_dependency_dirty = True
                break
        
        if is_dependency_dirty:
            key_file = os.path.join(mod_path, KEY_FILES.get(mod, "CMakeLists.txt"))
            if os.path.exists(key_file):
                # print(f"Dependency changed, touching {key_file} to force rebuild...")
                os.utime(key_file, None)

        print(f"\n>>> Processing: {mod}")

        if args.clean and os.path.exists(binary_dir):
            import shutil
            shutil.rmtree(binary_dir)

        if args.config or not os.path.exists(os.path.join(binary_dir, "build.ninja")):
            ret, _ = run_command(["cmake", "--preset", PRESET], mod_path)
            if ret != 0: sys.exit(1)

        # Run build and capture output to see if it actually did something
        ret, output = run_command(["cmake", "--build", "--preset", PRESET], mod_path)
        if ret != 0: sys.exit(1)

        # If Ninja did anything more than just saying "no work to do", mark as dirty
        if "ninja: no work to do" not in output and "Linking" in output:
            print(f"Module {mod} was rebuilt.")
            dirty_modules.add(mod)

    print("\nWorkspace Build complete!")

if __name__ == "__main__":
    main()