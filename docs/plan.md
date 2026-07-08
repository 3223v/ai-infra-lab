# AI Infra 12 周完整每日计划（BlockServe + TinyGPT + 1B 推理基线）

起止日期：2026-07-08 至 2026-09-29  
总周期：12 周，84 天  
总投入：300 小时  
节奏：工作日每天 3 小时，周六周日每天 5 小时，一周 25 小时  
当前基础：C++ 有基础但高级能力不足；已有必要计算机基础；本地环境为 macOS arm64、Apple clang、Python 3.9，当前无 CUDA 本地环境。  
核心目标：先完成 BlockServe-Sim 和调度/KV Cache 能力，再建立 1B 小模型本地推理基线，随后手写 TinyGPT 并训练 10M 级模型，最后把自训练模型接入 BlockServe CPU 推理路径，并评估 100M 模型训练可行性。

---

## 0. 重新评估后的结论

1. 原 8 周计划的主线是合理的：BlockServe-Sim、CUDA Kernel Lab、ClusterPilot-Lite 和完整文档。它已经把每天的投入固定为工作日 3 小时、休息日 5 小时，并要求每天至少一次 commit、记录问题和结论。
2. 根据你当前的 C++ 能力，前 2 周不能只堆功能，必须把 CMake、测试、错误处理、状态机、输入解析压实。否则后面 KV Cache 和调度会失控。
3. “在 4GB 显存电脑上推理 1B 左右模型”可以作为第 6 周的基线目标，但不应理解为 12 周内从零写出完整 1B Llama/Qwen Runtime。现实目标是：用 llama.cpp/Metal 或 CPU 跑通 0.5B~1.1B GGUF 模型，记录性能；自研 BlockServe 优先接入自己训练的 TinyGPT。
4. “手写小 GPT 并训练 10M 模型”适合作为第 9~11 周目标。100M 模型训练只做可行性预研，不强行训练完成。
5. 本计划采用“先系统、再模型、最后接入”的顺序：BlockServe-Sim → 1B 推理基线 → CUDA/Kernel Lab → ClusterPilot → TinyGPT-Train → BlockServe CPU 推理接入。

---

## 1. 阶段产出

### 第 1~5 周：BlockServe-Sim v0.1
- C++ 调度模拟器
- Request 状态机
- WorkloadLoader
- Paged KV Cache 模拟
- Sequential / Fixed Batch / Continuous Batching 对比
- Benchmark、图表、文档

### 第 6 周：1B 本地推理基线与 BlockServe-Real 边界
- 使用 llama.cpp/Metal 或 CPU 跑通 Qwen2.5-0.5B 或 TinyLlama-1.1B 量化模型
- 记录 tokens/s、内存占用、上下文限制
- 写清楚自研 Runtime 与工业 1B Runtime 的差距

### 第 7 周：CUDA Kernel Lab
- vector add
- RMSNorm
- RoPE 或 sampling
- 如果没有 CUDA 机器，保留 CPU reference、CUDA 源文件和远程运行说明

### 第 8 周：ClusterPilot-Lite
- 虚拟 GPU 集群
- FIFO / Bin Packing / Gang Scheduling
- GPU utilization、queue time、fragmentation 指标

### 第 9~12 周：TinyGPT-Train + BlockServe 接入
- 阅读 minGPT / nanoGPT
- 手写 TinyGPT-Train
- 训练 char-level GPT 和 10M 模型
- 导出 checkpoint
- BlockServe CPU backend 加载并生成
- 100M 模型训练可行性报告

---

## 2. 每日固定工作方式

### 工作日 3 小时

| 时间 | 内容 |
|---|---|
| 35 分钟 | 学习当天相关概念，只读必要资料 |
| 100 分钟 | 写代码或改项目结构 |
| 30 分钟 | 测试、调试、修正 |
| 15 分钟 | 写当日记录和 commit |

### 休息日 5 小时

| 时间 | 内容 |
|---|---|
| 50 分钟 | 学习与复盘 |
| 210 分钟 | 核心开发 |
| 25 分钟 | 测试或 benchmark |
| 15 分钟 | 文档、记录、commit |

### 每日硬性要求
- 至少一次 Git commit。
- 至少补一条文档或当日记录。
- 至少跑一次可执行程序或测试。
- 当天任务未完成时，先保证主线，不做扩展项。
- 不允许只看资料不产出。

