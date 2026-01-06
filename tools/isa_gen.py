import json
import os
import sys
import argparse
from jinja2 import Environment, FileSystemLoader

def main():
    parser = argparse.ArgumentParser(description="SionFlow ISA Code Generator")
    parser.add_argument("--isa", required=True, help="Path to isa.json")
    parser.add_argument("--backend", help="Path to optional backend spec (e.g. cpu_spec.json)")
    parser.add_argument("--compiler", help="Path to optional compiler spec (e.g. compiler_spec.json)")
    parser.add_argument("--manifest", help="Path to optional manifest spec (e.g. manifest.json)")
    parser.add_argument("--render", nargs="+", help="Pairs of 'template:output'")
    
    args = parser.parse_args()
    
    # 1. Load ISA
    with open(args.isa, "r") as f:
        isa_data = json.load(f)
        
    # 2. Load Backend (Optional)
    backend_data = {}
    if args.backend:
        with open(args.backend, "r") as f:
            backend_data = json.load(f)

    # 3. Load Compiler (Optional)
    compiler_data = {}
    if args.compiler:
        with open(args.compiler, "r") as f:
            compiler_data = json.load(f)

    # 4. Load Manifest (Optional)
    manifest_data = {}
    if args.manifest:
        with open(args.manifest, "r") as f:
            manifest_data = json.load(f)
            
    # 5. Prepare Context
    ports_by_opcode = {}
    for node in isa_data["nodes"]:
        opcode = node["opcode"]
        if opcode not in ports_by_opcode:
            ports = [p["name"] for p in node.get("inputs", [])]
            while len(ports) < 4:
                ports.append(None)
            ports_by_opcode[opcode] = ports

    context = {
        "opcodes": isa_data["opcodes"],
        "nodes": isa_data["nodes"],
        "dtypes": isa_data["dtypes"],
        "type_masks": isa_data["type_masks"],
        "nodes_by_id": {n["id"]: n for n in isa_data["nodes"]},
        "ports_by_opcode": ports_by_opcode,
        "implementations": backend_data.get("implementations", {}),
        "compiler": compiler_data,
        "manifest": manifest_data.get("manifest", {})
    }
    
    # 5. Initialize Jinja2
    # We allow templates to be anywhere, so we'll use an absolute path for each
    for pair in args.render:
        tmpl_path, out_path = pair.split(":")
        
        tmpl_dir = os.path.dirname(os.path.abspath(tmpl_path))
        tmpl_name = os.path.basename(tmpl_path)
        
        env = Environment(loader=FileSystemLoader(tmpl_dir))
        template = env.get_template(tmpl_name)
        
        # 5. Render
        output = template.render(**context)
        
        # 6. Save
        os.makedirs(os.path.dirname(os.path.abspath(out_path)), exist_ok=True)
        with open(out_path, "w") as f:
            f.write(output)
            
        print(f"Generated {out_path} from {tmpl_path}")

if __name__ == "__main__":
    main()