# SionFlow Roadmap v3: The Pixel Engine

**Vision:** To prove that SionFlow is a true Data-Oriented Engine by implementing a UI/Renderer purely as a mathematical function of state.

---

## Current Status: Performance & Reliability Hardening

### Phase 1-11: Completed Foundations
*   Task-Driven Execution, Stride Baking, N-Dimensional Indexing.
*   Spec-First Architecture, Jinja2 Code Generation.
*   SFC 2.0 Declarative Binary Format.

## Phase 12: Architectural Polish & Minimalism (Complete)
**Goal:** Добиться предельной чистоты кода и структуры данных. Убрать все лишние сущности.

- [x] **Eliminate COPY Node:** Удален COPY, используется алиасинг регистров.
- [x] **Zero-Copy Views:** SLICE и RESHAPE превращены в метаданные.
- [x] **Total Register Aliasing:** Внедрено в `sf_pass_liveness`.
- [x] **SFC 2.0 Serialization:** Полная автоматизация сохранения/загрузки картриджей через `binary_layout.json`.
- [x] **Smart Graph API:** Внедрен унифицированный API для манипуляции графом (Grafting, Inlining, Lowering).
- [x] **Task Plan Extraction:** Логика планирования задач и барьеров вынесена из Codegen в отдельный пасс.

## Phase 13: The Meta-Engine (Full Automation) (In Progress)
**Goal:** SionFlow должен стать "самогенерирующимся" на основе метаданных.

- [x] **Declarative Shape Inference:** Правила вывода форм описаны в JSON, `sf_pass_analyze` генерируется автоматически.
- [x] **Constraint-Based Validation:** Ассерты перенесены в JSON, `sf_pass_validate` генерируется автоматически.
- [x] **Unified Serialization (SFC 2.0):** Структура файла описана в JSON, функции save/load генерируются автоматически.
- [x] **Professional Inlining & Lowering:** Пассы инлайнинга и декомпозиции переведены на Smart Graph API.
- [x] **Metadata Sterilization:** Из `sf_op_metadata` удалены все поля, дублирующиеся в логике пассов.
- [ ] **13.6 Declarative Lowering:** Перенести правила разложения сложных узлов (`Mean`, `SmoothStep`) из Си-кода в `compiler_spec.json`.
- [ ] **13.7 Advanced Kernel DSL:** Внедрить абстрактный DSL в `sf-spec/tools/templates` для описания математики ядер, независимой от бэкенда (CPU/GPU).

## Phase 13.5: Compiler IR Evolution (The Great Refactoring)
**Goal:** Перейти от "индексной бухгалтерии" к изящному Pointer-based IR и разделить уровни абстракции.

- [ ] **Port-Centric IR (Graph 2.0):** Заменить плоские массивы `links` на прямые указатели между портами узлов (Def-Use chains).
- [ ] **Logical/Physical Separation:** Разделить `sf_ir_node` на чистый логический узел (Graph IR) и физический дескриптор (Execution Manifest).
- [ ] **Atomic Mutation API:** Создать библиотеку "строителя графа" (Graph Builder) для атомарных и безопасных трансформаций (Replace, Fuse, Inline) без ручного пересчета индексов.
- [ ] **O(1) Access Structures:** Внедрить хеш-таблицы или списки смежности для мгновенного поиска соседей узла, избавившись от линейного поиска в пассах.

## Phase 14: Zero-Boilerplate Runtime & GPU (Future)
**Goal:** Максимальное упрощение рантайма и запуск на GPU.

- [ ] **Resource Layout Spec:** Описать правила выделения памяти для тензоров (Alignment, Ping-Pong) в метаданных, чтобы Engine работал по готовой карте памяти.
- [ ] **Declarative Barrier Planning:** Перенести логику расстановки RAW-барьеров из Си-кода компилятора в спецификацию зависимостей.
- [ ] **Vulkan Backend:** Реализация первого GPU бэкенда, использующего те же шаблоны ядер, что и CPU.
- [ ] **Shader Auto-Gen:** Генерация GLSL/WGSL кода напрямую из метаданных ISA.

---

## Phase 15: The UI Engine (Final Proof)
**Goal:** Реализация полнофункционального UI-движка, работающего на SionFlow.