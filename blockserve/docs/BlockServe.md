# BlockServe 项目说明书 v2

## 0. 执行假设

**项目名称**：BlockServe  
**项目类型**：面向受限显存与异构设备的 LLM 推理运行时 / 推理系统实验项目  
**主语言**：C++20  
**辅助语言**：Python  
**目标岗位**：GPU Runtime、AI 推理引擎、AI Infra、异构计算、性能工程、国产 GPU 软件栈

可用硬件：

- MacBook：日常 C++ 开发、CPU 模拟、Python 原型和文档。
- NVIDIA RTX 3050 4GB：CUDA kernel、4GB 显存约束实验、小模型推理基线。
- AMD 200GB 显存资源，约 300 小时：PyTorch/ROCm 训练、较大模型验证、后期 HIP/ROCm 可行性实验。

当前能力假设：

- 已有 C++ 基础，但 C++ 工程、测试、RAII、optional、CMake、CUDA、模型推理系统仍在补。
- 因此 BlockServe 不从一开始追求完整 vLLM 级框架，而采用“模拟器 -> 小 kernel -> TinyGPT -> 外部 1B 基线 -> 后续真实 Runtime”的路线。

---

## 1. 项目定位

BlockServe 不是聊天应用、OpenAI API 包装、RAG/Agent 框架，也不是 vLLM/TensorRT-LLM 的低配复刻。

它要解决的是：

> 在受限显存与多请求场景下，如何管理请求生命周期、KV Cache、调度策略和推理执行路径，并用实验数据说明这些设计带来的影响。

核心能力：

- 理解自回归推理；
- 理解 prefill、decode、KV Cache；
- 理解请求调度、batching、显存预算；
- 用 C++ 实现稳定的运行时控制逻辑；
- 用 Python/PyTorch 做训练、导出、正确性参考；
- 在 NVIDIA CUDA 与 AMD ROCm 资源上做分层验证；
- 用 benchmark 和文档解释项目，而不是只展示截图。

---

## 2. BlockServe 要回答的问题

### 2.1 KV Cache 为什么是核心资源

自回归模型生成每个 token 时，需要保留之前 token 的 Key/Value。上下文越长、并发请求越多，KV Cache 占用越大。错误的管理方式会导致过度预留、显存碎片、OOM、资源释放不及时以及静态 batch 浪费。

### 2.2 请求为什么不能简单 FIFO 跑完

真实推理服务中，请求到达时间、prompt 长度、输出长度不同。一个长 prompt 请求如果独占 GPU，会拖慢已经进入 decode 阶段的短请求。BlockServe 需要验证 prefill/decode 分离、decode 优先、token budget、chunked prefill 等策略的价值。

### 2.3 4GB 显存机器的意义

RTX 3050 4GB 不能支撑大规模模型服务，但适合验证：

- 受限显存下的拒绝、排队、回收策略；
- 小模型真实推理；
- CUDA kernel 正确性；
- 低预算 KV Cache 压力实验；
- 量化 0.5B~1B 模型外部基线。

### 2.4 AMD 200GB 资源的作用

AMD 200GB 显存资源不应一开始用于重写 HIP Runtime。优先用途：

- 使用 PyTorch/ROCm 训练 10M~100M TinyGPT；
- 跑更大 batch 的训练实验；
- 验证 0.5B~1B 开源模型的推理基线；
- 后期做 HIP/ROCm 后端可行性研究。

### 2.5 1B 模型目标如何处理

BlockServe 的 1B 目标分两层：

1. **外部基线**：用 llama.cpp、PyTorch、Transformers、vLLM 或 ROCm/CUDA 现有生态跑 0.5B~1B 模型，记录显存、速度和限制。
2. **自研 Runtime 路径**：优先支持自己训练的 1M~10M TinyGPT，再考虑 100M 模型；不把 1B 完整接入自研 Runtime 作为 12 周硬目标。

---

## 3. 目标与非目标

### 3.1 核心目标

最终应形成以下能力：

1. C++ 请求生命周期与状态机；
2. workload trace 输入；
3. arrival-time simulation；
4. prefill/decode 状态推进；
5. Paged KV Cache 模拟；
6. contiguous cache 与 paged cache 对照；
7. sequential、fixed batch、continuous batching 对照；
8. benchmark 指标与图表；
9. CUDA kernel lab：vector add、RMSNorm、RoPE、sampling 或简单 attention；
10. TinyGPT 训练、权重导出、C++ CPU 推理接入；
11. 0.5B~1B 开源模型外部推理基线；
12. 100M 训练可行性报告或小规模验证。