---

## 3. 12 周每日计划

# 第 1 周

周目标：C++ 工程骨架、CMake、基础类型与输入读取；承接已提前推进的 Day1~Day4。重点是把当前代码整理成稳定基础，不追求功能复杂度。

## Day 1｜2026-07-08｜周三｜3h

主题：仓库与现状整理。

任务：
1. 确认当前 git 状态与已完成内容。
2. 整理根 README 与 blockserve-sim README。
3. 记录当前环境：macOS arm64、Apple clang、Python 3.9、无 CUDA。

产出 / 验收：
- git log 清晰
- docs/current_status.md

## Day 2｜2026-07-09｜周四｜3h

主题：CMake 与 Request 模块整理。

任务：
1. 统一 target 名、输出格式、RequestStatus 大写。
2. 检查 include/src 组织。
3. 补 docs/cpp_cmake_notes.md。

产出 / 验收：
- blockserve_sim 可编译运行
- 文档能解释 target/include/build

## Day 3｜2026-07-10｜周五｜3h

主题：WorkloadLoader 稳定化。

任务：
1. 修正 ID 自动递增逻辑。
2. 统一错误信息格式。
3. 补 sample/bad workload。

产出 / 验收：
- 正常/错误 workload 都可运行
- docs/workload_format.md 初版

## Day 4｜2026-07-11｜周六｜5h

主题：Arrival Simulation。

任务：
1. 实现或整理 arrival-time simulation。
2. 按 arrival_time 排序进入等待队列。
3. 输出 waiting_queue_size。

产出 / 验收：
- 请求按时间进入系统
- docs/day04.md

## Day 5｜2026-07-12｜周日｜5h

主题：测试框架接入。

任务：
1. 接入 Catch2 或 GoogleTest，优先 Catch2 单文件/FetchContent。
2. 为 RequestStatus、RequestQueue、WorkloadLoader 写最小测试。

产出 / 验收：
- ctest 可运行
- 至少 6 个测试

## Day 6｜2026-07-13｜周一｜3h

主题：RAII 与 optional 复盘。

任务：
1. 写 docs/cpp_raii_optional_notes.md。
2. 把 pop/loader 错误处理梳理清楚。
3. 补空队列、坏输入测试。

产出 / 验收：
- 能解释 optional/nullopt
- 错误路径测试通过

## Day 7｜2026-07-14｜周二｜3h

主题：第1周整理。

任务：
1. 清理命名和目录。
2. 补 README 运行命令。
3. 从干净 build 目录重编译。

产出 / 验收：
- docs/week1_review.md
- v0 工程骨架稳定


# 第 2 周

周目标：BlockServe-Sim 请求生命周期：时间推进、状态机、取消/超时/拒绝、基础指标。

## Day 8｜2026-07-15｜周三｜3h

主题：模拟主循环。

任务：
1. 实现 Simulator 主循环。
2. 支持 current_time 推进。
3. 将 arrived 请求放入 waiting_queue。

产出 / 验收：
- 给定 workload 能看到 tick 日志

## Day 9｜2026-07-16｜周四｜3h

主题：请求状态字段扩展。

任务：
1. 为 Request 增加 start_time、first_token_time、finish_time、remaining_prefill_tokens 等字段。
2. 不做复杂设计，只满足模拟。

产出 / 验收：
- 状态字段能被打印和记录

## Day 10｜2026-07-17｜周五｜3h

主题：Sequential 生命周期。

任务：
1. 实现单请求 WAITING→PREFILLING→DECODING→FINISHED。
2. prefill 用模拟 cost，decode 每步 1 token。

产出 / 验收：
- 一个请求能完整完成

## Day 11｜2026-07-18｜周六｜5h

主题：多请求 Sequential。

任务：
1. 多个请求按顺序处理。
2. 计算 latency、queue_time、TTFT。

产出 / 验收：
- 输出每个请求的结果

## Day 12｜2026-07-19｜周日｜5h

主题：取消与超时。

任务：
1. workload 支持 cancel_time、timeout。
2. 请求在任意阶段可变为 CANCELLED/TIMEOUT。

产出 / 验收：
- 异常状态测试通过

