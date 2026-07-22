# AI Infra Lab

## 项目目标

完成一个面向 LLM 推理与 GPU 调度的系统实验项目组合，重点训练 C++ 工程能力、LLM Serving 调度能力、KV Cache 管理能力、CUDA 基础能力和 GPU 集群调度能力。

## 子项目

### 1. TinyInfer


目标：

- 

### 2. CUDA Kernel Lab

CUDA 基础 kernel 实验。

目标：

- 实现 vector add
- 实现 RMSNorm
- 实现 RoPE
- 使用 Python / NumPy / PyTorch 做正确性对照
- 记录基础性能数据

### 3. ClusterPilot-Lite

GPU 集群调度模拟器。

目标：

- 模拟多节点 GPU 集群
- 实现 FIFO 调度
- 实现 Bin Packing
- 实现 Gang Scheduling
- 对比 GPU 利用率、排队时间和碎片率