### 3.2 明确非目标

前 12 周不做：

- 完整 vLLM；
- 完整 TensorRT-LLM；
- 通用 Hugging Face 模型加载器；
- 生产级 OpenAI API；
- 多机多卡训练或推理；
- MoE、多模态、复杂量化系统；
- 复杂前端 UI；
- Agent/RAG/工作流；
- 直接从零训练 1B 可用模型；
- 自研通用 Tensor Library；
- 自研 GEMM 替代 cuBLAS/rocBLAS。

---

## 4. 项目分层

### 4.1 Simulation Layer

目标：不依赖模型和 GPU，验证调度和资源规则。

模块：

- Request；
- RequestQueue；
- WorkloadLoader；
- Simulator；
- Scheduler；
- KV Block Pool；
- MetricsRecorder；
- Benchmark Runner。

这是前 8 周最重要的层。

### 4.2 Kernel Layer

目标：用 CUDA 学习真实 GPU 编程，不急着接入完整 runtime。

模块：

- vector add；
- RMSNorm；
- RoPE；
- argmax/sampling；
- 可选：简单 attention；
- correctness reference；
- cudaEvent timing；
- Nsight 记录。

3050 4GB 是这一层的主设备。

### 4.3 Tiny Model Layer

目标：自己训练一个小 GPT，并让 BlockServe 读它、推理它。

模块：

- TinyGPT PyTorch 训练；
- 1M 原型；
- 10M 主模型；
- checkpoint 保存；
- 权重导出；
- C++ CPU 推理；
- greedy decode；
- logits correctness。

MacBook 与 AMD 200GB 资源都可使用，AMD 优先承担训练。

### 4.4 External Baseline Layer

目标：跑 0.5B~1B 开源模型，作为现实推理资源基线。

可选工具：

- llama.cpp + GGUF；
- PyTorch/Transformers；
- vLLM；
- ROCm/HIP 生态；
- CUDA 生态。

该层不要求 BlockServe 直接加载这些模型。它负责给 BlockServe 后续设计提供参照数据。

---

## 5. 功能优先级

### P0：C++ 工程骨架与请求模拟

必须完成：

- CMake 工程；
- 目录结构；
- RequestStatus；
- Request；
- RequestQueue；
- WorkloadLoader；
- JSONL 输入；
- arrival-time simulation；
- 基础日志；
- README 和日常记录。

验收：

- 能从 workload 文件读取请求；
- 请求按 arrival_time 进入系统；
- 错误 workload 能输出明确错误。

### P1：请求生命周期与基础指标

必须完成：

- WAITING -> PREFILLING -> DECODING -> FINISHED；
- CANCELLED / TIMEOUT / REJECTED；
- first token time；
- finish time；
- queue time；
- latency；
- TTFT；
- TPOT；
- CSV/JSON 指标输出。

验收：

- 一个请求能完整走完生命周期；
- 多请求能按时间进入并被处理；
- 异常状态能释放资源；
- 输出指标可复查。

### P2：Paged KV Cache 模拟

必须完成：

- total_blocks；
- block_size；
- BlockPool；
- RequestBlockTable；
- 按需分配；
- 请求完成/取消/超时释放；
- block 不足时 reject 或等待；
- contiguous cache baseline；
- paged cache 对照。

验收：

- prompt_len=33、block_size=16 时需要 3 个 block；
- 所有请求结束后 used_blocks 回到 0；
- paged cache 与 contiguous cache 在受限场景下出现可解释差异。

### P3：调度策略与 Continuous Batching

必须完成：

- SequentialScheduler；
- FixedBatchScheduler；
- ContinuousBatchScheduler；
- decode 优先；
- token budget；
- prefill/decode 分离；
- 可选 chunked prefill；
- 调度指标输出。

验收：

- 相同 workload 下能比较三种策略；
- 能解释 fixed batch 的等待浪费；
- 能解释 continuous batching 如何降低等待或提高完成数；
- 至少有一组 tight memory workload 体现 paged cache 价值。

### P4：Benchmark 与实验报告

必须完成：

- workload generator；
- benchmark runner；
- result CSV；
- plot scripts；
- short_chat；
- mixed_length；
- long_prompt；
- tight_memory；
- benchmark report。

验收：

