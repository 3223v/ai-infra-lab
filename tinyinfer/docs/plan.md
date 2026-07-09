# BlockServe 真实推理优先计划

周期：8 周，56 天  
投入：工作日每天 3 小时，周六周日每天 5 小时，约 200 小时  
当前阶段目标：集中补齐 C++ 与真实 BlockServe 推理框架能力，在本地 RTX 3050 4GB 显存环境中成功运行一个开源 1B 以下 decoder-only 模型。

---

## 1. 计划重构原则

原计划的问题是：前半段大量时间投入到模拟器、记分板、图表和调度对比，真实模型推理被放得太晚。当前阶段不再以 BlockServe为主，而是直接围绕真实推理框架推进。

新的核心目标是：

```text
C++ 工程基础
→ Python 权重转换与 tokenizer 辅助
→ C++ 模型权重加载
→ CPU 正确性路径
→ CUDA backend
→ KV Cache
→ 本地 RTX 3050 4GB 跑通开源 1B 以下模型
```

本阶段不做：

- ClusterPilot；
- 10M / 100M 自训练模型；
- 大量图表；
- 大型 benchmark 报告；
- 完整 OpenAI API；
- 多模型通用兼容；
- 多机多卡；
- 复杂量化框架；
- 训练框架。

这些全部放到下一阶段。

---

## 2. 最终验收标准

8 周结束时，BlockServe 应至少达到：

```text
1. 能加载一个开源 1B 以下 decoder-only 模型；
2. 能在本地 RTX 3050 4GB 显存上运行；
3. 能输入 prompt 并生成文本；
4. 推理主路径在 C++ 中完成；
5. Python 只用于 tokenizer、权重转换、参考验证；
6. 支持 CUDA backend；
7. 支持基础 KV Cache decode；
8. 支持 greedy decode；
9. 有最小 README 和运行命令；
10. 有简单显存和 tokens/s 记录，但不做复杂图表。
```

最终演示命令目标：

```bash
python tools/run_prompt.py \
  --model-path models/qwen2_5_0_5b_bs \
  --prompt "你好，简单介绍一下你自己" \
  --max-new-tokens 32 \
  --device cuda
```

或者低层 C++ 命令：

```bash
./build/blockserve_cli \
  --model models/qwen2_5_0_5b_bs \
  --input-ids "151644,872,198" \
  --max-new-tokens 32 \
  --device cuda
```

---

## 3. 目标模型选择

### 主目标模型

优先选择：

```text
Qwen2.5-0.5B-Instruct
```

原因：

- 参数量低于 1B；
- fp16 权重大约 1GB 量级；
- RTX 3050 4GB 有机会承载；
- 架构具有代表性：RMSNorm、RoPE、Attention、MLP、LM Head；
- 后续可扩展到更大 Qwen/Llama-like 模型。

### 备选模型

如果 Qwen2.5-0.5B 适配难度过高，改用：

```text
SmolLM2-360M-Instruct
```

原因：

- 参数更小；
- 更适合调通真实推理链路；
- 仍然是开源小型 decoder-only LLM；
- 可以降低 4GB 显存和 C++ 实现压力。

### 不作为当前目标

```text
TinyLlama-1.1B
```

原因：它已经超过 1B，不符合当前“1B 以下模型”的硬目标，可以作为下一阶段或额外测试。

---

## 4. 技术路线

### 4.1 权重格式

不直接在 C++ 中解析 Hugging Face safetensors 或 GGUF。

当前阶段采用：

```text
Hugging Face checkpoint
→ Python 转换脚本
→ BlockServe 自定义格式
→ C++ Runtime 加载
```

建议格式：

```text
model.json
weights.bin
vocab/tokenizer 由 Python 侧处理
```

`model.json` 保存结构参数：

```json
{
  "model_type": "qwen2_like",
  "hidden_size": 896,
  "num_layers": 24,
  "num_attention_heads": 14,
  "num_kv_heads": 2,
  "head_dim": 64,
  "vocab_size": 151936,
  "dtype": "fp16"
}
```

`weights.bin` 按固定顺序存储 tensor。

### 4.2 Tokenizer 策略