## Day 13｜2026-07-20｜周一｜3h

主题：Rejected 与输入校验。

任务：
1. 加入 max_prompt_len 或 max_total_tokens 限制。
2. 超过限制标记 REJECTED。

产出 / 验收：
- 非法/过大请求被拒绝

## Day 14｜2026-07-21｜周二｜3h

主题：第2周整理。

任务：
1. 写 docs/blockserve_request_lifecycle.md。
2. 补状态机测试。
3. 清理日志。

产出 / 验收：
- week2_review 完成
- 生命周期能讲清楚


# 第 3 周

周目标：Paged KV Cache 模拟：BlockPool、Request Block Table、按需分配、释放、contiguous baseline。

## Day 15｜2026-07-22｜周三｜3h

主题：KV Cache 概念建模。

任务：
1. 写 docs/blockserve_kv_cache.md 初版。
2. 定义 block_size、total_blocks、token_to_block 规则。

产出 / 验收：
- 能解释 token 数如何映射到 block

## Day 16｜2026-07-23｜周四｜3h

主题：BlockPool。

任务：
1. 实现 BlockPool：allocate/free/free_count/used_count。
2. 写边界测试。

产出 / 验收：
- 重复释放/无 block 时行为正确

## Day 17｜2026-07-24｜周五｜3h

主题：RequestBlockTable。

任务：
1. 为每个 request 维护 block list。
2. 支持 allocate_for_request/free_request。

产出 / 验收：
- 多请求不共享 block

## Day 18｜2026-07-25｜周六｜5h

主题：按需分配。

任务：
1. 当 token 跨 block 边界时分配新 block。
2. 覆盖 prompt_len=33/block_size=16 场景。

产出 / 验收：
- 33 tokens 需要 3 blocks

## Day 19｜2026-07-26｜周日｜5h

主题：资源回收。

任务：
1. FINISHED/CANCELLED/TIMEOUT 后释放所有 blocks。
2. 写长循环测试。

产出 / 验收：
- 所有请求结束 used_blocks 回到 0

## Day 20｜2026-07-27｜周一｜3h

主题：Contiguous baseline。

任务：
1. 实现 contiguous cache 模拟：按 max_total_tokens 预留。
2. 与 paged 按需增长形成对照。

产出 / 验收：
- 相同 workload 能输出两种占用

## Day 21｜2026-07-28｜周二｜3h

主题：第3周整理。

任务：
1. 完善 KV Cache 文档。
2. 画一张文字/ASCII 映射图。
3. 补 README。

产出 / 验收：
- week3_review 完成


# 第 4 周

周目标：调度策略：Sequential、Fixed Batch、Continuous Batching、Prefill/Decode 分离、Chunked Prefill。

## Day 22｜2026-07-29｜周三｜3h

主题：SequentialScheduler。

任务：
1. 把之前 sequential 生命周期封装为策略。
2. 输出统一 metrics。

产出 / 验收：
- Sequential baseline 可独立运行

## Day 23｜2026-07-30｜周四｜3h

主题：FixedBatchScheduler。

任务：
1. 实现固定 batch 收集与处理。
2. 模拟短请求等待长请求。

产出 / 验收：
- 能观察 batch padding/等待

## Day 24｜2026-07-31｜周五｜3h

主题：ContinuousBatchScheduler v0。

任务：
1. 每轮 decode 优先。
2. 引入 token_budget。
3. 剩余预算加入 prefill。

产出 / 验收：
- 多请求可交错执行

## Day 25｜2026-08-01｜周六｜5h

主题：Prefill/Decode 分离。

任务：
1. 维护 prefill queue 和 decode queue。
2. prefill 完成进入 decode。

产出 / 验收：
- 长 prompt 不完全阻塞 decode

## Day 26｜2026-08-02｜周日｜5h

主题：Chunked Prefill。

任务：
1. 将长 prompt 拆 chunk。
2. 对比 non-chunked。

产出 / 验收：
- TTFT/TPOT 变化可记录

## Day 27｜2026-08-03｜周一｜3h

主题：Paged Cache 接入调度。

任务：
1. continuous batching 每生成 token 更新 block。
2. block 不足时 reject 或等待。

产出 / 验收：
- 调度与 KV 联动

## Day 28｜2026-08-04｜周二｜3h

主题：第4周整理。

