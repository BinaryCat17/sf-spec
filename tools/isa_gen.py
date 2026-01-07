import json
import os
import sys
import argparse
from jinja2 import Environment, FileSystemLoader

def validate_json(data, schema_path):
    try:
        import jsonschema
        with open(schema_path, "r") as f:
            schema = json.load(f)
        jsonschema.validate(instance=data, schema=schema)
    except ImportError:
        print(f"Warning: jsonschema not found. Skipping validation for {schema_path}")
    except Exception as e:
        print(f"Error: Validation failed for {schema_path}")
        print(f"Details: {str(e)}")
        sys.exit(1)

def main():
    parser = argparse.ArgumentParser(description="SionFlow ISA Code Generator")
    parser.add_argument("--isa", required=True, help="Path to isa.json")
    parser.add_argument("--backend", help="Path to optional backend spec (e.g. cpu_spec.json)")
    parser.add_argument("--compiler", help="Path to optional compiler spec (e.g. compiler_spec.json)")
    parser.add_argument("--manifest", help="Path to optional manifest spec (e.g. manifest.json)")
    parser.add_argument("--render", nargs="+", help="Pairs of 'template:output'")
    
    args = parser.parse_args()
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    schema_dir = os.path.join(script_dir, "metadata", "schema")

    # 1. Load ISA
    with open(args.isa, "r") as f:
        isa_data = json.load(f)
    validate_json(isa_data, os.path.join(schema_dir, "isa_schema.json"))
        
    # 2. Load Backend (Optional)
    backend_data = {}
    if args.backend:
        with open(args.backend, "r") as f:
            backend_data = json.load(f)
        validate_json(backend_data, os.path.join(schema_dir, "backend_schema.json"))

    # 3. Load Compiler (Optional)
    compiler_data = {}
    if args.compiler:
        with open(args.compiler, "r") as f:
            compiler_data = json.load(f)
        validate_json(compiler_data, os.path.join(schema_dir, "compiler_schema.json"))

    # 4. Load Manifest (Optional)
    manifest_data = {}
    if args.manifest:
        with open(args.manifest, "r") as f:
            manifest_data = json.load(f)
            
    # 5. Prepare Context
    meta_by_opcode = {}
    for node in isa_data["nodes"]:
        opcode = node["opcode"]
        if opcode == "NOOP": continue # Skip internal nodes without real opcodes
        
        if opcode not in meta_by_opcode:
            ports = [p["name"] for p in node.get("inputs", [])]
            while len(ports) < 4:
                ports.append(None)
            
            meta_by_opcode[opcode] = {
                "ports": ports,
                "category": node.get("category", "atomic"),
                "strategy": node.get("strategy", "default"),
                "shape_rule": node.get("shape_rule", "broadcast"),
                "type_rule": node.get("type_rule", "same_as_input"),
                "access": node.get("access", "linear")
            }

    context = {
        "opcodes": isa_data["opcodes"],
        "nodes": isa_data["nodes"],
        "dtypes": isa_data["dtypes"],
        "type_masks": isa_data["type_masks"],
        "constants": isa_data["constants"],
        "nodes_by_id": {n["id"]: n for n in isa_data["nodes"]},
        "meta_by_opcode": meta_by_opcode,
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