当前阶段不在 C++ 中实现 BPE tokenizer。

使用 Python 包装：

```text
prompt string
→ Python tokenizer encode
→ C++ BlockServe 推理 token ids
→ Python tokenizer decode
→ 输出文本
```

这不削弱项目价值。当前阶段的核心是推理 runtime，不是 tokenizer 工程。

### 4.3 CUDA 策略

- Linear / QKV / MLP 使用 cuBLAS 或 cuBLASLt；
- RMSNorm 自写 CUDA kernel；
- RoPE 自写 CUDA kernel；
- Sampling / argmax 自写 CUDA kernel 或先 CPU 实现；
- Attention 第一版可以朴素实现，不追求 FlashAttention；
- KV Cache 第一版使用连续布局，不强制做 Paged KV Cache。

### 4.4 AMD 200GB 资源使用原则

当前 8 周主目标是 RTX 3050 4GB 本地运行，因此主 backend 是 CUDA。

AMD 200GB / 300 小时资源用于：

- PyTorch reference；
- 模型转换验证；
- 大显存 sanity check；
- 后续 HIP / ROCm backend 预留。

本阶段不实现 HIP backend。

---

## 5. 每日工作模板

### 工作日 3 小时

| 时间 | 内容 |
|---|---|
| 30 分钟 | 只读当天必须资料 |
| 120 分钟 | 写核心代码 |
| 30 分钟 | 编译、运行、修 bug |

### 周末 5 小时

| 时间 | 内容 |
|---|---|
| 45 分钟 | 复盘和补必要概念 |
| 210 分钟 | 核心开发 |
| 45 分钟 | 验证、整理、commit |

每日要求：

- 至少一次 commit；
- 至少跑一次程序；
- 文档只写当天必要记录，不写长报告；
- 不能只看资料不写代码。

---

# 第 1 周：工程重构与真实推理框架骨架

周目标：停止继续扩大模拟器，把仓库主线切到真实 BlockServe。完成 C++ 工程骨架、模型配置、Tensor 基础、权重格式设计和 Python 参考路径。

## Day 1｜2026-07-09｜周四｜3h

主题：冻结旧模拟器，创建真实 runtime 分支。

任务：

1. 保留当前已有代码，但不要继续扩展模拟器。
2. 新建真实 runtime 目录：

```text
blockserve/
├── include/blockserve/
├── src/
├── tools/
├── models/
├── tests/
└── CMakeLists.txt
```

3. 创建 `docs/current_scope.md`，写清楚当前 8 周目标：真实推理框架，不做 ClusterPilot 和训练。
4. 修改根 README，标注当前主线为 `blockserve/`。

验收：

```bash
cmake -B build
cmake --build build
./build/blockserve_cli --help
```

## Day 2｜2026-07-10｜周五｜3h

主题：C++ Runtime 基础类型。

任务：

1. 实现基础类型：`DataType`、`DeviceType`、`Shape`、`TensorMeta`。
2. 实现错误处理：`Status` 或 `Result<T>`。
3. 实现最小 CLI 参数解析。
4. 写 3 个基础测试。

验收：

- `blockserve_cli --help` 可运行；
- 基础类型测试通过；
- 不引入复杂模板。

## Day 3｜2026-07-11｜周六｜5h

主题：模型结构调研与目标模型确认。

任务：

1. 下载或准备 Qwen2.5-0.5B-Instruct 的配置文件。
2. 用 Python 打印模型结构字段：层数、hidden size、head 数、kv head 数、vocab size。
3. 记录每类权重名称和 shape。
4. 建立 `tools/inspect_hf_model.py`。
5. 写 `docs/qwen_model_shapes.md`。

验收：

- 能列出所有权重名称和 shape；
- 能判断需要实现哪些算子。

## Day 4｜2026-07-12｜周日｜5h

主题：BlockServe 自定义权重格式 v0。

任务：

1. 设计 `model.json + weights.bin`。
2. 写 `tools/export_debug_model.py`，先导出一个极小 debug 模型，不导出完整 0.5B。
3. C++ 实现 `ModelConfig` 读取。
4. C++ 实现 `WeightManifest` 或简单 tensor 索引。