任务：
1. 写 docs/blockserve_scheduler.md。
2. 补三种策略说明和测试。

产出 / 验收：
- week4_review 完成


# 第 5 周

周目标：Benchmark 与报告：workload 生成、批量运行、指标统计、图表、阶段性 BlockServe-Sim v0.1。

## Day 29｜2026-08-05｜周三｜3h

主题：workload 生成器。

任务：
1. 写 scripts/generate_workload.py。
2. 生成 short_chat/mixed/long_prompt。

产出 / 验收：
- 随机种子可复现

## Day 30｜2026-08-06｜周四｜3h

主题：tight memory workload。

任务：
1. 设计显存受限 block 配置。
2. 让 contiguous/paged 出现差异。

产出 / 验收：
- 能触发 reject 或排队

## Day 31｜2026-08-07｜周五｜3h

主题：benchmark runner。

任务：
1. 写 scripts/run_benchmark.py。
2. 批量跑 scheduler/cache 组合。

产出 / 验收：
- 一条命令跑完整组

## Day 32｜2026-08-08｜周六｜5h

主题：指标统计。

任务：
1. 统计 completed/rejected/avg/P95/TTFT/TPOT/tokens/s/peak_blocks。

产出 / 验收：
- 输出 CSV

## Day 33｜2026-08-09｜周日｜5h

主题：图表。

任务：
1. 画完成数、P95 latency、peak block、reject 对比图。

产出 / 验收：
- 至少 4 张图

## Day 34｜2026-08-10｜周一｜3h

主题：Benchmark 报告。

任务：
1. 写 docs/blockserve_benchmark.md。
2. 说明环境、配置、workload、结果。

产出 / 验收：
- 别人可复现

## Day 35｜2026-08-11｜周二｜3h

主题：BlockServe-Sim v0.1。

任务：
1. 整理 README、命令、限制、下一步。
2. 打 tag 可选。

产出 / 验收：
- BlockServe-Sim 可作为简历雏形


# 第 6 周

周目标：真实小模型推理准备：llama.cpp/Metal 基准、模型格式理解、BlockServe CPU 后端最小化，不硬写完整 1B Runtime。

## Day 36｜2026-08-12｜周三｜3h

主题：1B 推理基线：llama.cpp。

任务：
1. 在 Mac 上构建 llama.cpp，优先启用 Metal；如不可行用 CPU。
2. 下载/准备 Qwen2.5-0.5B GGUF 或 TinyLlama 1.1B GGUF。

产出 / 验收：
- 能用基准工具跑一个小模型

## Day 37｜2026-08-13｜周四｜3h

主题：显存/内存测量。

任务：
1. 记录模型文件大小、上下文长度、运行内存、tokens/s。
2. 比较 0.5B 与 1.1B 的可行性。

产出 / 验收：
- docs/local_1b_inference_baseline.md

## Day 38｜2026-08-14｜周五｜3h

主题：模型结构阅读。

任务：
1. 阅读 Qwen/Llama decoder-only 架构字段。
2. 记录 embedding、RMSNorm、RoPE、QKV、MLP、lm_head。

产出 / 验收：
- docs/model_arch_notes.md

## Day 39｜2026-08-15｜周六｜5h

主题：BlockServe-Real 边界定义。

任务：
1. 明确 12 周内自研 Runtime 不完整支持 1B。
2. 定义 CPU-only TinyGPT 推理路径作为可完成目标。

产出 / 验收：
- docs/blockserve_real_scope.md

## Day 40｜2026-08-16｜周日｜5h

主题：Tokenizer/权重格式预研。

任务：
1. 了解 GGUF/safetensors/自定义 weights.bin 的差异。
2. 决定先用 Python 导出自定义权重。

产出 / 验收：
- docs/weight_format_notes.md

## Day 41｜2026-08-17｜周一｜3h

主题：CPU Tensor 最小封装。

任务：
1. 实现极简 CPU tensor 或先用 vector<float> 辅助。
2. 只支持后续 TinyGPT 需要的形状。

产出 / 验收：
- 最小单元测试通过

## Day 42｜2026-08-18｜周二｜3h

主题：第6周整理。

任务：
1. 整理 1B 基线数据。
2. 写结论：能跑基准，不把自研 1B 作为短期硬目标。

