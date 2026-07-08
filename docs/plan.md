# AI Infra 两个月每日计划

起止日期：2026-07-08 至 2026-09-01  
总周期：8 周，56 天  
总投入：200 小时  
节奏：工作日每天 3 小时，周六周日每天 5 小时  
项目目标：2 个月内完成一个可展示的 AI Infra 学习与项目组合雏形，包括 BlockServe-Sim、CUDA Kernel Lab、ClusterPilot-Lite 和完整文档。

---

## 总体产出

两个月结束时，至少应形成以下成果：

1. `blockserve-sim/`
   - C++ 调度模拟器
   - 请求状态机
   - Paged KV Cache 模拟
   - Sequential / Fixed Batch / Continuous Batching 对比
   - benchmark 输入、输出、图表、文档

2. `cuda-kernel-lab/`
   - vector add
   - RMSNorm
   - RoPE 或 Argmax/Sampling
   - 正确性验证
   - 简单性能记录

3. `clusterpilot-lite/`
   - GPU 集群调度模拟器
   - FIFO
   - Bin Packing
   - Gang Scheduling
   - GPU 利用率、等待时间、碎片率统计

4. `docs/`
   - BlockServe KV Cache 说明
   - BlockServe Scheduler 说明
   - Benchmark 报告
   - CUDA Kernel Lab 报告
   - ClusterPilot-Lite 报告
   - 两个月总结文档

---

## 每日固定工作方式

### 工作日 3 小时模板

| 时间 | 内容 |
|---|---|
| 45 分钟 | 学习当天相关概念 |
| 90 分钟 | 写代码或改项目结构 |
| 30 分钟 | 测试、调试、修正 |
| 15 分钟 | 写当日记录 |

### 休息日 5 小时模板

| 时间 | 内容 |
|---|---|
| 60 分钟 | 学习与复盘 |
| 180 分钟 | 核心开发 |
| 45 分钟 | 测试与 benchmark |
| 15 分钟 | 整理文档和提交记录 |

### 每天必须完成

- 至少一次 Git commit。
- 至少记录一个问题、一个结论、一个下一步。
- 不允许只看资料不产出。
- 当天任务未完成时，优先保留主线，砍掉扩展项。

---

# 第 1 周：C++ 工程基础与仓库骨架

周目标：建立工程化开发基础，完成仓库结构、CMake、测试框架、日志、JSON 配置读取。

## Day 1｜2026-07-08｜周三｜3h

主题：建立仓库与总结构。

任务：

1. 创建仓库 `ai-infra-lab`。
2. 建立目录：
   - `blockserve-sim/`
   - `cuda-kernel-lab/`
   - `clusterpilot-lite/`
   - `docs/`
   - `scripts/`
3. 写根目录 `README.md`，说明项目目标和两个月计划。
4. 安装或确认工具链：CMake、GCC/Clang、Python、Git。

产出：

- 根目录结构完成。
- `README.md` 初版。
- 第一次 Git commit。

验收：

- 仓库结构清晰。
- 能说明三个子项目分别解决什么问题。

## Day 2｜2026-07-09｜周四｜3h

主题：CMake 基础与最小 C++ 程序。

任务：

1. 在 `blockserve-sim/` 下创建最小 CMake 项目。
2. 编译一个 `main.cpp`。
3. 学习并记录：target、include directory、build directory。
4. 增加 `src/`、`include/`、`tests/` 目录。

产出：

- `blockserve-sim` 可以被 CMake 编译。
- `docs/cpp_cmake_notes.md` 初版。

验收：

- 可以执行：`cmake -B build && cmake --build build`。

## Day 3｜2026-07-10｜周五｜3h

主题：C++ 基础结构与编译单元。

任务：

1. 学习并使用：`.h` / `.cpp` 分离。
2. 写一个简单的 `AppConfig` 类。
3. 理解 namespace、class、struct、构造函数。
4. 整理代码风格：命名、目录、头文件保护。

产出：

- `AppConfig` 或类似配置类。
- 一个最小可运行 CLI。

验收：

- 能解释头文件和源文件如何组织。
- 能解释 class 与 struct 的使用场景。