- 一条命令能重跑主要实验；
- 至少 4 张图；
- 报告包含环境、配置、指标、结果、限制；
- 不夸大结论。

### P5：CUDA Kernel Lab

必须完成：

- 3050 4GB CUDA 环境确认；
- vector add；
- RMSNorm；
- RoPE 或 sampling；
- CPU/Python reference；
- correctness；
- timing；
- README。

验收：

- 至少两个 LLM 相关 kernel；
- 能说明 kernel launch、grid、block、thread；
- 能说明 RMSNorm/RoPE 的模型作用；
- 能记录 max error、mean error、runtime。

### P6：TinyGPT-Train 与 BlockServe CPU 推理接入

必须完成：

- 阅读 minGPT/nanoGPT；
- PyTorch 实现 TinyGPT；
- char-level 或 BPE 简化训练；
- 1M 模型跑通；
- 10M 模型训练；
- checkpoint 保存；
- 导出 BlockServe 可读权重；
- C++ CPU 推理；
- greedy decode；
- logits 对照。

验收：

- 10M 级模型训练完成或有明确训练记录；
- C++ 能加载导出权重；
- 同一输入下 C++ CPU logits 与 Python reference 接近；
- 能生成文本；
- 文档记录训练配置和限制。

### P7：1B 外部推理基线与 100M 预研

必须完成：

- 选择 0.5B~1B 开源小模型；
- 在 3050 4GB 或 AMD 200GB 上跑外部推理；
- 记录显存、速度、上下文长度；
- 评估 100M 从零训练所需数据、时间和显存；
- 写 100M feasibility report。

验收：

- 有一份真实外部基线数据；
- 有一份 100M 训练评估；
- 明确后续是否值得进入 100M 训练。

---

## 6. 推荐技术栈

### C++ 工程

- C++20；
- CMake；
- Ninja 可选；
- Catch2 或 GoogleTest；
- fmt 可选；
- spdlog 可选；
- clang-format；
- clang-tidy 可选。

### Python 工具链

- Python 3.10+ 优先；
- PyTorch；
- Transformers；
- NumPy；
- Matplotlib；
- pytest；
- safetensors 可选；
- sentencepiece/tokenizers 可选。

### NVIDIA CUDA 路线

- CUDA Runtime API；
- cudaEvent；
- Nsight Systems；
- Nsight Compute；
- cuBLAS/cuBLASLt 后期再引入；
- 3050 4GB 用于 correctness 和小规模 benchmark。

### AMD ROCm 路线

- PyTorch ROCm；
- HIP/ROCm 文档；
- rocBLAS 后期再研究；
- AMD 200GB 资源优先用于训练和外部基线；
- 自研 HIP 后端为长期扩展，不作为前 12 周硬目标。

### 外部基线工具

- llama.cpp / GGUF；
- vLLM；
- PyTorch/Transformers；
- LitGPT；
- nanoGPT；
- llm.c 后期阅读。

---

## 7. 输入输出规格

### Workload JSONL

```json
{"id":1,"arrival_time":0,"prompt_len":64,"max_new_tokens":32}
{"id":2,"arrival_time":3,"prompt_len":512,"max_new_tokens":64,"timeout":1000}
{"id":3,"arrival_time":5,"prompt_len":128,"max_new_tokens":16,"cancel_time":40}
```

必需字段：

- arrival_time；
- prompt_len；
- max_new_tokens。

可选字段：

- id；
- timeout；
- cancel_time；
- priority；
- request_type。

### Config JSON

```json
{
  "scheduler": "continuous",
  "cache_policy": "paged",
  "block_size": 16,
  "total_blocks": 1024,
  "max_batch_tokens_per_step": 128,
  "max_prefill_tokens_per_step": 64,
  "prefill_cost_per_token": 2,
  "decode_cost_per_token": 1
}
```

### Benchmark CSV

```csv
scheduler,cache_policy,workload,completed,rejected,avg_latency,p95_latency,avg_ttft,p95_ttft,tokens_per_second,peak_used_blocks,cache_utilization
continuous,paged,mixed,98,2,120.5,300.0,25.1,80.0,240.0,900,0.88
```

### TinyGPT 权重导出

第一版使用自定义简单格式：

```text
model.json
weights.bin
vocab.json 或 chars.txt
```

不要求一开始支持完整 safetensors。

---

## 8. 实验设计

### 调度策略实验

对比：

- sequential；
- fixed batch；
- continuous batching。

指标：