产出 / 验收：
- week6_review 完成


# 第 7 周

周目标：CUDA Kernel Lab 或远程 GPU 替代路径：vector add、RMSNorm、RoPE；如果无 CUDA 环境，完成 CPU reference + CUDA 文档与远程脚本。

## Day 43｜2026-08-19｜周三｜3h

主题：CUDA 环境确认。

任务：
1. 寻找远程 NVIDIA GPU/Colab/实验室机器。
2. 若没有，写 CUDA 代码但本地不运行，完成 CPU reference。

产出 / 验收：
- 明确 CUDA 路线

## Day 44｜2026-08-20｜周四｜3h

主题：Vector Add。

任务：
1. 建立 cuda-kernel-lab 结构。
2. 写 vector add kernel 或 CUDA 源文件草稿。
3. CPU reference。

产出 / 验收：
- 可在 CUDA 环境运行或代码可编译计划明确

## Day 45｜2026-08-21｜周五｜3h

主题：CUDA 内存与计时。

任务：
1. 学习 cudaMalloc/cudaMemcpy/cudaEvent。
2. 记录笔记。

产出 / 验收：
- docs/cuda_basics.md

## Day 46｜2026-08-22｜周六｜5h

主题：RMSNorm reference。

任务：
1. Python/C++ CPU reference。
2. 测试数据生成。

产出 / 验收：
- RMSNorm 数学可解释

## Day 47｜2026-08-23｜周日｜5h

主题：RMSNorm CUDA。

任务：
1. 实现 naive RMSNorm kernel。
2. 如无环境，写代码 + expected validation plan。

产出 / 验收：
- README 说明验证方式

## Day 48｜2026-08-24｜周一｜3h

主题：RoPE reference。

任务：
1. 写 RoPE CPU/Python reference。
2. 准备输入数据。

产出 / 验收：
- RoPE 作用可解释

## Day 49｜2026-08-25｜周二｜3h

主题：第7周整理。

任务：
1. 整理 cuda-kernel-lab README。
2. 明确远程运行清单。

产出 / 验收：
- week7_review 完成


# 第 8 周

周目标：ClusterPilot-Lite：GPU 集群调度模拟器，完成 FIFO、Bin Packing、Gang Scheduling 与报告。

## Day 50｜2026-08-26｜周三｜3h

主题：Cluster 建模。

任务：
1. 定义 Node/GPU/Job。
2. 写 cluster config 和 job workload。

产出 / 验收：
- 能加载虚拟集群

## Day 51｜2026-08-27｜周四｜3h

主题：FIFO 调度。

任务：
1. 有资源启动 job，不足等待，完成释放。

产出 / 验收：
- FIFO 结果可读

## Day 52｜2026-08-28｜周五｜3h

主题：GPU utilization 指标。

任务：
1. 统计利用率、queue time、completed jobs。

产出 / 验收：
- 结果 CSV

## Day 53｜2026-08-29｜周六｜5h

主题：Bin Packing。

任务：
1. 优先选择最合适节点。
2. 统计碎片率。

产出 / 验收：
- 与 FIFO 对比

## Day 54｜2026-08-30｜周日｜5h

主题：Gang Scheduling。

任务：
1. job 要么拿到全部 GPU 要么等待。
2. 设计大任务 workload。

产出 / 验收：
- 大任务不会部分占用

## Day 55｜2026-08-31｜周一｜3h

主题：实验与图表。

任务：
1. 跑 FIFO/BinPacking/Gang。
2. 画 2 张图。

产出 / 验收：
- clusterpilot_report 初版

## Day 56｜2026-09-01｜周二｜3h

主题：第8周整合。

任务：
1. 整理 README。
2. 写 week8_review。
3. 更新总 README。

产出 / 验收：
- 原 8 周项目组合 v0.1


# 第 9 周

周目标：TinyGPT 学习与 char-level GPT：阅读 minGPT/nanoGPT，训练一个 <1M~3M 参数的 character-level GPT。

## Day 57｜2026-09-02｜周三｜3h

主题：minGPT/nanoGPT 阅读。

任务：
1. 读 minGPT 核心文件和 nanoGPT train.py。
2. 记录训练循环、模型结构、生成流程。

产出 / 验收：
- docs/tinygpt_reading_notes.md

