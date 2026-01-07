# SionFlow

**SionFlow** is a high-performance, data-oriented computation engine designed for real-time applications, games, and UI rendering. It strictly separates the **logic definition** (Node Graphs) from its **execution**, utilizing an optimizing compiler and a specialized Runtime Engine to drive various backends (CPU/GPU).

> **Status:** Phase 10 (Architectural Consolidation) complete. Transitioning to Phase 11 (GPU Readiness).

## Core Principles

*   **Task-Driven Execution:** Logic is segmented into parallelizable tasks with pre-analyzed memory barriers.
*   **The Cartridge Model:** Full applications (kernels, assets, pipeline) are baked into single binary `.sfc` files.
*   **Zero-Overhead Dispatch:** Pre-calculated strides and data views minimize runtime logic in the hot execution loop.
*   **Spec-First Design:** The ISA and compiler rules are driven by declarative metadata (`isa.json`), auto-generating kernels and validation logic.

## Ecosystem Structure

SionFlow is composed of several independent but strictly synchronized modules:

*   **sf-spec:** The Foundation. Contains the ISA, base math/memory utilities, and development tooling.
*   **sf-compiler:** The Brain. Translates high-level JSON graphs into efficient binary bytecode. Includes the `sfc` utility.
*   **sf-runtime:** The Heart. Manages resources, orchestrates execution, and provides host abstractions (Headless/SDL2).
*   **sf-backend-cpu:** The Muscle. Multithreaded CPU execution engine with auto-generated math kernels.
*   **sf-samples:** The Proof. Test projects, integration tests, and reference applications.

---

## Getting Started

### Prerequisites
*   CMake (3.25+)
*   C11 compatible compiler (GCC 11+, Clang 14+, MSVC 2022)
*   Python 3.x (for code generation and dev tools)
*   [vcpkg](https://github.com/microsoft/vcpkg)

### Build Instructions

SionFlow uses CMake Presets for a unified build experience. Ensure the environment variable `SIONFLOW_BUILD_ROOT` is set to your desired build directory.

**Build Sequence:**
1. `sf-spec`
2. `sf-compiler`
3. `sf-backend-cpu`
4. `sf-runtime`
5. `sf-samples`

**Commands (Example for Linux):**
```bash
cd sf-spec
cmake --preset x64-linux
cmake --build --preset x64-linux
```

---

## Development Workflow

### Auto-Touch Mechanism
SionFlow features an automated development workflow. When building `sf-samples` or other dependent modules, the system automatically detects changes in local module source code (e.g., in `sf-spec`) and triggers a rebuild via vcpkg without requiring manual version increments.

**To enable:** Simply use the `x64-linux` or `x64-windows` CMake presets. The `CMAKE_PROJECT_INCLUDE_BEFORE` hook will handle the rest.

---

## Tools & Usage

### 1. Compile a Cartridge
Use `sfc` to bake a JSON graph or an `.mfapp` manifest into a binary cartridge:
```bash
# Single graph
sfc input.json output.sfc

# Full application manifest
sfc my_app.mfapp my_app.sfc
```

### 2. Run Headless (Testing)
Execute a cartridge for a specific number of frames and see the result:
```bash
sf-runner my_app.sfc --headless --frames 1
```

### 3. Run Visual (GUI)
Launch a cartridge in an interactive window:
```bash
sf-window my_app.sfc
```

---

## Documentation

*   [Architecture](docs/architecture.md) - Deep dive into Execution Plans, Registers, and Strides.
*   [Roadmap](docs/roadmap.md) - Track our progress towards the Vulkan GPU backend.
*   [App Creation Guide](docs/guide_app_creation.md) - Learn how to build nodes and manifests.
