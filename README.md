# AI Infra Lab

## 项目目标

完成一个面向 LLM 推理与 GPU 调度的系统实验项目组合，重点训练 C++ 工程能力、LLM Serving 调度能力、KV Cache 管理能力、CUDA 基础能力和 GPU 集群调度能力。

## 子项目

### 1. BlockServe-Sim

LLM Serving 调度与 KV Cache 模拟器。

目标：

- 实现请求状态机
- 实现 Prefill / Decode 分离
- 实现 Continuous Batching 模拟
- 实现 Paged KV Cache 模拟
- 对比 Sequential、Fixed Batch、Continuous Batching 等策略
- 输出 benchmark 指标和实验报告

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

## 阶段计划

- 第 1 周：完成 C++ 工程基础、CMake、测试框架和仓库骨架
- 第 2 周：完成 BlockServe-Sim 请求模型与状态机
- 第 3 周：完成 Paged KV Cache 模拟
- 第 4 周：完成调度策略与 Continuous Batching
- 第 5 周：完成 benchmark、图表和实验报告
- 第 6 周：完成 CUDA Kernel Lab
- 第 7 周：完成 ClusterPilot-Lite
- 第 8 周：整理文档、实验结果和简历描述

## 当前状态

项目初始化阶段。