## Day 58｜2026-09-03｜周四｜3h

主题：char-level 数据集。

任务：
1. 准备 tiny shakespeare 或小中文语料。
2. 实现字符级 tokenizer。

产出 / 验收：
- 能 encode/decode

## Day 59｜2026-09-04｜周五｜3h

主题：TinyGPT 最小模型。

任务：
1. 用 PyTorch 搭建 <1M 参数 GPT。
2. 跑 forward 和 loss。

产出 / 验收：
- loss 可计算

## Day 60｜2026-09-05｜周六｜5h

主题：训练循环。

任务：
1. 实现 train/eval loop。
2. 保存 checkpoint。

产出 / 验收：
- 能跑 100~500 step

## Day 61｜2026-09-06｜周日｜5h

主题：生成文本。

任务：
1. 实现 generate。
2. 记录不同 temperature/top-k 的输出。

产出 / 验收：
- 能生成可读风格文本

## Day 62｜2026-09-07｜周一｜3h

主题：训练记录。

任务：
1. 画 train/val loss 曲线。
2. 整理超参数。

产出 / 验收：
- docs/tinygpt_char_report.md

## Day 63｜2026-09-08｜周二｜3h

主题：第9周整理。

任务：
1. 把 TinyGPT 学习结果写入 README。
2. 清理实验脚本。

产出 / 验收：
- week9_review 完成


# 第 10 周

周目标：自研 TinyGPT-Train：自己实现训练脚本，目标 10M 参数级模型，保存 checkpoint，生成文本。

## Day 64｜2026-09-09｜周三｜3h

主题：自研 TinyGPT-Train 骨架。

任务：
1. 新建 tinygpt-train/。
2. 不用照抄 nanoGPT，自己组织 config/model/train。

产出 / 验收：
- 项目可运行

## Day 65｜2026-09-10｜周四｜3h

主题：10M 模型配置。

任务：
1. 估算 n_layer/n_head/n_embd 参数量。
2. 定义 5M/10M 两档配置。

产出 / 验收：
- docs/param_count_notes.md

## Day 66｜2026-09-11｜周五｜3h

主题：训练 10M v0。

任务：
1. 跑短训练，验证显存/内存/速度。
2. 优先 CPU/MPS/云 GPU 之一。

产出 / 验收：
- 能完成短训练

## Day 67｜2026-09-12｜周六｜5h

主题：Checkpoint 管理。

任务：
1. 保存 model_config、state_dict、tokenizer meta。
2. 恢复训练。

产出 / 验收：
- 断点恢复测试

## Day 68｜2026-09-13｜周日｜5h

主题：采样与评估。

任务：
1. 生成样本文本。
2. 记录 loss，不追求效果。

产出 / 验收：
- sample_outputs.md

## Day 69｜2026-09-14｜周一｜3h

主题：10M 报告。

任务：
1. 写训练环境、数据、超参、loss、限制。

产出 / 验收：
- docs/tinygpt_10m_report.md

## Day 70｜2026-09-15｜周二｜3h

主题：第10周整理。

任务：
1. 清理 TinyGPT-Train。
2. 写 README。

产出 / 验收：
- week10_review 完成


# 第 11 周

周目标：10M~50M 模型训练与权重导出：稳定训练、记录 loss、导出权重，准备 BlockServe 接入。

## Day 71｜2026-09-16｜周三｜3h

主题：10M 稳定训练。

任务：
1. 延长训练或改良数据处理。
2. 记录更稳定 loss。

产出 / 验收：
- 可展示 checkpoint

## Day 72｜2026-09-17｜周四｜3h

主题：权重导出格式。

任务：
1. 设计 model.json + weights.bin/safetensors 导出。
2. 先导出 embedding/lm_head/一层权重。

产出 / 验收：
- 导出脚本初版

## Day 73｜2026-09-18｜周五｜3h

主题：BlockServe CPU 加载器。

任务：
1. BlockServe 读取 model.json 和权重文件。
2. 不急着完整 forward。

产出 / 验收：
- 能读取配置和权重形状

## Day 74｜2026-09-19｜周六｜5h

主题：CPU forward v0。

任务：
1. 实现 TinyGPT 单层或完整小模型 forward 的 CPU 路径。
2. 对齐 PyTorch logits。