验收：

- Python 能导出 debug model；
- C++ 能读取 `model.json`；
- C++ 能打印 tensor 名称、shape、offset。

## Day 5｜2026-07-13｜周一｜3h

主题：CPU Tensor 与权重加载。

任务：

1. 实现 `CpuTensor`，内部先用 `std::vector<float>` 或 `std::vector<uint16_t>`。
2. 实现从 `weights.bin` 读取 tensor。
3. 完成 debug tensor 加载测试。

验收：

- C++ 能加载一个 tensor 并打印前几个值；
- 和 Python 导出值一致。

## Day 6｜2026-07-14｜周二｜3h

主题：Python Reference 路径。

任务：

1. 写 `tools/reference_forward.py`。
2. 对 debug model 做一次 forward。
3. 保存输入 token ids、logits 到文件。
4. C++ 侧准备读取这些 reference 数据。

验收：

- Python reference 可生成 logits；
- 后续 C++ 可以用它做对齐。

## Day 7｜2026-07-15｜周三｜3h

主题：第 1 周收敛。

任务：

1. 清理 CMake。
2. 确认 `blockserve_cli`、`inspect_hf_model.py`、`export_debug_model.py` 可运行。
3. 写最小 README：如何构建，如何导出 debug model。

验收：

- 从空 build 目录可重新编译；
- debug model 可以导出并被 C++ 读取。

---

# 第 2 周：CPU 正确性路径，跑通 debug decoder-only 模型

周目标：先用小尺寸 debug model 跑通真实 Transformer forward，保证结构正确。不要碰完整 0.5B，也不要碰 CUDA。

## Day 8｜2026-07-16｜周四｜3h

主题：Embedding + LM Head。

任务：

1. 实现 embedding lookup。
2. 实现 CPU matmul。
3. 实现 lm_head logits。
4. 与 Python reference 对齐。

验收：

- 小输入 token ids 能得到 logits；
- 误差可统计。

## Day 9｜2026-07-17｜周五｜3h

主题：RMSNorm CPU。

任务：

1. 实现 RMSNorm CPU。
2. 与 Python reference 对齐。
3. 写测试覆盖 hidden size 不同情况。

验收：

- RMSNorm max error 在可解释范围内。

## Day 10｜2026-07-18｜周六｜5h

主题：RoPE CPU。

任务：

1. 实现 RoPE CPU。
2. 明确 q/k 的 shape 布局。
3. 与 Python reference 对齐。

验收：

- 同位置、同 head 的 RoPE 输出一致。

## Day 11｜2026-07-19｜周日｜5h

主题：Attention CPU。

任务：

1. 实现 causal self-attention CPU。
2. 只支持 batch=1。
3. 支持 GQA：num_heads 和 num_kv_heads 不一致。
4. 与 Python reference 对齐。

验收：

- debug model 单层 attention 输出对齐。

## Day 12｜2026-07-20｜周一｜3h

主题：MLP / SwiGLU CPU。

任务：

1. 实现 gate_proj、up_proj、down_proj。
2. 实现 SiLU。
3. 与 Python reference 对齐。

验收：

- MLP 输出对齐。

## Day 13｜2026-07-21｜周二｜3h

主题：Decoder Layer CPU。

任务：

1. 串联 RMSNorm、Attention、Residual、MLP。
2. 完成一个 decoder layer forward。
3. 与 Python reference 对齐。

验收：

- 单层 logits 或 hidden state 对齐。

## Day 14｜2026-07-22｜周三｜3h

主题：完整 debug model forward。

任务：

1. 跑通多层 debug model。
2. 实现 greedy argmax。
3. 生成 1 个 token。

验收：

- C++ debug model 能完成一次 generate step；
- 与 Python top-1 token 一致或能解释误差。

---

# 第 3 周：接入真实开源模型权重，完成 CPU/GPU 准备

周目标：把目标模型从 debug model 推进到真实 Qwen2.5-0.5B 或备选 SmolLM2-360M。重点是权重转换、shape 验证、内存布局和 GPU 准备。

## Day 15｜2026-07-23｜周四｜3h

