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

### Phase 7: Quality of Life & Extreme Shrink (In Progress)
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
- [ ] **Task Specialization (Fast Path):** Внедрить флаги задач (например, `SF_TASK_CONTIGUOUS`). Бэкенд должен использовать SIMD или `memcpy` для полностью непрерывных данных без проверки страйдов в цикле.
- [ ] **Linear Access Simplification:** Упростить `cpu_worker_job`, чтобы для линейных задач не выполнялась дорогостоящая логика развертки N-мерных индексов (`tile_offset`).

## Phase 9: The "Cartridge" Model (Autonomous Packaging) (Completed)
**Goal:** Полная архитектурная изоляция. Превращение `.bin` в самодостаточный "картридж", содержащий настройки хоста, ресурсы и весь вычислительный пайплайн. Рантайм становится "тупым" плеером.

- [x] **Resource Templates in Binary:** Перенесено описание глобальных ресурсов и их флагов (`READONLY`, `PERSISTENT`) прямо в таблицу символов `sf_program`.
- [x] **Unified App Header:** Расширен `sf_bin_header`, добавлены параметры хоста (Window Title, Width, Height, VSync, NumThreads) прямо в файл программы.
- [x] **Zero-JSON Runtime (Partial):** Рантайм теперь может запускаться напрямую из `.bin/.sfc` без использования `.mfapp` или других JSON-файлов.
- [x] **Binary Pipeline Bundling:** Формат умеет хранить несколько программ (логика, физика, рендер) и описание их связей внутри одного файла.
- [x] **Standalone Compiler CLI (`sfc`):** Создана утилита, которая принимает JSON и все связанные ресурсы, проводит полную валидацию и «запекает» их в один финальный картридж.
- [x] **Asset Embedding (The Blob):** Реализована упаковка внешних ассетов (текстуры, шрифтов) прямо в секции бинарного файла для создания 100% автономных приложений.

## Phase 10: The Great Decoupling & Standardization
**Goal:** Transform SionFlow into a professional, multi-repo ecosystem with a strict separation between Specification, Tooling, and Runtime.

### 10.1 Specification-First Design (The "Single Source of Truth")
- [x] **Decoupled ISA Definition (`isa.json`):** Purge all implementation-specific logic (C expressions) from the core specification. It must only define the "Contract".
- [x] **Backend Implementation Specs (`cpu_spec.json`):** Created backend-specific mapping files in `tools/metadata` that link ISA nodes to concrete implementations.
- [x] **Template-Based Code Generation (Jinja2):** Replaced brittle X-Macros with robust Python + Jinja2 templates for Opcodes, Metadata, and Kernels.
- [ ] **Compiler Transformation Spec (`compiler_spec.json`):** Move high-level compiler logic into declarative metadata.
    - [ ] **Fusion & Lowering:** Define patterns (e.g., `Mul+Add -> FMA`) and macro-node decompositions (e.g., `Mean -> Sum/Size`) in JSON.
    - [ ] **Mathematical Axioms:** Declare commutativity, associativity, and purity (side-effects) to automate optimizer permutations.
    - [ ] **Provider Registry:** Map external strings (`host.time`, `host.index.N`) to internal opcodes via aliases.
- [ ] **Data-Driven Validation & Analysis:** Move rank/shape constraints and type promotion rules from manual C code to schema-validated JSON formulas.
- [ ] **Schema Validation:** Ensure all `.json` metadata files adhere to a strict schema for professional-grade reliability.

### 10.7 Legacy Purge & Structural Refinement
- [ ] **Data-Driven Optimizer:** Rewrite `sf_pass_fuse.c` to use generated pattern-matching logic from `compiler_spec.json`, removing 100+ lines of manual C-code.
- [ ] **Template-Only Kernels:** Eliminate the `SF_KERNEL_AUTO` macro in `sf_kernel_utils.h`. Move the hot-loop logic entirely into Jinja2 templates for 100% transparent generated code.
- [ ] **Unified Validation:** Replace manual `switch/if` chains in `sf_pass_validate.c` with a single loop driven by `sf_op_metadata` (arity, type masks).
- [ ] **Zero-Boilerplate Manual Kernels:** Refactor complex kernels (`DOT`, `NORMALIZE`) to use generated pointer-arithmetic wrappers, reducing risk of stride errors.
- [ ] **Metadata Stripping:** Wrap debug metadata (port names, opcode strings) in `sf_opcodes.c` with `#ifndef NDEBUG` to minimize production binary footprint.
- [ ] **Strict Generation Checks:** Integrate JSON Schema validation into `isa_gen.py` to catch specification errors before the C-compiler runs.

### 10.2 Modular Dependency Management
- [x] **vcpkg Integration:** Create official `vcpkg` ports for `sf-spec`, `sf-compiler`, and `sf-runtime`.
- [x] **Developer Overlay Workflow:** Implement a `vcpkg-overlays/` system to allow seamless local development across repositories without constant re-installation.
- [x] **CMake Export Strategy:** Implement proper `find_package` support by creating `*Config.cmake` files and export targets for all modules.
- [x] **Strict Linking:** Ensure the Runtime can be built and distributed without a single line of Compiler or JSON-parsing code.

### 10.3 Runtime Sterilization
- [ ] **Binary-Only Loader:** Remove JSON loading from `sf_loader`. The production runtime will strictly accept only `.sfc` (binary) cartridges.
- [ ] **Stateless Execution:** Finalize the removal of all global state from the loader and engine to support multi-instance hosting.

### 10.4 Integration & Validation Ecosystem (mf-tests)
- [x] **Dedicated Test Repo:** Separate all `.mfapp` projects, JSON graphs, and assets into a standalone repository (sf-samples/tests).
- [x] **Automated Test Runner:** Create a utility that automatically compiles all test graphs using `sfc` and verifies their execution against golden outputs.
- [ ] **Cross-Component CI:** Implement CI that triggers on changes in any repo to ensure the Spec, Compiler, and Runtime remain perfectly synchronized.

### 10.5 Library Distribution & Stability
- [ ] **C-API Hardening:** Guarantee a stable ABI for `libsionflow-rt` for 3rd-party language bindings (Python/Rust).
- [ ] **Reflection Metadata:** Allow optional embedding of debug symbols in cartridges for tooling support without bloating the runtime.

### 10.6 Release Engineering
- [ ] **Semantic Versioning (SemVer):** Implement strict versioning for all components to manage breaking changes in the ISA or binary format.
- [ ] **Automated Release Pipeline:** Tools to synchronize versions across all four repositories during a release.
