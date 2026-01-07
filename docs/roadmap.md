# SionFlow Roadmap v3: The Pixel Engine

**Vision:** To prove that SionFlow is a true Data-Oriented Engine by implementing a UI/Renderer purely as a mathematical function of state.

---

## Current Status: Performance & Reliability Hardening

### Phase 1: Backend Refactoring (Completed)
- [x] **Remove Instruction Baking**
- [x] **Execution Plan (Register Plan)**
- [x] **Builtin ID System**
- [x] **Zero-Guess Dispatch**

### Phase 2: Compiler Hardening & Reliability (Completed)
- [x] **Stride Promotion**
- [x] **Explicit Domain Tracking**
- [x] **Dual Type Masks**
- [x] **Strict Shape Validation**

### Phase 3: Logic Expansion & Infrastructure (Completed)
- [x] **Domain Inheritance in Subgraphs**
- [x] **Index Shape Auto-Inference**
- [x] **Unified Error Context**
- [x] **Centralized Provider Registry**

---

## Active Development: Consolidation & Purity

### Phase 6: Architectural Consolidation (The 'Shrink' Phase) (Completed)
**Goal:** Сокращение объема кода при сохранении функционала. Борьба с дублированием данных и логики.
- [x] **Unified Operation Registry (X-Macros):** Создан `sf_ops_db.inc`. Удалены дубликаты в `isa` и `compiler`.
- [x] **Generic Kernel Patterns:** Унификация циклов через `SF_KERNEL_*`. Удалена ручная регистрация в модулях `ops`.
- [x] **Loader Unification:** Слияние `sf_loader` и `sf_manifest_loader`. Удалены лишние файлы.
- [x] **Deep Tensor Decoupling:** Компилятор и `sf_program` переведены на `sf_type_info`. `sf_tensor` используется только в рантайме.
- [x] **Backend Encapsulation:** Бэкенд изолирован через `sf_task`.

### Phase 7: Quality of Life & Extreme Shrink (Completed)
**Goal:** Стандартизация, подготовка к векторизации и финальное сокращение boilerplate-кода.
- [x] **Extreme Kernel Unification:** 80% ядер (atomic ops) генерируются автоматически через один X-Macro в `sf_ops_db.inc`. Математика (`+`, `-`, `*`, `/`) и функции (`sinf`, `cosf`) перенесены в БД операций.
- [x] **Standardize Opcode Naming:** Убраны алиасы, унифицированы имена портов (`a`, `b`, `c`, `d`, `x`, `cond`, `true`, `false`).
- [x] **Unified Compiler Passes:** `sf_pass_validate`, `sf_pass_analyze` и `sf_codegen` переведены на универсальные циклы по портам. Удалено дублирование логики `s1/s2/s3/s4`.
- [x] **Improved Metadata:** В структуру `sf_op_metadata` добавлена арность (`arity`), что позволило автоматизировать проверку подключений.
- [x] **Documentation Sync:** Обновить `docs/architecture.md` и `docs/guide_app_creation.md` согласно изменениям Фаз 6 и 7.

## Phase 8: Robustness & Compiler Maturity (Hardening) (Completed)
**Goal:** Превращение компилятора в надежный инструмент с вменяемой диагностикой и чистой архитектурой.
- [x] **Total Diagnostic Coverage:** Искоренить все "тихие падения" (`return false` без репорта). Каждый сбой должен иметь текстовое описание и локацию в коде.
- [x] **Metadata Simplification (Wrappers):** Внедрить семантические макросы-обертки (например, `SF_MATH_BIN`) внутри `sf_ops_db.inc`. Это уберет визуальный шум (`NULL, MANUAL`) и исключит ошибки в позиционных аргументах.
- [x] **Rank Normalization Pass:** Внедрить автоматическое приведение рангов. Если операция ожидает скаляр `[]`, а получает вектор `[1]`, компилятор должен выполнять неявный *Squeeze*.
- [x] **Formal Broadcasting Logic:** Довести `sf_shape.c` до полной поддержки правил NumPy (правильное выравнивание осей по правому краю), сохраняя при этом различие между `[]` (rank 0) и `[1]` (rank 1).
- [x] **Contract & Loader Decoupling (The "Clean Split"):**
    - [x] **Autonomous Compiler:** Избавить компилятор от зависимости от `sf_compile_contract`. Он должен компилировать граф на основе типов/форм, указанных внутри самих узлов `Input/Output`.
    - [x] **Dynamic Resolver in Engine:** Перенести логику сопоставления портов ядра с ресурсами манифеста из `sf_loader.c` в `sf_engine`. Движок должен сам находить нужные регистры в `sf_program` по таблице символов.
    - [x] **Pure IO Loader:** Лоадер должен только читать файлы и парсить базовые структуры, не вмешиваясь в логику сопоставления типов и не вызывая компилятор «вручную» с хитрыми параметрами.