主题：完整模型权重转换脚本。

任务：

1. 写 `tools/export_hf_to_blockserve.py`。
2. 从 HF safetensors 读取权重。
3. 转成统一 tensor 命名。
4. fp32/bf16 统一转 fp16 存储。

验收：

- 能生成完整 `model.json` 和 `weights.bin`；
- 转换过程打印每个 tensor 的 shape 和 dtype。

## Day 16｜2026-07-24｜周五｜3h

主题：C++ 加载真实模型元数据。

任务：

1. C++ 加载完整 `model.json`。
2. 不加载全部权重，只验证 metadata。
3. 检查 tensor 数量、shape、offset。

验收：

- C++ 能列出完整模型权重清单。

## Day 17｜2026-07-25｜周六｜5h

主题：真实权重分块加载。

任务：

1. 实现按 tensor 名称加载指定权重。
2. 加载 embedding、lm_head、第一层权重。
3. 验证前几个数值和 Python 一致。

验收：

- C++ 能正确读取真实模型部分权重。

## Day 18｜2026-07-26｜周日｜5h

主题：CPU 侧真实模型 one-layer smoke test。

任务：

1. 用真实模型第一层跑一个极短输入。
2. 只做 correctness smoke，不追求速度。
3. 与 PyTorch 输出对齐到可解释误差。

验收：

- 真实模型第一层 CPU forward 可运行。

## Day 19｜2026-07-27｜周一｜3h

主题：CUDA 工程接入。

任务：

1. 在 RTX 3050 机器上配置 CUDA build。
2. CMake 支持 CUDA。
3. 实现 `DeviceBuffer` 和 CUDA error check。

验收：

- 一个 CUDA 空 kernel 或 vector add 可运行。

## Day 20｜2026-07-28｜周二｜3h

主题：cuBLAS GEMM wrapper。

任务：

1. 封装 cuBLAS / cuBLASLt matmul。
2. 支持 fp16 input、fp16/float accumulate 的基本路径。
3. 对齐 CPU matmul 小矩阵结果。

验收：

- GPU matmul 小测试通过。

## Day 21｜2026-07-29｜周三｜3h

主题：第 3 周收敛。

任务：

1. 整理 CUDA build 文档。
2. 确认 debug model CPU 路径未损坏。
3. 确认真实模型权重能加载。

验收：

- debug model CPU generate 可运行；
- 真实模型权重 metadata 和部分 tensor 可读取；
- CUDA matmul 可运行。

---

# 第 4 周：CUDA 算子与 debug model GPU 推理

周目标：把 debug model 从 CPU 推到 GPU，建立完整 CUDA 推理路径。不要急着上完整 0.5B。

## Day 22｜2026-07-30｜周四｜3h

主题：GPU Tensor 与权重上传。

任务：

1. 实现 `CudaTensor`。
2. 支持 host 到 device 拷贝。
3. 支持 device 到 host 拷贝。
4. 上传 debug model 权重。

验收：

- GPU 上能保存并读回 tensor。

## Day 23｜2026-07-31｜周五｜3h

主题：Embedding CUDA。

任务：

1. 实现 embedding lookup CUDA kernel。
2. 与 CPU/Python 输出对齐。

验收：

- token ids → hidden states 在 GPU 正确。

## Day 24｜2026-08-01｜周六｜5h

主题：RMSNorm CUDA。

任务：

1. 实现 naive RMSNorm CUDA kernel。
2. 不做复杂优化。
3. 与 CPU 输出对齐。

验收：

- debug hidden size 下正确。

## Day 25｜2026-08-02｜周日｜5h

主题：RoPE CUDA。

任务：

1. 实现 RoPE CUDA kernel。
2. 支持 batch=1、seq_len 可变。
3. 与 CPU 输出对齐。

验收：

- q/k 经过 RoPE 后和 CPU 结果一致。

## Day 26｜2026-08-03｜周一｜3h

主题：Attention CUDA v0。

任务：

1. 实现朴素 causal attention CUDA。
2. 只支持 debug model 小尺寸。
3. 不做 FlashAttention。

验收：

- debug attention 输出和 CPU 接近。

