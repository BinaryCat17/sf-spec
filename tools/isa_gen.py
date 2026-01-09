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
    parser.add_argument("--layout", help="Path to binary layout spec (e.g. binary_layout.json)")
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

    # 4.5 Load Layout (Optional)
    layout_data = {}
    if args.layout:
        with open(args.layout, "r") as f:
            layout_data = json.load(f)
            
    # 5. Prepare Context
    meta_by_opcode = {}
    nodes_by_id = {n["id"]: n for n in isa_data["nodes"]}
    
    # Pre-calculate type masks as bitfields
    dtype_to_id = {name.upper(): data["id"] for name, data in isa_data["dtypes"].items()}
    mask_bitfields = {}
    for mask_name, dtypes in isa_data["type_masks"].items():
        bitfield = 0
        for dt in dtypes:
            bitfield |= (1 << dtype_to_id[dt.upper()])
        mask_bitfields[mask_name] = bitfield

    for node in isa_data["nodes"]:
        opcode_name = node.get("opcode", "NOOP")
        is_virtual = "opcode" not in node
        
        # Resources (INPUT/OUTPUT etc) are technically virtual as they have no runtime bytecode
        if node["id"] in ["INPUT", "OUTPUT", "CONST", "CALL"]:
            is_virtual = True
            opcode_name = "NOOP"
        
        ports = [p["name"] for p in node.get("inputs", [])]
        arity = len(ports)
        while len(ports) < 4:
            ports.append(None)
            
        # Calculate input mask for the node (usually from first port or 'all')
        input_mask = 0xFFFF
        if node.get("inputs"):
            mask_name = node["inputs"][0].get("mask", "all")
            input_mask = mask_bitfields.get(mask_name, 0xFFFF)

        meta = {
            "id": node["id"],
            "name": node.get("name", node["id"]),
            "opcode": opcode_name,
            "ports": ports,
            "arity": arity,
            "category": node.get("category", "atomic"),
            "strategy": node.get("strategy", "default"),
            "shape_rule": node.get("shape_rule", "broadcast"),
            "type_rule": node.get("type_rule", "same_as_input"),
            "access": node.get("access", "linear"),
            "input_mask": input_mask,
            "flags": node.get("flags", []),
            "assertions": node.get("assertions", [])
        }

        # meta_by_opcode should only contain entries for actual bytecode instructions
        if not is_virtual:
            meta_by_opcode[opcode_name] = meta
        
        # Add a special field to node for templates to check
        node["is_virtual"] = is_virtual
        node["effective_opcode"] = opcode_name

    context = {
        "isa": isa_data["isa"],
        "opcodes": isa_data["opcodes"],
        "nodes": isa_data["nodes"],
        "dtypes": isa_data["dtypes"],
        "type_masks": isa_data["type_masks"],
        "mask_bitfields": mask_bitfields,
        "constants": isa_data["constants"],
        "nodes_by_id": nodes_by_id,
        "meta_by_opcode": meta_by_opcode,
        "implementations": backend_data.get("implementations", {}),
        "compiler": compiler_data,
        "manifest": manifest_data.get("manifest", {}),
        "layout": layout_data
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