## Day 4｜2026-07-11｜周六｜5h

主题：测试框架与基础容器。

任务：

1. 集成 GoogleTest 或 Catch2。
2. 写 3 个最小单元测试。
3. 学习并使用：`std::vector`、`std::queue`、`std::unordered_map`。
4. 写一个简单队列类，用测试覆盖 push/pop/empty。

产出：

- 测试框架可运行。
- `tests/` 下至少 3 个测试用例。

验收：

- 可以执行测试命令。
- 测试失败时能定位问题。

## Day 5｜2026-07-12｜周日｜5h

主题：RAII、资源管理与错误处理。

任务：

1. 学习 RAII、`std::unique_ptr`、引用和值语义。
2. 写一个简单资源对象，例如模拟资源句柄。
3. 增加错误处理方式：返回 bool / enum / optional。
4. 写测试覆盖资源申请、释放、失败场景。

产出：

- 一个资源管理小模块。
- `docs/cpp_raii_notes.md`。

验收：

- 能解释为什么系统项目里 RAII 重要。

## Day 6｜2026-07-13｜周一｜3h

主题：JSON 配置与日志。

任务：

1. 集成 `nlohmann/json`。
2. 集成 `spdlog` 或使用简单日志封装。
3. 读取一个 `config.json`。
4. 打印配置参数。

产出：

- `configs/default.json`。
- 配置读取模块。

验收：

- 修改 JSON 后程序行为能变化。

## Day 7｜2026-07-14｜周二｜3h

主题：第 1 周整理。

任务：

1. 重构目录结构。
2. 修正 CMake。
3. 补充 README 运行说明。
4. 清理无用代码。
5. 写第 1 周复盘。

产出：

- `docs/week1_review.md`。
- 稳定可编译的工程骨架。

验收：

- 从干净环境 clone 后能编译和运行测试。

---

# 第 2 周：BlockServe-Sim 请求模型与状态机

周目标：实现请求生命周期模拟，不涉及 KV Cache 和 GPU。

## Day 8｜2026-07-15｜周三｜3h

主题：定义请求模型。

任务：

1. 定义请求的必要字段：到达时间、prompt 长度、输出长度、状态。
2. 定义请求状态枚举。
3. 实现请求创建和基础校验。
4. 写单元测试。

产出：

- Request 模块。
- RequestStatus 枚举。

验收：

- 能创建合法请求。
- 非法请求能被拒绝或标记。

## Day 9｜2026-07-16｜周四｜3h

主题：Workload 输入。

任务：

1. 设计 JSONL workload 格式。
2. 实现 workload 读取。
3. 对输入字段做校验。
4. 写 2 个示例 workload。

产出：

- `benchmark/workloads/simple.jsonl`
- WorkloadLoader。

验收：

- 程序能读取多个请求并按 arrival time 排序。

## Day 10｜2026-07-17｜周五｜3h

主题：模拟时间推进。

任务：

1. 实现模拟器主循环。
2. 支持当前时间 tick 推进。
3. 到达时间满足时，将请求加入等待队列。
4. 输出每个 tick 的队列状态。

产出：

- Simulator 初版。

验收：

- 给定 workload，能看到请求按时间进入系统。

## Day 11｜2026-07-18｜周六｜5h

主题：请求状态机。

任务：

1. 实现 WAITING → PREFILLING → DECODING → FINISHED。
2. 模拟 prefill 消耗时间。
3. 模拟 decode 每次生成一个 token。
4. 记录 first token time 和 finish time。
5. 写状态转换测试。

产出：

- 请求生命周期完整跑通。

验收：

- 每个请求都有开始时间、首 token 时间、完成时间。

## Day 12｜2026-07-19｜周日｜5h

主题：取消、超时、拒绝。

任务：

1. 增加 CANCELLED 状态。
2. 增加 TIMEOUT 状态。
3. 增加 REJECTED 状态。
4. workload 支持可选 cancel_time 和 timeout。
5. 写测试覆盖异常状态。

产出：

- 异常状态可模拟。

验收：

- 请求可以在任意阶段被取消或超时。

## Day 13｜2026-07-20｜周一｜3h

