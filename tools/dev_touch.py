import os
import time

# Get the root SionFlow directory (two levels up from sf-spec/tools)
script_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.abspath(os.path.join(script_dir, "..", ".."))

ports = [
    "sf-spec/vcpkg-ports/sf-spec/.vcpkg_touch",
    "sf-compiler/vcpkg-ports/sf-compiler/.vcpkg_touch",
    "sf-runtime/vcpkg-ports/sf-runtime/.vcpkg_touch",
    "sf-backend-cpu/vcpkg-ports/sf-backend-cpu/.vcpkg_touch",
    "sf-samples/vcpkg-ports/sf-samples/.vcpkg_touch"
]

def main():
    timestamp = str(time.time())
    for port_rel_path in ports:
        port_file = os.path.join(root_dir, port_rel_path)
        if os.path.exists(os.path.dirname(port_file)):
            with open(port_file, "w") as f:
                f.write(timestamp)
            print(f"Updated {port_file}")

if __name__ == "__main__":
    main()