- [x] **Zero-Allocation Dispatch:** Перенести аллокации `reduction_scratch` и `sync_scratch` из `dispatch` в `bake`, чтобы рантайм был полностью свободен от `malloc/calloc`.

## Phase 8.5: Backend Decoupling & Brain Shift (Optimization) (Completed)
**Goal:** Перенос высокоуровневой логики из бэкенда в компилятор. Превращение бэкенда в "чистый исполнитель" без знания о конкретных опкодах.
- [x] **Compiler-Driven Segmentation:** Полностью перенести логику разделения на сегменты (барьеры) в кодогенерацию. Бэкенд получает готовый список `sf_task` и не пересчитывает их.
- [x] **Deterministic Scratchpad Sizing:** Компилятор высчитывает требуемый размер `sync_scratch` и `reduction_scratch` на этапе кодогенерации и записывает его в `sf_bin_header`.
- [x] **Dispatch Strategy Metadata:** Хардкод `if (opcode == CUMSUM)` заменен на декларативные стратегии в `sf_op_metadata`.
- [x] **Zero-Allocation Dispatch:** Рантайм полностью свободен от `malloc/calloc` в горячем цикле.
- [x] **Pre-Analyzed Liveness in Binary:** Перенести расчет списка `active_regs` для каждой задачи в компилятор. Бэкенд не должен итерироваться по инструкциям в `bake`.
- [x] **Rich Tensor Flags:** Добавить флаги (`REDUCTION`, `GENERATOR`, `PERSISTENT`) в `sf_bin_tensor_desc`, чтобы убрать анализ зависимостей из бэкенда.
- [x] **Register Binding Optimization:** Упростить `prepare_registers` в бэкенде, избавившись от лишних проверок и `switch`.
- [x] **Instruction Shrink (The 'Thin' VM):** Удалить массив `strides[5]` из `sf_instruction`. Уменьшить размер структуры до ~16 байт для резкого снижения давления на L1 Instruction Cache.
- [x] **Stride Promotion to Task Metadata:** Перенести хранение страйдов в метаданные привязки регистров внутри задачи (`sf_task`).
- [x] **Byte-Stride Pre-calculation:** Компилятор должен сразу вычислять байтовые смещения (`stride * sizeof(dtype)`), избавляя бэкенд от привязки к размерам типов.

## Phase 9: The "Cartridge" Model (Autonomous Packaging) (Completed)
**Goal:** Полная архитектурная изоляция. Превращение `.bin` в самодостаточный "картридж", содержащий настройки хоста, ресурсы и весь вычислительный пайплайн. Рантайм становится "тупым" плеером.

- [x] **Resource Templates in Binary:** Перенесено описание глобальных ресурсов и их флагов (`READONLY`, `PERSISTENT`) прямо в таблицу символов `sf_program`.
- [x] **Unified App Header:** Расширен `sf_bin_header`, добавлены параметры хоста (Window Title, Width, Height, VSync, NumThreads) прямо в файл программы.
- [x] **Zero-JSON Runtime (Partial):** Рантайм теперь может запускаться напрямую из `.bin/.sfc` без использования `.mfapp` или других JSON-файлов.
- [x] **Binary Pipeline Bundling:** Формат умеет хранить несколько программ (логика, физика, рендер) и описание их связей внутри одного файла.
- [x] **Standalone Compiler CLI (`sfc`):** Создана утилита, которая принимает JSON и все связанные ресурсы, проводит полную валидацию и «запекает» их в один финальный картридж.
- [x] **Asset Embedding (The Blob):** Реализована упаковка внешних ассетов (текстуры, шрифтов) прямо в секции бинарного файла для создания 100% автономных приложений.

## Phase 10: The Great Decoupling & Standardization (Completed)
**Goal:** Transform SionFlow into a professional, multi-repo ecosystem with a strict separation between Specification, Tooling, and Runtime.