主题：指标记录。

任务：

1. 实现 MetricsRecorder。
2. 记录 latency、queue time、TTFT、完成数量。
3. 输出 CSV 或 JSON。
4. 写一个简单结果文件。

产出：

- `benchmark/results/simple_result.json` 或 `.csv`。

验收：

- 能根据结果计算平均延迟和完成请求数。

## Day 14｜2026-07-21｜周二｜3h

主题：第 2 周整理。

任务：

1. 补充 `docs/blockserve_request_lifecycle.md`。
2. 清理状态机代码。
3. 补全测试。
4. 写第 2 周复盘。

产出：

- 请求生命周期文档。
- `docs/week2_review.md`。

验收：

- 能完整解释请求从进入系统到完成的过程。

---

# 第 3 周：Paged KV Cache 模拟

周目标：实现 block-based KV Cache 管理，不做真实 GPU 显存。

## Day 15｜2026-07-22｜周三｜3h

主题：KV Cache 问题建模。

任务：

1. 学习 KV Cache 的作用。
2. 写笔记：为什么 LLM serving 会被 KV Cache 限制。
3. 定义 block_size、total_blocks、used_blocks。
4. 在配置文件中加入 KV 参数。

产出：

- `docs/blockserve_kv_cache.md` 初版。
- 配置支持 KV 参数。

验收：

- 能用自己的话解释 KV Cache 和 block 的关系。

## Day 16｜2026-07-23｜周四｜3h

主题：BlockPool。

任务：

1. 实现 BlockPool。
2. 支持 allocate one block。
3. 支持 free one block。
4. 支持查询 free/used 数量。
5. 写单元测试。

产出：

- BlockPool 模块。

验收：

- block 申请和释放行为正确。

## Day 17｜2026-07-24｜周五｜3h

主题：Request Block Table。

任务：

1. 为每个请求维护 block table。
2. 支持请求申请新 block。
3. 支持请求释放全部 block。
4. 防止重复释放。
5. 写单元测试。

产出：

- RequestBlockTable 模块。

验收：

- 多请求不会共享同一个 block。

## Day 18｜2026-07-25｜周六｜5h

主题：token 增长与按需分配。

任务：

1. 模拟 prefill 期间 token 增长。
2. 模拟 decode 期间 token 逐步增加。
3. 当 token 数跨过 block 边界时申请新 block。
4. block 不足时拒绝或暂停请求。
5. 写测试覆盖边界。

产出：

- token-to-block 模拟逻辑。

验收：

- prompt_len=33、block_size=16 时需要 3 个 block。

## Day 19｜2026-07-26｜周日｜5h

主题：资源回收。

任务：

1. 请求 FINISHED 后释放 block。
2. 请求 CANCELLED 后释放 block。
3. 请求 TIMEOUT 后释放 block。
4. 增加长时间随机测试。
5. 检查 block 是否泄漏。

产出：

- 资源回收逻辑稳定。

验收：

- 所有请求结束后 used_blocks 回到 0。

## Day 20｜2026-07-27｜周一｜3h

主题：连续 KV Cache baseline。

任务：

1. 模拟 contiguous cache 策略。
2. 每个请求按最大长度预留 block。
3. 与 paged cache 的按需增长做对比准备。
4. 写测试。

产出：

- ContiguousCacheSimulator。

验收：

- 同一请求在 contiguous 下通常占用更多 block。

## Day 21｜2026-07-28｜周二｜3h

主题：第 3 周整理。

任务：

1. 完成 `docs/blockserve_kv_cache.md`。
2. 补测试。
3. 补 README 中 KV Cache 说明。
4. 写第 3 周复盘。

产出：

- KV Cache 文档成型。
- `docs/week3_review.md`。

验收：

- 能讲清楚 paged cache 和 contiguous cache 的差异。

---

# 第 4 周：调度策略与 Continuous Batching

周目标：实现 sequential、fixed batch、continuous batching 三类策略。

## Day 22｜2026-07-29｜周三｜3h

主题：Sequential baseline。

任务：

1. 实现一个请求跑完再处理下一个请求。
2. 接入已有请求状态机。
3. 输出 latency、TTFT、完成数。
4. 写测试。