- completed；
- rejected；
- avg latency；
- P95 latency；
- TTFT；
- TPOT；
- tokens/s。

### KV Cache 实验

对比：

- contiguous cache；
- paged cache。

指标：

- peak blocks；
- rejected requests；
- cache utilization；
- OOM/reject 次数；
- 完成请求数。

### 4GB 显存实验

目标：

- 小模型真实推理；
- 量化 0.5B~1B 外部基线；
- 显存占用；
- 短上下文与长上下文差异。

不要求：

- 高吞吐；
- 多并发大模型；
- 工业级性能。

### AMD 200GB 实验

目标：

- 10M TinyGPT 训练；
- 100M 训练预研；
- 外部 1B 模型推理基线；
- PyTorch/ROCm 环境记录。

不要求：

- 自研 HIP 推理后端；
- 大规模分布式训练。

---

## 9. 测试要求

### C++ 单元测试

覆盖：

- RequestStatus；
- RequestQueue；
- WorkloadLoader；
- Simulator；
- BlockPool；
- RequestBlockTable；
- MetricsRecorder；
- Scheduler；
- cache release；
- invalid workload。

### Python 测试

覆盖：

- workload generator；
- result parser；
- plotting script；
- TinyGPT forward；
- checkpoint save/load；
- export script；
- Python reference logits。

### Correctness

必须有：

- TinyGPT Python vs C++ CPU logits 对齐；
- contiguous vs paged 模拟结果一致性检查；
- CUDA kernel vs CPU reference 误差统计。

---

## 10. 风险控制

### 模型接入拖垮 Runtime 主线

控制：

- 前 8 周只做模拟器和 benchmark；
- TinyGPT 放到第 9 周后；
- 1B 模型先做外部基线，不直接接自研 Runtime。

### CUDA/HIP 环境消耗过多时间

控制：

- 3050 只做小 kernel；
- AMD 资源先跑 PyTorch/ROCm；
- HIP 后端只做文档预研或长期目标。

### 100M/1B 训练目标失控

控制：

- 10M 是强制目标；
- 100M 是预研或可选训练；
- 1B 从零训练不做；
- 1B 只做外部推理基线或微调预研。

---

## 11. 最终交付物

### 12 周版本必须包含

- BlockServe C++ 模拟器；
- Paged KV Cache 模拟；
- Continuous Batching 模拟；
- benchmark 脚本；
- benchmark 报告；
- CUDA Kernel Lab；
- TinyGPT-Train；
- 10M 模型训练记录；
- C++ CPU TinyGPT 推理；
- 0.5B~1B 外部基线报告；
- 100M feasibility report；
- README、运行说明、已知限制。

### 长期版本可扩展

- C++/CUDA 真正单请求推理；
- CUDA Paged Attention；
- cuBLASLt 接入；
- HTTP/SSE；
- safetensors loader；
- HIP/ROCm backend；
- 100M 模型训练；
- 1B 模型接入自研 Runtime；
- vLLM/llama.cpp 开源贡献。

---

## 12. 简历表达方向

可写成：

> 设计并实现 BlockServe，一个面向 LLM Serving 的 C++ 推理系统实验项目。完成请求生命周期、workload trace、Paged KV Cache 模拟、Sequential/Fixed Batch/Continuous Batching 调度对比和 benchmark 报告；使用 CUDA 实现 RMSNorm/RoPE 等基础 kernel；训练 10M TinyGPT 并实现权重导出与 C++ CPU 推理接入；在 4GB NVIDIA GPU 与 AMD ROCm 资源上记录小模型推理和训练实验。

---

## 13. 参考资料

- vLLM Documentation: https://docs.vllm.ai/
- vLLM PagedAttention paper: https://arxiv.org/abs/2309.06180
- CUDA Programming Guide: https://docs.nvidia.com/cuda/cuda-programming-guide/index.html
- ROCm HIP Documentation: https://rocm.docs.amd.com/projects/HIP/
- AMD ROCm Programming Guide: https://rocm.docs.amd.com/en/latest/how-to/programming_guide.html
- llama.cpp: https://github.com/ggml-org/llama.cpp
- minGPT: https://github.com/karpathy/minGPT
- nanoGPT: https://github.com/karpathy/nanoGPT
- llm.c: https://github.com/karpathy/llm.c
- LitGPT: https://github.com/Lightning-AI/litgpt
- FlashAttention paper: https://arxiv.org/abs/2205.14135
