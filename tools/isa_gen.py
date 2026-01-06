import json
import os
import sys

# Default paths relative to script location
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
DEFAULT_ISA_JSON = os.path.join(SCRIPT_DIR, "..", "isa", "isa.json")
DEFAULT_OUTPUT_INC = os.path.join(SCRIPT_DIR, "..", "generated", "include", "sionflow", "isa", "sf_ops_db.inc")

def generate_ops_db(data, output_path):
    S = chr(92)
    lines = []
    lines.append("#ifndef SF_OPS_DB_INC")
    lines.append("#define SF_OPS_DB_INC")
    lines.append("")
    lines.append("/**")
    lines.append(" * Automatically generated from isa.json. DO NOT EDIT.")
    lines.append(" */")
    lines.append("")
    
    # 1. Opcode List
    lines.append("/**")
    lines.append(" * SionFlow Operation Opcodes (The VM Instructions)")
    lines.append(" */")
    lines.append(f"#define SF_OPCODE_LIST {S}")
    
    opcodes = sorted(data["opcodes"].items(), key=lambda x: x[1])
    for i, (name, val) in enumerate(opcodes):
        entry = f"    SF_OPCODE({name}, {val})"
        if i < len(opcodes) - 1:
            entry += f" {S}"
        lines.append(entry)
    lines.append("")

    # 2. Node List (Compiler)
    lines.append(f"#define SF_OP_LIST {S}")
    
    nodes = data["nodes"]
    for i, node in enumerate(nodes):
        suffix = node["id"]
        json_name = node["name"]
        opcode = node["opcode"].upper()
        category = f"SF_OP_CAT_{node['category'].upper()}"
        strategy = f"SF_STRATEGY_{node.get('strategy', 'default').upper()}"
        
        in_mask = f"SF_TYPE_MASK_{node.get('input_mask', 'all').upper()}" 
        if "inputs" in node and node["inputs"]:
            in_mask = f"SF_TYPE_MASK_{node['inputs'][0]['mask'].upper()}"
        
        out_mask = in_mask
        if "output_mask" in node:
            out_mask = f"SF_TYPE_MASK_{node['output_mask'].upper()}"
        elif "outputs" in node and node["outputs"]:
             out_mask = f"SF_TYPE_MASK_{node['outputs'][0].get('mask', 'all').upper()}"

        type_rule = f"SF_OUT_{node['type_rule'].upper()}"
        shape_rule = f"SF_SHAPE_{node['shape_rule'].upper()}"
        access_rule = f"SF_ACCESS_{node.get('access', 'linear').upper()}"
        
        ports = [f'"{p["name"]}"' for p in node.get("inputs", [])]
        while len(ports) < 4:
            ports.append("NULL")
            
        arity = len(node.get("inputs", []))
        
        impls = node.get("implementations", {})
        c11_impl = impls.get("c11", {})
        ktype = "AUTO" if "expression" in c11_impl else "MANUAL"
        kexpr = c11_impl.get("expression", "NULL")
            
        entry = f'    SF_OP({suffix: <12}, "{json_name}", {opcode: <8}, {category: <18}, {strategy: <22}, {in_mask: <20}, {out_mask: <20}, {type_rule: <22}, {shape_rule: <18}, {access_rule: <18}, {ports[0]: <8}, {ports[1]: <8}, {ports[2]: <8}, {ports[3]: <8}, {ktype: <6}, {kexpr}, {arity})'
        if i < len(nodes) - 1:
            entry += f" {S}"
        lines.append(entry)

    lines.append("")
    lines.append("#endif // SF_OPS_DB_INC")
    
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, "w") as f:
        f.write("\n".join(lines))

def main():
    isa_path = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_ISA_JSON
    out_path = sys.argv[2] if len(sys.argv) > 2 else DEFAULT_OUTPUT_INC
    
    with open(isa_path, "r") as f:
        data = json.load(f)
        
    generate_ops_db(data, out_path)
    print(f"Successfully generated {out_path} from {isa_path}")

if __name__ == "__main__":
    main()