产出：

- SequentialScheduler。

验收：

- 能作为最简单 baseline 运行。

## Day 23｜2026-07-30｜周四｜3h

主题：Fixed Batch baseline。

任务：

1. 实现固定 batch 收集逻辑。
2. batch 内请求对齐处理。
3. 模拟短请求等待长请求。
4. 输出指标。

产出：

- FixedBatchScheduler。

验收：

- 能观察 fixed batch 的等待和浪费。

## Day 24｜2026-07-31｜周五｜3h

主题：Continuous Batching 基础。

任务：

1. 设计每轮调度 step。
2. decode 请求优先生成一个 token。
3. 有剩余预算时加入新 prefill。
4. 引入 token budget。

产出：

- ContinuousBatchScheduler 初版。

验收：

- 多个请求可交错执行。

## Day 25｜2026-08-01｜周六｜5h

主题：prefill/decode 分离。

任务：

1. 明确 prefill 队列和 decode 队列。
2. 实现 prefill 完成后进入 decode。
3. 实现 max_prefill_tokens_per_step。
4. 模拟长 prompt 对 decode 的影响。
5. 写测试。

产出：

- Prefill / Decode 分离调度。

验收：

- 长 prompt 不会完全阻塞所有 decode。

## Day 26｜2026-08-02｜周日｜5h

主题：chunked prefill。

任务：

1. 将长 prompt 拆成多个 chunk。
2. 每轮只处理一个或多个 chunk。
3. 对比一次性 prefill 与 chunked prefill。
4. 记录 TTFT 和 TPOT 变化。

产出：

- Chunked prefill 模拟。

验收：

- 同一 workload 下可比较 chunked 与 non-chunked。

## Day 27｜2026-08-03｜周一｜3h

主题：Paged cache 接入调度。

任务：

1. continuous batching 接入 paged cache。
2. 请求每生成 token 时更新 block 使用。
3. block 不足时触发 reject 或等待。
4. 记录 peak_used_blocks。

产出：

- Continuous + Paged Cache 可运行。

验收：

- 调度和 KV 资源管理联动。

## Day 28｜2026-08-04｜周二｜3h

主题：第 4 周整理。

任务：

1. 完成 `docs/blockserve_scheduler.md`。
2. 整理三种调度策略说明。
3. 补测试。
4. 写第 4 周复盘。

产出：

- Scheduler 文档。
- `docs/week4_review.md`。

验收：

- 能讲清楚 fixed batch 与 continuous batching 的区别。

---

# 第 5 周：Benchmark、实验与报告

周目标：让 BlockServe-Sim 形成可展示实验结果。

## Day 29｜2026-08-05｜周三｜3h

主题：workload 生成器。

任务：

1. 写 Python workload 生成脚本。
2. 生成 short_chat workload。
3. 生成 mixed_length workload。
4. 生成 long_prompt workload。

产出：

- `scripts/generate_workload.py`
- 三组 workload。

验收：

- workload 可重复生成，支持随机种子。

## Day 30｜2026-08-06｜周四｜3h

主题：tight memory workload。

任务：

1. 设计显存受限场景。
2. 生成 tight_memory workload。
3. 调整 total_blocks 和 block_size 参数。
4. 确保能触发 reject 或排队。

产出：

- `tight_memory.jsonl`。

验收：

- paged cache 与 contiguous cache 出现明显差异。

## Day 31｜2026-08-07｜周五｜3h

主题：benchmark runner。

任务：

1. 写 benchmark 批量运行脚本。
2. 自动跑不同 scheduler 和 cache 策略。
3. 输出统一格式结果。
4. 保存配置和结果。

产出：

- `scripts/run_benchmark.py`。

验收：

- 一条命令能跑完一组完整对比。

## Day 32｜2026-08-08｜周六｜5h

主题：结果统计。

任务：

1. 统计 completed、rejected、avg latency、P95 latency。
2. 统计 TTFT、TPOT、tokens/s。
3. 统计 peak blocks、cache utilization。
4. 统一写入 CSV。

产出：

- benchmark result CSV。

验收：