产出 / 验收：
- 小输入误差可测

## Day 75｜2026-09-20｜周日｜5h

主题：生成路径。

任务：
1. BlockServe CPU backend 进行 greedy generate。
2. 接入自训练 checkpoint。

产出 / 验收：
- 能生成 token/text

## Day 76｜2026-09-21｜周一｜3h

主题：50M 可行性实验。

任务：
1. 评估 50M 配置训练速度和内存。
2. 决定是否继续。

产出 / 验收：
- docs/tinygpt_50m_feasibility.md

## Day 77｜2026-09-22｜周二｜3h

主题：第11周整理。

任务：
1. 整理导出器、CPU 推理文档。
2. 列出未完成 forward/性能问题。

产出 / 验收：
- week11_review 完成


# 第 12 周

周目标：BlockServe 接入自训练模型 + 100M 预研：CPU 推理路径、权重加载、生成验证、100M 训练可行性报告、最终文档。

## Day 78｜2026-09-23｜周三｜3h

主题：1B 推理复盘。

任务：
1. 整理 llama.cpp 0.5B/1.1B 基线。
2. 明确自研 Runtime 到 1B 的缺口：算子、量化、权重格式、Metal/CUDA。

产出 / 验收：
- docs/1b_runtime_gap.md

## Day 79｜2026-09-24｜周四｜3h

主题：BlockServe + TinyGPT 演示。

任务：
1. 写一键脚本：训练小模型或加载 checkpoint → 导出 → BlockServe 推理。

产出 / 验收：
- demo 脚本可运行

## Day 80｜2026-09-25｜周五｜3h

主题：100M 训练预研。

任务：
1. 估算参数、数据 token、训练时间、硬件需求。
2. 给出本地/云 GPU 两套方案。

产出 / 验收：
- docs/100m_training_plan.md

## Day 81｜2026-09-26｜周六｜5h

主题：最终 Benchmark/报告补齐。

任务：
1. 更新 BlockServe-Sim、TinyGPT、ClusterPilot 图表和数据。

产出 / 验收：
- 报告一致

## Day 82｜2026-09-27｜周日｜5h

主题：简历化表达。

任务：
1. 写 resume bullets：BlockServe、TinyGPT、ClusterPilot。
2. 准备面试 Q&A。

产出 / 验收：
- docs/resume_bullets.md

## Day 83｜2026-09-28｜周一｜3h

主题：最终复现。

任务：
1. 从零运行主要命令。
2. 修文档链接和脚本路径。

产出 / 验收：
- 仓库可复现

## Day 84｜2026-09-29｜周二｜3h

主题：最终验收与下一阶段。

任务：
1. 写 docs/final_12week_review.md。
2. 打 tag v0.2-12week-plan。
3. 列第13~16周计划。

产出 / 验收：
- 12周版本完成

---

## 4. 资料阅读顺序

### BlockServe / LLM Serving
1. vLLM Documentation：先看 PagedAttention、continuous batching、chunked prefill、prefix caching 的概念，不读深层源码。
2. PagedAttention paper：理解 KV Cache fragmentation 和 paging 思想。
3. llama.cpp：作为本地 0.5B~1.1B 量化模型推理基线，不作为当前自研 Runtime 的照抄对象。

### C++ / CUDA
1. CMake 官方教程：只学 target、include、build、test。
2. C++ Primer：只看 STL、类、RAII、引用、optional、文件 IO。
3. CUDA C++ Programming Guide：只看 kernel launch、thread hierarchy、global memory、cudaMalloc/cudaMemcpy/cudaEvent。

### TinyGPT / 训练
1. minGPT：理解 GPT 从零实现。
2. nanoGPT：理解训练脚本、数据加载、checkpoint、生成。
3. llm.c：后期参考 C/CUDA 训练系统，不在前 8 周深入。
4. LitGPT：作为工程化训练、微调、部署 recipe 的参考。

### 模型选择
1. 第 6 周本地推理基线：优先 Qwen2.5-0.5B GGUF，其次 TinyLlama-1.1B GGUF。
2. 第 9~11 周训练：先 char-level TinyGPT，再 10M 参数模型。
3. 第 12 周 100M：只做可行性评估，不作为必须完成训练目标。

---

## 5. 参考资料