## Day 27｜2026-08-04｜周二｜3h

主题：MLP CUDA。

任务：

1. 使用 cuBLAS 完成 gate/up/down projection。
2. 自写 SiLU/elementwise multiply kernel。
3. 与 CPU 对齐。

验收：

- debug MLP 输出正确。

## Day 28｜2026-08-05｜周三｜3h

主题：debug model GPU generate。

任务：

1. 串联 GPU embedding、layer、lm_head。
2. 实现 GPU 或 CPU argmax。
3. 生成 1~4 个 token。

验收：

- debug model 可以通过 CUDA backend generate；
- top-1 结果和 CPU/Python 基本一致。

---

# 第 5 周：真实 0.5B 模型 GPU 单步推理

周目标：把 CUDA backend 从 debug model 扩展到真实 0.5B 模型。先完成单步 logits，再完成短生成。

## Day 29｜2026-08-06｜周四｜3h

主题：真实模型 GPU 内存预算。

任务：

1. 统计权重总大小。
2. 统计临时 buffer 需求。
3. 设定最大 context：128 / 256 起步。
4. 记录 RTX 3050 可用显存。

验收：

- 有明确内存预算，不盲目加载。

## Day 30｜2026-08-07｜周五｜3h

主题：真实权重上传 GPU。

任务：

1. 分批上传 embedding、lm_head、若干层权重。
2. 确认显存占用。
3. 若 OOM，设计 layer streaming 备用方案。

验收：

- 至少能上传完整 embedding、lm_head 和一层权重。

## Day 31｜2026-08-08｜周六｜5h

主题：真实模型单层 GPU forward。

任务：

1. 用真实模型第一层在 GPU 跑短输入。
2. 与 CPU 或 PyTorch 对齐。
3. 修正 shape、stride、transpose 问题。

验收：

- 真实模型第一层 GPU forward 可运行。

## Day 32｜2026-08-09｜周日｜5h

主题：真实模型多层串联。

任务：

1. 串联前 N 层，N 从 2、4、8 逐步增加。
2. 处理残差和中间 buffer 复用。
3. 记录显存。

验收：

- 多层 forward 可运行，不 OOM。

## Day 33｜2026-08-10｜周一｜3h

主题：完整模型 prefill v0。

任务：

1. 串联全部层。
2. 输入短 prompt，例如 8~32 tokens。
3. 得到最后 logits。

验收：

- 完整 0.5B 模型 GPU prefill 能跑出 logits。

## Day 34｜2026-08-11｜周二｜3h

主题：greedy 单 token。

任务：

1. 对 logits 做 argmax。
2. 生成下一个 token。
3. 与 Python 参考输出做粗对齐。

验收：

- 真实模型能生成 1 个 token。

## Day 35｜2026-08-12｜周三｜3h

主题：第 5 周修复。

任务：

1. 清理所有 shape bug。
2. 增加 `--max-context`、`--device cuda` 参数。
3. 补最小运行说明。

验收：

- `blockserve_cli` 可以对真实模型生成 1 token。

---

# 第 6 周：KV Cache 与真实短文本生成

周目标：让真实模型从“单步 logits”变成“可连续生成文本”。

## Day 36｜2026-08-13｜周四｜3h

主题：KV Cache 数据结构。

任务：

1. 设计连续 KV Cache layout。
2. 按 layer 分配 K/V buffer。
3. 支持 batch=1、max_context=256 起步。

验收：

- KV Cache 能成功分配并释放。

## Day 37｜2026-08-14｜周五｜3h

主题：prefill 写入 KV Cache。

任务：

1. prefill 阶段把所有 prompt token 的 K/V 写入 cache。
2. 验证 cache position 正确。

验收：

- prompt K/V 能写入每层 cache。

## Day 38｜2026-08-15｜周六｜5h

主题：decode 使用 KV Cache。

任务：

1. 每次 decode 只输入最后一个 token。
2. Attention 读取历史 KV。
3. 新 token K/V 追加到 cache。

验收：

- decode step 能持续推进 position。

## Day 39｜2026-08-16｜周日｜5h

主题：连续生成 v0。