- 指标含义清楚，计算方式可解释。

## Day 33｜2026-08-09｜周日｜5h

主题：图表与分析。

任务：

1. 用 Python 画完成请求数对比图。
2. 画 P95 latency 对比图。
3. 画 peak block usage 对比图。
4. 画 rejected requests 对比图。
5. 写初步分析。

产出：

- `benchmark/plots/` 下至少 4 张图。

验收：

- 图表能说明系统策略差异。

## Day 34｜2026-08-10｜周一｜3h

主题：Benchmark 报告。

任务：

1. 写 `docs/blockserve_benchmark.md`。
2. 说明实验环境、配置、workload、结果。
3. 不夸大结论，只描述数据。
4. 写当前限制。

产出：

- Benchmark 报告初版。

验收：

- 别人能根据文档复现实验。

## Day 35｜2026-08-11｜周二｜3h

主题：BlockServe-Sim 总整理。

任务：

1. 整理 BlockServe-Sim README。
2. 补充运行命令。
3. 清理代码和无用文件。
4. 写第 5 周复盘。

产出：

- BlockServe-Sim 形成阶段性完整项目。
- `docs/week5_review.md`。

验收：

- BlockServe-Sim 可以作为简历项目雏形展示。

---

# 第 6 周：CUDA Kernel Lab

周目标：掌握 CUDA 基础，完成 2~3 个真实 LLM 相关小 kernel。

## Day 36｜2026-08-12｜周三｜3h

主题：CUDA 环境与 vector add。

任务：

1. 确认 CUDA 编译环境。
2. 建立 `cuda-kernel-lab` CMake 项目。
3. 写 vector add kernel。
4. 用 CPU 结果验证正确性。

产出：

- vector add 可运行。

验收：

- 能解释 kernel launch、grid、block、thread。

## Day 37｜2026-08-13｜周四｜3h

主题：CUDA 内存与计时。

任务：

1. 学习 cudaMalloc、cudaMemcpy、cudaFree。
2. 使用 cudaEvent 计时。
3. 给 vector add 增加不同输入规模测试。
4. 记录运行时间。

产出：

- vector add benchmark。

验收：

- 能区分数据拷贝时间和 kernel 时间。

## Day 38｜2026-08-14｜周五｜3h

主题：RMSNorm 数学与 CPU reference。

任务：

1. 学习 RMSNorm 在 Transformer 中的位置。
2. 用 Python 或 C++ 写 CPU reference。
3. 设计输入输出格式。
4. 准备测试数据。

产出：

- RMSNorm reference。

验收：

- 能解释 RMSNorm 的输入、输出、参数。

## Day 39｜2026-08-15｜周六｜5h

主题：RMSNorm CUDA kernel。

任务：

1. 实现 naive RMSNorm kernel。
2. 与 CPU/Python reference 对比。
3. 记录 max error 和 mean error。
4. 增加不同 hidden size 测试。

产出：

- RMSNorm kernel 初版。

验收：

- 正确性误差在可接受范围内。

## Day 40｜2026-08-16｜周日｜5h

主题：RMSNorm 简单优化与分析。

任务：

1. 优化线程组织。
2. 尝试减少重复全局内存访问。
3. 对比 naive 和 optimized。
4. 写 RMSNorm 小报告。

产出：

- `cuda-kernel-lab/rmsnorm/README.md`。

验收：

- 能说明优化前后差异，不要求达到工业性能。

## Day 41｜2026-08-17｜周一｜3h

主题：RoPE reference。

任务：

1. 学习 RoPE 的作用。
2. 写 CPU/Python reference。
3. 准备输入数据。
4. 明确维度布局。

产出：

- RoPE reference。

验收：

- 能解释 RoPE 为什么和 position 相关。

## Day 42｜2026-08-18｜周二｜3h

主题：RoPE CUDA kernel 与第 6 周整理。

任务：

1. 实现简单 RoPE kernel。
2. 做 correctness 对比。
3. 整理 `cuda-kernel-lab/README.md`。
4. 写第 6 周复盘。

产出：

- RoPE kernel 初版。
- `docs/week6_review.md`。

验收：

