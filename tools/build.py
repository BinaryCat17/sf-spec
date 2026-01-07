#!/usr/bin/env python3
import os
import subprocess
import sys
import argparse

# Configuration
PRESET = "x64-linux"

def get_root():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    return os.path.abspath(os.path.join(script_dir, "..", ".."))

def run_command(cmd, cwd):
    print(f"Executing: {" ".join(cmd)} in {cwd}")
    try:
        process = subprocess.Popen(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        for line in process.stdout:
            print(line, end="")
        process.wait()
        return process.returncode
    except KeyboardInterrupt:
        print("\nBuild interrupted by user.")
        sys.exit(1)

def main():
    parser = argparse.ArgumentParser(description="SionFlow Workspace Build Orchestrator")
    parser.add_argument("--config", action="store_true", help="Force re-configure")
    parser.add_argument("--clean", action="store_true", help="Remove build directory")
    args = parser.parse_args()

    root = get_root()
    build_root = os.environ.get("SIONFLOW_BUILD_ROOT")
    
    if not build_root:
        print("Error: SIONFLOW_BUILD_ROOT environment variable is not set.")
        sys.exit(1)

    spec_path = os.path.join(root, "sf-spec")
    binary_dir = os.path.join(build_root, "workspace", PRESET)

    if args.clean and os.path.exists(binary_dir):
        import shutil
        shutil.rmtree(binary_dir)

    print(f"\n>>> Building SionFlow Workspace")

    if args.config or not os.path.exists(os.path.join(binary_dir, "build.ninja")):
        # We run CMake in sf-spec but enable SF_WORKSPACE to pull everything else
        cmake_cmd = [
            "cmake", "--preset", PRESET,
            "-B", binary_dir,
            "-S", spec_path,
            "-DSF_WORKSPACE=ON",
            "-DVCPKG_MANIFEST_FEATURES=full"
        ]
        ret = run_command(cmake_cmd, spec_path)
        if ret != 0: sys.exit(1)

    # Build everything
    ret = run_command(["cmake", "--build", binary_dir], binary_dir)
    if ret != 0: sys.exit(1)

    print("\nWorkspace Build complete!")

if __name__ == "__main__":
    main()