### 10.1 Specification-First Design (The "Single Source of Truth") (Completed)
- [x] **Decoupled ISA Definition (`isa.json`):** Purge all implementation-specific logic (C expressions) from the core specification.
- [x] **Backend Implementation Specs (`cpu_spec.json`):** Mapping ISA nodes to concrete implementations.
- [x] **Template-Based Code Generation (Jinja2):** Replaced brittle X-Macros with robust Python + Jinja2 templates for Opcodes, Metadata, and Kernels.
- [x] **Compiler Transformation Spec (`compiler_spec.json`):** Move high-level compiler logic into declarative metadata.
- [x] **Data-Driven Validation & Analysis:** Move rank/shape constraints and type promotion rules to schema-validated JSON.
- [x] **Declarative App Manifest:** Define window and runtime settings in `manifest.json`.

### 10.7 Legacy Purge & Structural Refinement (Completed)
- [x] **Data-Driven Optimizer:** Rewrite `sf_pass_fuse.c` to use generated pattern-matching logic.
- [x] **Template-Only Kernels:** Eliminate the `SF_KERNEL_AUTO` macro. Move loop logic entirely into Jinja2 templates.
- [x] **Unified Validation:** Metadata-driven loop in `sf_pass_validate.c`.
- [x] **Zero-Boilerplate Manual Kernels:** Refactor `DOT`, `NORMALIZE` to use consistent naming and prepared structures.
- [x] **Developer Auto-Touch Tooling:** Implemented `dev_touch.py` and `dev_init.cmake` to automate vcpkg rebuilds.

### 10.2 Modular Dependency Management (Completed)
- [x] **vcpkg Integration:** Create official `vcpkg` ports for all modules.
- [x] **Unified buildPresets:** Added `buildPresets` to all `CMakePresets.json` for standardized building.
- [x] **CMake Export Strategy:** Proper `find_package` support for all modules.

### 10.3 Runtime Sterilization (Completed)
- [x] **Stateless Execution:** Verified Engine/Runtime have no global state.
- [x] **Double-Free Prevention:** Fixed backend shutdown logic to prevent crashes.
- [x] **Binary-Only Loader:** Removed JSON loading from `sf_loader`. The runtime now strictly accepts only `.sfc/.bin` cartridges.

### 10.4 Integration & Validation Ecosystem (mf-tests) (Completed)
- [x] **Dedicated Test Repo:** Separate all `.mfapp` projects into a standalone structure.
- [x] **Automated Test Runner:** Automatic compilation and verification utility.
- [x] **Universal Module Ports:** Added vcpkg ports for all modules, including `sf-samples`.

---

## Phase 11: GPU Readiness & Execution Model Evolution (Optimization) (Completed)
**Goal:** Prepare for high-performance GPU backends (Vulkan/Metal/CUDA) by moving all "intelligence" into the Engine and simplifying the Backend to a "Pure Executor".

### 11.1 Engine Brain Shift (Zero-Overhead Dispatch) (Completed)
- [x] **N-Dimensional Stride Model:** Move from a single "linear stride" to a full `int32_t strides[SF_MAX_REGISTERS][SF_MAX_DIMS]` array in the execution context.
- [x] **Stride Baking:** Pre-calculate all N-D strides during the `Bake` phase or when resources are resized.
- [x] **Execution Grid Formation (Tiling):** Engine must calculate the N-Dimensional execution grid (e.g. 1920x1080x1) and pass it to the Backend.
- [x] **Linear Access Simplification:** Simplify `cpu_worker_job` by removing manual `idx % shape` logic for 1D tasks.
- [x] **Strict Generation Checks:** Integrate JSON Schema validation into `isa_gen.py` to ensure specification integrity.

### 11.2 Synchronization & Memory Safety (Completed)
- [x] **Barrier Planning:** Implement an automated Memory Barrier planner.
- [x] **Dependency Graph:** Analyze task sequences to determine where Read-after-Write (RAW) barriers are needed.
- [x] **Static Sync Analysis:** Delegate barrier planning to the **Compiler** by introducing `SF_TASK_FLAG_BARRIER`.
- [x] **Abstract Barrier API:** Create `sf_backend_barrier()` to map Engine's sync plan to `VkMemoryBarrier` or `stdatomic_thread_fence`.

### 11.3 GPU-Friendly Kernels (Completed)
- [x] **N-D Indexing in Kernels:** Update all generated and manual kernels to use N-D strides.
- [x] **Task Specialization (Fast Path):** Use SIMD/memcpy for contiguous tasks without stride overhead.
- [x] **DSL-Enhanced Kernels:** Implement a simplified DSL within Jinja2 templates for complex math (e.g. DOT, MATMUL) and unify it with N-D indexing for cross-backend portability.
- [x] **Push Constant Optimization:** Group small uniform constants (rank-0 tensors with SF_TENSOR_FLAG_CONSTANT) into a dedicated "Constant Block" to minimize binding overhead.