- CUDA Kernel Lab 至少有 vector add、RMSNorm、RoPE 三个可运行样例。

---

# 第 7 周：ClusterPilot-Lite GPU 集群调度模拟器

周目标：完成小型 GPU 集群调度器模拟，证明分布式资源调度理解。

## Day 43｜2026-08-19｜周三｜3h

主题：集群与 Job 建模。

任务：

1. 定义 Node、GPU、Job。
2. 定义 job 字段：arrival_time、gpu_count、duration、priority。
3. 建立 cluster config。
4. 写简单 workload。

产出：

- ClusterPilot-Lite 项目骨架。

验收：

- 能加载一个虚拟 GPU 集群。

## Day 44｜2026-08-20｜周四｜3h

主题：FIFO 调度。

任务：

1. 实现 FIFO 队列。
2. 有资源则启动 job。
3. 资源不足则等待。
4. job 完成后释放 GPU。
5. 记录完成时间。

产出：

- FIFO 调度器。

验收：

- 多个 job 可以按顺序运行完成。

## Day 45｜2026-08-21｜周五｜3h

主题：指标统计。

任务：

1. 统计 GPU utilization。
2. 统计 average queue time。
3. 统计 completed jobs。
4. 统计 large job wait time。

产出：

- Cluster metrics。

验收：

- FIFO 策略有可读结果。

## Day 46｜2026-08-22｜周六｜5h

主题：Bin Packing。

任务：

1. 实现 bin packing 策略。
2. 优先把 job 放到已有占用节点或最合适节点。
3. 统计碎片率。
4. 与 FIFO 对比。

产出：

- BinPackingScheduler。

验收：

- 能说明 bin packing 对碎片和利用率的影响。

## Day 47｜2026-08-23｜周日｜5h

主题：Gang Scheduling。

任务：

1. 实现 gang scheduling。
2. job 要么拿到所需全部 GPU，要么不启动。
3. 设计大任务 workload。
4. 对比 FIFO 和 gang 策略。

产出：

- GangScheduler。

验收：

- 大 GPU 任务不会被部分分配导致资源占用。

## Day 48｜2026-08-24｜周一｜3h

主题：workload 与实验。

任务：

1. 生成混合 workload。
2. 运行 FIFO、Bin Packing、Gang Scheduling。
3. 输出 CSV。
4. 画 2 张图。

产出：

- ClusterPilot 实验结果。

验收：

- 三种策略有可比较数据。

## Day 49｜2026-08-25｜周二｜3h

主题：第 7 周整理。

任务：

1. 写 `clusterpilot-lite/README.md`。
2. 写 `docs/clusterpilot_report.md`。
3. 补测试。
4. 写第 7 周复盘。

产出：

- ClusterPilot-Lite 阶段性完成。
- `docs/week7_review.md`。

验收：

- 能讲清楚 GPU 集群为什么需要 gang scheduling 和 bin packing。

---

# 第 8 周：整合、文档、简历化

周目标：把已有成果整理成可展示仓库和可写入简历的项目。

## Day 50｜2026-08-26｜周三｜3h

主题：仓库清理。

任务：

1. 删除临时代码。
2. 统一目录命名。
3. 统一 README 风格。
4. 检查所有命令是否可运行。

产出：

- 干净仓库结构。

验收：

- 根目录 README 能引导别人运行三个子项目。

## Day 51｜2026-08-27｜周四｜3h

主题：BlockServe-Sim 最终文档。

任务：

1. 完善 BlockServe-Sim README。
2. 加入架构说明，但不写过度设计。
3. 加入 benchmark 结果图。
4. 加入当前限制和后续计划。

产出：

- BlockServe-Sim 文档完成。

验收：

- 一个不了解项目的人能在 10 分钟内看懂目标和运行方式。

## Day 52｜2026-08-28｜周五｜3h

主题：CUDA Kernel Lab 最终文档。

任务：

1. 完善 CUDA Kernel Lab README。
2. 说明每个 kernel 的目的。
3. 记录 correctness 结果。
4. 记录简单性能结果。

产出：

- CUDA Kernel Lab 文档完成。

验收：

- 能展示你已经开始具备 CUDA 基础。

