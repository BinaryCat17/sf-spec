# App Creation Guide

This guide walks you through creating a SionFlow application, from defining logic in a JSON graph to running it as a binary cartridge.

---

## 1. Defining Logic (The Graph)

Logic in SionFlow is defined as a directed acyclic graph (DAG). Each node represents an operation.

**Example: `logic.json` (Calculate `(a + b) * 2`)**
```json
{
  "nodes": [
    { "id": "in_A", "type": "Input" },
    { "id": "in_B", "type": "Input" },
    { "id": "two",  "type": "Const", "value": 2.0 },
    { 
      "id": "sum", "type": "Add", 
      "inputs": { "a": "in_A", "b": "in_B" } 
    },
    { 
      "id": "out_Result", "type": "Mul", 
      "inputs": { "a": "sum", "b": "two" },
      "flags": ["Output"]
    }
  ]
}
```

### Key Rules:
*   **Input Ports:** Use standard names like `a`, `b`, `c`, `x`, `in`. Refer to `sf-spec/tools/metadata/isa.json` for the exact port names and arity of each opcode.
*   **Formatting:** You can use `sf-spec/tools/format_json.py` to ensure your JSON files follow the canonical format.
*   **Output Nodes:** Mark at least one node with `"flags": ["Output"]` or use the dedicated `Output` node type to make data accessible to the host.
*   **DTypes:** By default, nodes use `F32`. You can specify `"dtype": "I32"` or `"U8"` for integer/mask operations.

---

## 2. Creating the Manifest (.mfapp)

A manifest bundles multiple kernels and defines global resources (memory) and application settings.

**Example: `app.mfapp`**
```json
{
  "window": {
    "title": "My Logic App",
    "width": 1,
    "height": 1
  },
  "pipeline": {
    "resources": [
      { "name": "in_A", "dtype": "F32", "shape": [1], "data": [10.0] },
      { "name": "in_B", "dtype": "F32", "shape": [1], "data": [5.0] },
      { "name": "out_Result", "dtype": "F32", "shape": [1], "flags": ["Output"] }
    ],
    "kernels": [
      {
        "id": "main_logic",
        "entry": "logic.json",
        "bindings": [
          { "port": "in_A", "resource": "in_A" },
          { "port": "in_B", "resource": "in_B" },
          { "port": "out_Result", "resource": "out_Result" }
        ]
      }
    ]
  }
}
```

---

## 3. Compiling to a Cartridge

Use the `sfc` tool to bake everything into a single binary file. This step performs validation, optimization, and resource allocation.

```bash
# Compile the manifest and all referenced JSONs into a .sfc cartridge
# If not in PATH, find 'sfc' in your workspace build directory (sf-compiler/sfc/sfc)
sfc app.mfapp my_app.sfc
```

The output `my_app.sfc` is a self-contained binary containing the bytecode, resource templates, and embedded assets.

---

## 4. Running the Application

### Headless Mode (For testing and computation)
```bash
sf-runner my_app.sfc --frames 1
```

### GUI Mode (For interactive apps)
If your app produces an output compatible with pixels (like `out_Color` [H, W, 4]), you can run it in a window:
```bash
sf-window my_app.sfc
```

---

## Tips for Success

1.  **Check Diagnostic Output:** The compiler provides precise error messages including line/column numbers if a graph is invalid.
2.  **Implicit Domains:** By default, an operation's domain is the shape of its first output. Use `domain_node_idx` for manual control.
3.  **Broadcasting:** SionFlow follows formal broadcasting rules. Scalars and unit-dimensions expand automatically to match larger tensors.
4.  **Buffer Aliasing:** Don't be afraid to create many temporary nodes; the compiler's liveness analysis will minimize the actual memory footprint by reusing registers.