- vLLM Documentation: https://docs.vllm.ai/en/stable/
- Efficient Memory Management for Large Language Model Serving with PagedAttention: https://arxiv.org/abs/2309.06180
- llama.cpp: https://github.com/ggml-org/llama.cpp
- CUDA C++ Programming Guide: https://docs.nvidia.com/cuda/cuda-programming-guide/index.html
- nanoGPT: https://github.com/karpathy/nanogpt
- minGPT: https://github.com/karpathy/minGPT
- llm.c: https://github.com/karpathy/llm.c
- LitGPT: https://github.com/Lightning-AI/litgpt
- Qwen2.5-0.5B: https://huggingface.co/Qwen/Qwen2.5-0.5B
- Qwen2.5-0.5B-Instruct-GGUF: https://huggingface.co/Qwen/Qwen2.5-0.5B-Instruct-GGUF
- TinyLlama: https://github.com/jzhang38/TinyLlama
- Chinchilla Scaling Laws / 20 tokens per parameter 参考: https://epoch.ai/publications/chinchilla-scaling-a-replication-attempt

---

## 6. 风险控制

### 如果 C++ 进度慢
- 暂缓 ClusterPilot-Lite 的 Bin Packing，只保留 FIFO + Gang Scheduling。
- 不做复杂模板、协程、多线程、锁自由结构。

### 如果 KV Cache 卡住
- 砍掉 contiguous baseline，先保证 BlockPool + RequestBlockTable 正确。

### 如果 continuous batching 卡住
- 先实现 decode 优先 + token budget，不强行完成 chunked prefill。

### 如果 CUDA 环境不可用
- CUDA Kernel Lab 保留 CPU reference + CUDA 源码 + 远程运行脚本。
- 第 6 周的 1B 基线使用 llama.cpp/Metal 或 CPU，不依赖 CUDA。

### 如果 1B 自研 Runtime 难度过高
- 用 llama.cpp 作为 1B 推理基线。
- BlockServe 自研 Runtime 只接入 TinyGPT 10M 模型。
- 在文档中明确差距，不伪装成果。

### 如果 10M 训练速度慢
- 缩小到 3M~5M，保证训练链路完整。
- 10M 作为配置和短训练结果保留。

### 如果 100M 不现实
- 只完成参数、数据、显存、训练时长估算，不启动完整训练。

---

## 7. 最低可接受成果

12 周最低版本必须达到：

```text
BlockServe-Sim:
- 请求状态机
- Paged KV Cache block pool
- Sequential vs Continuous Batching
- 一组 benchmark
- 一篇完整报告

1B 推理基线:
- 使用 llama.cpp/Metal 或 CPU 跑通 0.5B~1.1B 量化模型
- 记录 tokens/s、内存、上下文长度

CUDA Kernel Lab:
- vector add
- RMSNorm reference 或 CUDA kernel

ClusterPilot-Lite:
- FIFO
- Gang Scheduling
- 简单 GPU utilization 统计

TinyGPT-Train:
- char-level GPT 可训练
- 10M 级模型短训练或稳定训练
- checkpoint 保存

BlockServe 接入:
- CPU 路径加载自训练 TinyGPT 权重
- greedy generate 可运行

100M 预研:
- 完成可行性报告，不要求训练完成
```

---

## 8. 简历阶段性表达

完成 12 周后，简历可以写成：

```text
AI Infra Lab：围绕 LLM 推理运行时、KV Cache 管理、GPU 调度和小模型训练构建系统实验项目。
- 使用 C++ 实现 BlockServe-Sim，支持请求状态机、Paged KV Cache 模拟、token-level continuous batching 与 benchmark 对比。
- 构建本地 0.5B~1.1B 量化模型推理基线，记录受限显存/内存环境下的 tokens/s、上下文长度与资源占用。
- 实现 CUDA Kernel Lab 或 CPU reference，覆盖 RMSNorm、RoPE 等 Transformer 基础算子，并记录正确性验证方式。
- 实现 ClusterPilot-Lite GPU 调度模拟器，支持 FIFO、Bin Packing/Gang Scheduling 和 GPU utilization 统计。
- 从零实现 TinyGPT-Train，完成 char-level GPT 与 10M 级模型训练、checkpoint 保存和 BlockServe CPU 推理接入。
```