## Day 53｜2026-08-29｜周六｜5h

主题：ClusterPilot-Lite 最终文档。

任务：

1. 完善 ClusterPilot-Lite README。
2. 补充三种调度策略说明。
3. 加入实验结果。
4. 写当前限制和下一步计划。

产出：

- ClusterPilot-Lite 文档完成。

验收：

- 能展示你理解 GPU 集群调度的基本问题。

## Day 54｜2026-08-30｜周日｜5h

主题：两个月总结报告。

任务：

1. 写 `docs/two_month_report.md`。
2. 总结 BlockServe-Sim、CUDA Kernel Lab、ClusterPilot-Lite。
3. 说明学到的核心概念。
4. 说明没有完成的内容和原因。
5. 制定下一阶段计划。

产出：

- 两个月总结报告。

验收：

- 报告可以作为面试前复习材料。

## Day 55｜2026-08-31｜周一｜3h

主题：简历化表达。

任务：

1. 写项目简历 bullet。
2. 准备 3 分钟项目介绍。
3. 准备 10 个面试自问自答。
4. 检查是否有夸大表述。

产出：

- `docs/resume_bullets.md`
- `docs/interview_notes.md`

验收：

- 能用简洁语言说明项目价值。

## Day 56｜2026-09-01｜周二｜3h

主题：最终验收。

任务：

1. 从头运行所有项目。
2. 确认 benchmark 可复现。
3. 确认图表和文档链接正确。
4. 打 tag：`v0.1-two-month-plan`。
5. 写最终复盘。

产出：

- 可展示版本 v0.1。
- `docs/final_review.md`。

验收：

- 仓库可编译、可运行、可读、可讲。

---

# 每周检查点

| 周 | 检查点 |
|---|---|
| 第 1 周 | C++ 工程骨架可编译，测试框架可运行 |
| 第 2 周 | 请求生命周期模拟完成 |
| 第 3 周 | Paged KV Cache 模拟完成 |
| 第 4 周 | 三种调度策略完成 |
| 第 5 周 | BlockServe-Sim benchmark 和报告完成 |
| 第 6 周 | CUDA Kernel Lab 完成至少 3 个 kernel 示例 |
| 第 7 周 | ClusterPilot-Lite 完成三种调度策略 |
| 第 8 周 | 文档、图表、简历 bullet、最终复盘完成 |

---

# 风险控制

## 如果第 3 周 KV Cache 卡住

砍掉 contiguous baseline，先把 paged block pool 做正确。

## 如果第 4 周 continuous batching 卡住

先实现 decode 优先和 token budget，不做 chunked prefill。

## 如果第 6 周 CUDA 卡住

保留 vector add 和 RMSNorm，RoPE 改为 CPU reference 加文档说明。

## 如果第 7 周时间不足

ClusterPilot-Lite 只保留 FIFO 和 Gang Scheduling，Bin Packing 延后。

## 如果整体进度落后

优先级如下：

1. BlockServe-Sim
2. Benchmark 和文档
3. CUDA Kernel Lab
4. ClusterPilot-Lite

不要为了保留 ClusterPilot-Lite 而牺牲 BlockServe-Sim 的完整度。

---

# 最低可接受成果

如果两个月内只完成最低版本，也必须达到：

```text
BlockServe-Sim：
- 请求状态机
- Paged KV Cache block pool
- Sequential vs Continuous Batching
- 一组 benchmark
- 一篇完整文档

CUDA Kernel Lab：
- vector add
- RMSNorm
- correctness test

ClusterPilot-Lite：
- FIFO
- Gang Scheduling
- 简单 GPU utilization 统计
```

这仍然可以构成一个清晰的 AI Infra 入门项目组合。

---

# 两个月后的下一阶段

完成本计划后，下一阶段再考虑：

1. BlockServe-Sim 接入真实小模型。
2. 写真实 CUDA Paged Attention。
3. 增加 C++ HTTP 服务。
4. 将 ClusterPilot-Lite 改为 Go 版本。
5. 研究 vLLM 代码并尝试提交小 PR。
6. 准备 AI Infra / GPU Runtime 方向实习。