任务：

1. 实现 generate loop。
2. 支持 `max_new_tokens=8/16/32`。
3. 输出 token ids。

验收：

- 真实模型能连续生成 token ids。

## Day 40｜2026-08-17｜周一｜3h

主题：Python tokenizer wrapper。

任务：

1. 写 `tools/run_prompt.py`。
2. Python encode prompt。
3. 调用 C++ CLI。
4. Python decode 输出 token ids。

验收：

- 输入中文或英文 prompt，能输出文本。

## Day 41｜2026-08-18｜周二｜3h

主题：4GB 显存适配。

任务：

1. 限制 context length。
2. 复用 activation buffer。
3. 确认显存峰值小于 4GB。
4. 如 OOM，切换到更小备选模型。

验收：

- RTX 3050 4GB 上可稳定生成 16 tokens。

## Day 42｜2026-08-19｜周三｜3h

主题：第 6 周收敛。

任务：

1. 整理运行命令。
2. 记录显存、tokens/s、最大 context。
3. 不画复杂图。

验收：

- 本地真实模型短文本生成可复现。

---

# 第 7 周：稳定性、CLI、最小服务能力

周目标：把“能跑一次”变成“别人能按 README 跑”。

## Day 43｜2026-08-20｜周四｜3h

主题：CLI 稳定化。

任务：

1. 支持参数：model path、device、max_new_tokens、max_context。
2. 错误信息清楚。
3. 输入 ids 非法时不崩溃。

验收：

- CLI 使用体验可接受。

## Day 44｜2026-08-21｜周五｜3h

主题：模型转换脚本稳定化。

任务：

1. `export_hf_to_blockserve.py` 支持重复运行。
2. 输出模型文件校验信息。
3. 失败时给出明确错误。

验收：

- 从 HF checkpoint 到 BlockServe model 可重复转换。

## Day 45｜2026-08-22｜周六｜5h

主题：端到端脚本。

任务：

1. 写 `scripts/prepare_model.sh`。
2. 写 `scripts/run_qwen_0_5b.sh`。
3. 一条命令完成运行。

验收：

- 新机器按文档执行可以跑通。

## Day 46｜2026-08-23｜周日｜5h

主题：内存和速度修复。

任务：

1. 减少 CPU↔GPU 不必要拷贝。
2. 只把最终 token/logits 必要部分拷回 CPU。
3. 增加 cudaEvent 简单计时。

验收：

- 生成过程不出现明显多余拷贝。

## Day 47｜2026-08-24｜周一｜3h

主题：采样增强可选。

任务：

1. 保留 greedy 为默认。
2. 可选实现 temperature。
3. top-k 如时间不足不做。

验收：

- greedy 稳定；temperature 可选。

## Day 48｜2026-08-25｜周二｜3h

主题：异常场景。

任务：

1. max_context 超限时拒绝。
2. 权重文件缺失时报错。
3. CUDA OOM 时提示降低 context 或换模型。

验收：

- 常见错误不会直接崩溃。

## Day 49｜2026-08-26｜周三｜3h

主题：第 7 周收敛。

任务：

1. 写 README 运行路径。
2. 写 `docs/known_limitations.md`。
3. 删除无关模拟器扩展任务。

验收：

- README 足够让别人运行真实模型。

---

# 第 8 周：最终打磨与验收

周目标：只做必要打磨，不新增大功能。最终交付一个能在 RTX 3050 4GB 上运行 1B 以下开源模型的 BlockServe v0.1。

## Day 50｜2026-08-27｜周四｜3h

主题：最终模型选择确认。

任务：

1. 确认主模型是否为 Qwen2.5-0.5B。
2. 如果主模型仍不稳定，切换 SmolLM2-360M。
3. 不再尝试新架构。

验收：

- 确定最终演示模型。

## Day 51｜2026-08-28｜周五｜3h

主题：最终端到端演示。

任务：

1. 从 prompt 到输出文本完整跑一遍。
2. 记录命令和输出样例。
3. 保存 `examples/sample_output.md`。

验收：

- 有可展示生成结果。

## Day 52｜2026-08-29｜周六｜5h

主题：从零复现。

任务：

1. 删除 build 目录重新编译。
2. 重新导出模型。
3. 重新运行 prompt。
4. 修复路径和脚本问题。

验收：

- 从零流程可跑通。

## Day 53｜2026-08-30｜周日｜5h

主题：最小正确性检查。

任务：

1. debug model CPU/GPU 仍能对齐。
2. 真实模型能生成非随机乱码。
3. 记录 logits top-1 或 token 输出对照。

验收：

- 不是只靠“能打印文本”蒙混通过。

## Day 54｜2026-08-31｜周一｜3h

主题：最终 README。

任务：

1. 写清楚项目目标。
2. 写清楚构建方式。
3. 写清楚模型转换方式。
4. 写清楚运行方式。
5. 写清楚已知限制。

验收：

- README 完整但不冗长。

## Day 55｜2026-09-01｜周二｜3h

主题：简历表述。

任务：

1. 写 `docs/resume_bullets.md`。
2. 只写真实完成内容，不写模拟器和未完成计划。
3. 准备 5 个面试问题。

验收：

- 可以准确描述 BlockServe 是真实推理框架。

## Day 56｜2026-09-02｜周三｜3h

主题：最终验收。

任务：

1. 最终运行一次本地模型。
2. 记录显存、tokens/s、context、模型名称。
3. 打 tag：`v0.1-real-inference`。
4. 写下一阶段计划：10M/100M 训练、ClusterPilot、Paged KV Cache、多请求。

验收：

- BlockServe v0.1 完成。

---

## 6. 当前阶段最低可接受成果

如果 8 周中途遇到困难，最低成果不是模拟器，而是：

```text
1. C++ BlockServe 能加载真实开源模型权重；
2. CUDA backend 能执行真实模型至少部分层；
3. 最终至少跑通一个 1B 以下模型的完整 generate；
4. 如果 Qwen2.5-0.5B 未完成，必须跑通 SmolLM2-360M；
5. Python 只负责 tokenizer 和转换，不负责推理主路径。
```

---

## 7. 失败处理策略

### 如果 Qwen2.5-0.5B 太复杂

立刻切到 SmolLM2-360M，不继续死磕。

### 如果 4GB 显存 OOM

按顺序处理：

```text
1. 降低 max_context 到 128 或 256；
2. 复用 activation buffer；
3. 不保存不必要 logits；
4. 切换更小模型；
5. 暂时使用 layer streaming；
6. 不把大模型训练任务混进当前阶段。
```

### 如果 CUDA 算子卡住

优先保证：

```text
embedding
RMSNorm
RoPE
matmul/cuBLAS
attention v0
greedy decode
```

不做：

```text
FlashAttention
PagedAttention
top-k/top-p
multi batch
HTTP server
```

### 如果时间不足

砍掉：

```text
temperature sampling
性能优化
多模型支持
复杂文档
图表
```

保留：

```text
真实模型加载
真实 CUDA 推理
KV Cache generate
本地 4GB 运行
```

---

## 8. 下一阶段任务，不在当前 8 周执行

当前 8 周结束后再做：

```text
1. TinyGPT 10M 训练；
2. 100M 训练可行性与实际训练；
3. ClusterPilot；
4. Paged KV Cache；
5. Continuous batching；
6. HTTP/SSE 服务；
7. HIP/ROCm backend；
8. 多模型兼容；
9. llama.cpp/GGUF loader；
10. vLLM 源码阅读和开源 PR。
```

---

## 9. 参考资料

- llama.cpp：用于外部对照和理解 GGUF / 本地推理，不作为当前 C++ Runtime 的照抄对象。
- Hugging Face Qwen2.5-0.5B-Instruct / GGUF：主目标模型候选。
- Hugging Face SmolLM2-360M-Instruct：备选模型。
- NVIDIA CUDA C++ Programming Guide：CUDA backend 基础。
- cuBLAS / cuBLASLt 文档：线性层和矩阵乘法。
- PyTorch / Transformers：Python reference、tokenizer、权重转换。
- vLLM 文档：理解 KV Cache 和推理服务问题，不在本阶段复刻。
