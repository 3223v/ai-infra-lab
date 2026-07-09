# ClusterPilot 项目说明书 v2

## 0. 执行假设

**项目名称**：ClusterPilot  
**项目类型**：面向 AI 工作负载的 GPU 集群调度与资源控制实验项目  
**主语言**：C++20 或 Python  
**当前推荐**：C++20  
**辅助语言**：Python，用于 workload 生成、结果分析、画图  
**目标岗位**：AI Infra、GPU 资源调度、分布式系统、云原生平台、训练平台、MLOps 平台

可用硬件：

- MacBook：日常开发与模拟；
- RTX 3050 4GB：可作为单节点 GPU 示例，但 ClusterPilot 不依赖真实 GPU；
- AMD 200GB 显存资源，约 300 小时：可用于生成更真实的训练/推理任务参数，但 ClusterPilot 核心仍是调度模拟器。

本文档根据当前能力重新收敛：ClusterPilot 不改名、不降级为临时项目，但第一阶段明确是“模拟优先、策略优先、C++/Python 优先”。不做 Go 控制面，不做 Kubernetes Operator，不做生产级平台。

---

## 1. 项目定位

ClusterPilot 解决的不是“单个模型怎么推理”，而是：

> 一个 AI 团队或实验室有多台 GPU 机器，很多训练、推理、实验任务同时到来时，系统如何决定谁先运行、放在哪些 GPU 上、资源不足时如何等待或拒绝，以及不同策略如何影响 GPU 利用率和排队时间。

它与 BlockServe 的关系：

- BlockServe：单卡或单模型服务内部，研究请求、KV Cache、batching；
- ClusterPilot：多任务、多节点、多 GPU 条件下，研究任务级资源分配；
- BlockServe 生成的推理 workload 数据，可作为 ClusterPilot 中推理类 job 的参数来源；
- ClusterPilot 不管理 BlockServe 内部 KV Cache，也不直接执行模型 kernel。

一句话：

> BlockServe 研究“一张卡内部怎么用”；ClusterPilot 研究“一堆卡怎么分”。

---

## 2. 项目要回答的问题

ClusterPilot 需要回答：

1. 一个需要 8 张 GPU 的训练任务，应该等待完整 8 张 GPU，还是部分启动？
2. 总空闲 GPU 足够，但分散在多个节点上时，任务是否能启动？
3. 小任务是否会被大任务长期阻塞？
4. 大任务是否会因为碎片化长期无法启动？
5. Bin Packing 是否真的能降低碎片？
6. Gang Scheduling 是否能避免部分分配造成的资源浪费？
7. 如果任务有优先级，是否会改善交互任务响应，同时牺牲低优先级任务？
8. 如何用实验数据说明一个策略的收益和代价？

---

## 3. 第一阶段范围

第一阶段目标不是做一个真正的 Kubernetes 调度器，而是做一个可运行、可复现、可解释的 GPU 集群调度实验系统。

### 3.1 必须完成

- 虚拟 GPU 集群建模；
- Node/GPU/Job 数据结构；
- Job workload 输入；
- Job arrival-time simulation；
- GPU 分配与释放；
- FIFO 调度；
- First-fit 调度；
- Bin Packing 调度；
- Gang Scheduling；
- 基础 Priority；
- GPU utilization；
- average queue time；
- P95 queue time；
- completed jobs；
- failed/rejected jobs；
- fragmentation score；
- large job wait time；
- CSV 输出；
- Python 图表；
- 实验报告。

### 3.2 第一阶段不做

- Kubernetes Operator；
- Go 控制面；
- PostgreSQL；
- Redis；
- NATS/Kafka；
- Prometheus/Grafana；
- Web UI；
- 真实容器启动；
- 真实训练任务执行；
- 多租户计费；
- 生产级鉴权；
- 复杂抢占恢复；
- MIG/vGPU；
- 多地域调度。

这些都可以作为长期扩展，但不能进入当前主线。

---

## 4. 工作负载类型

### 4.1 分布式训练任务

特征：

- 需要多张 GPU；
- 需要同时启动；
- 运行时间长；
- 对节点位置敏感；
- 可 checkpoint，但第一阶段不模拟真实 checkpoint。

调度关注点：

- Gang Scheduling；
- 大任务等待时间；
- GPU 碎片；
- 拓扑偏好。

### 4.2 在线推理任务

特征：

- GPU 数量较少；
- 对启动延迟敏感；
- 可能长期运行；
- 不适合频繁抢占。

调度关注点：

- priority；
- queue time；
- 快速放置；
- 避免被大训练任务挡住。

### 4.3 离线批处理任务

特征：

- 可延迟；
- 运行时间中等或较长；
- 可以填补碎片资源；
- 可作为后期抢占候选。

调度关注点：

- 资源利用率；
- bin packing；
- 低优先级等待时间。

### 4.4 交互实验任务

特征：

- 通常 1 张 GPU；
- 到达频繁；
- 用户等待敏感；
- 运行时间不稳定。

调度关注点：

- 小任务不要长期饥饿；
- fairness；
- priority；
- 队列延迟。

---

## 5. 数据模型

### 5.1 Cluster 配置

第一版使用 JSON：

```json
{
  "nodes": [
    {"id": "node-0", "gpus": 4},
    {"id": "node-1", "gpus": 4},
    {"id": "node-2", "gpus": 8},
    {"id": "node-3", "gpus": 8}
  ]
}
```

可选扩展字段：

```json
{
  "id": "node-2",
  "gpus": 8,
  "memory_gb_per_gpu": 80,
  "rack": "rack-a",
  "nvlink_domain": "node-2"
}
```

第一阶段只需要 `id` 和 `gpus`。

### 5.2 Job workload

使用 JSONL：

```json
{"arrival_time":0,"job_id":1,"gpu_count":8,"duration":120,"priority":1,"job_type":"training","user":"alice"}
{"arrival_time":5,"job_id":2,"gpu_count":1,"duration":30,"priority":3,"job_type":"interactive","user":"bob"}
{"arrival_time":10,"job_id":3,"gpu_count":4,"duration":60,"priority":1,"job_type":"batch","user":"alice"}
```

必需字段：

- arrival_time；
- job_id；
- gpu_count；
- duration。

可选字段：

- priority；
- job_type；
- user；
- checkpointable；
- max_wait_time；
- preferred_topology。

### 5.3 Job 状态

第一阶段状态：

```text
WAITING
RUNNING
FINISHED
REJECTED
FAILED
```

可选扩展状态：

```text
PREEMPTED
RETRYING
CANCELLED
```

第一阶段不要求复杂抢占，最多只实现 priority 对调度顺序的影响。

---

## 6. 调度策略优先级

### P0：FIFO

最简单基线。

规则：

- 按 arrival_time 进入队列；
- 谁先来谁先调度；
- 如果队首任务资源不足，可以选择 strict FIFO 阻塞，或 non-blocking FIFO 尝试后续任务；
- 第一阶段建议同时实现 strict FIFO 和 non-blocking FIFO，用于展示 head-of-line blocking。

价值：

- 作为所有策略的基线；
- 能展示大任务阻塞小任务的问题。

### P1：First-fit

规则：

- 找到第一个能放下任务的节点或节点组合；
- 能放就启动；
- 不能放就等待或拒绝。

价值：

- 快速实现；
- 作为比 FIFO 更接近资源调度的基线。

### P2：Bin Packing

规则：

- 尽量把任务放入最合适的节点；
- 优先填满已经使用的节点；
- 尽量保留完整大节点给未来大任务；
- 避免把空闲 GPU 打散成无法满足大任务的碎片。

价值：

- 展示碎片化问题；
- 对比 GPU utilization 与 large job wait time。

### P3：Gang Scheduling

规则：

- 一个 job 要么一次性拿到所需全部 GPU；
- 要么不启动；
- 不允许部分启动后占着资源等待剩余 GPU。

价值：

- 模拟分布式训练约束；
- 避免部分资源长期占用；
- 是 ClusterPilot 从普通队列变成 AI 调度项目的关键。

### P4：Priority

规则：

- 高优先级任务更早被调度；
- 第一阶段不要求抢占正在运行的任务；
- 只影响等待队列排序。

价值：

- 展示交互/推理任务与离线任务的差异；
- 为后期抢占做准备。

### P5：Fair Share（可选）

规则：

- 按用户或队列统计资源使用；
- 长期占用过多资源的用户优先级降低；
- 资源不足时照顾使用较少的用户。

价值：

- 展示多租户公平性；
- 但实现复杂度较高，第一阶段可后置。

---

## 7. 核心指标

### 7.1 效率指标

- GPU utilization；
- idle GPU time；
- completed jobs；
- throughput；
- large job completion rate。

### 7.2 延迟指标

- average queue time；
- P50 queue time；
- P95 queue time；
- P99 queue time；
- high priority job start latency；
- large job wait time。

### 7.3 碎片指标

建议定义一个简单 fragmentation score：

```text
fragmentation_score = 1 - (largest_free_block / total_free_gpus)
```

解释：

- 总空闲 GPU 很多，但最大连续可用块很小，碎片高；
- 总空闲 GPU 集中在一个节点或可满足大任务，碎片低。

这只是第一阶段近似指标，不代表工业系统完整碎片模型。

### 7.4 公平性指标

可选：

- per-user allocated GPU time；
- per-user completed jobs；
- starvation count；
- max queue time by user。

第一阶段可只记录，不强制优化。

---

## 8. 输出格式

### 8.1 Job 结果 CSV

```csv
strategy,job_id,user,job_type,arrival_time,start_time,finish_time,queue_time,gpu_count,allocated_nodes,status
fifo,1,alice,training,0,0,120,0,8,node-2,FINISHED
fifo,2,bob,interactive,5,5,35,0,1,node-0,FINISHED
```

### 8.2 Summary CSV

```csv
strategy,workload,completed_jobs,rejected_jobs,avg_queue_time,p95_queue_time,gpu_utilization,fragmentation_score,large_job_wait_time
fifo,mixed,100,3,12.4,45.0,0.71,0.32,80.0
binpack,mixed,105,1,10.2,38.0,0.78,0.21,52.0
```

### 8.3 Event Log

```text
[time=0] job_arrived job_id=1 gpu_count=8
[time=0] job_started job_id=1 nodes=node-2
[time=5] job_arrived job_id=2 gpu_count=1
[time=5] job_started job_id=2 nodes=node-0
[time=120] job_finished job_id=1 released_gpus=8
```

---

## 9. 实验设计

### 9.1 Head-of-line Blocking 实验

目标：展示 strict FIFO 下，大任务阻塞后面小任务。

Workload：

- 队首 16 GPU 大任务；
- 后续多个 1 GPU 短任务；
- 当前集群短时间内只有 8 GPU 空闲。

对比：

- strict FIFO；
- non-blocking FIFO；
- priority。

指标：

- 小任务平均等待时间；
- GPU utilization；
- 大任务启动时间。

### 9.2 Fragmentation 实验

目标：展示随机放置和 bin packing 对碎片的影响。

Workload：

- 混合 1 GPU、2 GPU、4 GPU、8 GPU 任务；
- 多节点异构 GPU 数量。

对比：

- first-fit；
- bin packing。

指标：

- fragmentation score；
- large job wait time；
- completed jobs。

### 9.3 Gang Scheduling 实验

目标：展示部分启动会造成资源浪费。

Workload：

- 多个需要 8/16 GPU 的训练任务；
- 同时加入小任务。

对比：

- allow partial allocation；
- gang scheduling。

指标：

- wasted GPU time；
- completed jobs；
- large job completion rate。

### 9.4 Priority 实验

目标：展示交互任务优先级的收益和代价。

Workload：

- 长训练任务；
- 后续突发高优先级交互任务；
- 低优先级批处理任务。

对比：

- FIFO；
- priority。

指标：

- high priority queue time；
- low priority queue time；
- overall utilization。

### 9.5 BlockServe-derived Workload 实验

目标：把 BlockServe benchmark 数据转化为 ClusterPilot 的推理 job。

方式：

- BlockServe 输出模型大小、并发需求、显存预算、请求负载；
- ClusterPilot 将其抽象为推理服务 job；
- 与训练/批处理 job 混合调度。

第一阶段只做数据映射，不做真实联动。

---

## 10. 推荐技术栈

### 10.1 当前阶段

推荐两种路线，优先 C++。

#### C++ 路线

适合目标：

- 强化 C++ 工程能力；
- 与 BlockServe 统一开发风格；
- 简历叙事更偏系统软件。

使用：

- C++20；
- CMake；
- Catch2 或 GoogleTest；
- nlohmann/json 或自写轻量 parser；
- Python 脚本画图；
- clang-format。

#### Python 路线

适合目标：

- 快速实验；
- 快速画图；
- 快速调整策略。

使用：

- Python 3.10+；
- dataclasses；
- pandas；
- matplotlib；
- pytest。

### 10.2 当前建议

以 C++ 实现核心模拟器，以 Python 负责 workload 生成和图表。

不建议第一阶段使用 Go。Go 放在长期版本中，用于真正控制面服务化和 Kubernetes 生态适配。

### 10.3 长期技术栈

后期可引入：

- Go；
- PostgreSQL；
- Prometheus；
- Grafana；
- Kubernetes Scheduler Framework；
- Volcano 对照；
- Docker Compose；
- OpenTelemetry。

这些不进入第一阶段主线。

---

## 11. 项目结构建议

```text
cluster-pilot/
├── CMakeLists.txt
├── include/clusterpilot/
│   ├── cluster.hpp
│   ├── job.hpp
│   ├── scheduler.hpp
│   ├── metrics.hpp
│   └── workload_loader.hpp
├── src/
│   ├── main.cpp
│   ├── cluster.cpp
│   ├── job.cpp
│   ├── scheduler_fifo.cpp
│   ├── scheduler_first_fit.cpp
│   ├── scheduler_binpack.cpp
│   ├── scheduler_gang.cpp
│   ├── metrics.cpp
│   └── workload_loader.cpp
├── configs/
│   └── cluster.json
├── workloads/
│   ├── mixed.jsonl
│   ├── fragmentation.jsonl
│   └── gang.jsonl
├── results/
├── scripts/
│   ├── generate_workload.py
│   └── plot_results.py
├── tests/
└── README.md
```

这是目录建议，不是强制设计。

---

## 12. 测试要求

必须测试：

- GPU 不会重复分配；
- job 完成后 GPU 会释放；
- 资源不足时 job 不会启动；
- Gang Scheduling 不允许部分启动；
- Bin Packing 不会超过节点容量；
- FIFO 顺序正确；
- priority 排序正确；
- workload 解析错误能被报告；
- 所有 job 完成后资源回到初始状态。

随机测试：

- 随机生成 job；
- 随机 duration；
- 随机 gpu_count；
- 验证任何时刻 allocated_gpus <= total_gpus；
- 验证同一 GPU 不会分给两个 job。

---

## 13. 实施顺序

### 阶段 A：最小模拟器

完成：

- cluster.json；
- jobs.jsonl；
- Node/GPU/Job；
- event loop；
- FIFO；
- CSV 输出。

### 阶段 B：资源放置策略

完成：

- first-fit；
- bin packing；
- fragmentation score；
- mixed workload；
- 对比报告。

### 阶段 C：AI 训练约束

完成：

- gang scheduling；
- partial allocation baseline；
- large training workload；
- wasted GPU time；
- 报告。

### 阶段 D：优先级与交互任务

完成：

- priority queue；
- interactive workload；
- high priority queue time；
- 代价分析。

### 阶段 E：整理与长期扩展

完成：

- README；
- 实验图；
- clusterpilot_report.md；
- Kubernetes/Volcano 概念对照；
- 后续 Go 控制面计划。

---

## 14. 风险控制

### ClusterPilot 抢占 BlockServe 时间

控制：

- ClusterPilot 每周投入不超过计划时间；
- BlockServe 未完成前，不做 Go、不做 API、不做 Kubernetes；
- 只做模拟器和实验。

### 调度策略很多但没有结论

控制：

- 每个策略必须有对照实验；
- 每个策略必须写收益和代价；
- 不写复杂策略，只写能解释的策略。

### 没有真实集群导致可信度不足

控制：

- 强调模拟器研究的是策略；
- 使用可重复 trace；
- 写清楚资源模型假设；
- 用 BlockServe 或真实训练记录生成更真实 workload 参数。

### C++ 实现拖慢进度

控制：

- 如果 C++ 卡住，允许 Python 先实现策略原型；
- C++ 只保留 FIFO、Bin Packing、Gang Scheduling；
- 图表与实验由 Python 完成。

---

## 15. 最终交付物

必须包含：

- C++ 或 Python 核心模拟器；
- cluster config；
- job workload；
- FIFO；
- First-fit；
- Bin Packing；
- Gang Scheduling；
- Priority 可选；
- 结果 CSV；
- 图表；
- README；
- clusterpilot_report.md；
- 测试；
- 已知限制；
- 后续 Go/Kubernetes 计划。

最终演示应是：

> 给定同一个 GPU 集群和同一个 workload，ClusterPilot 用不同调度策略运行，输出 GPU 利用率、排队时间、碎片率和任务完成数的差异。

不是：

> 做一个网页显示 GPU 卡片。

---

## 16. 简历表达方向

可写成：

> 设计并实现 ClusterPilot，一个面向 AI 工作负载的 GPU 集群调度模拟系统。使用 C++ 构建虚拟多节点 GPU 集群、任务生命周期、FIFO/First-fit/Bin Packing/Gang Scheduling 等策略，生成可重复 workload trace，并通过 GPU utilization、queue time、fragmentation score 和 completed jobs 对不同策略进行实验对比。

---

## 17. 参考资料

- Kubernetes Scheduler: https://kubernetes.io/docs/concepts/scheduling-eviction/kube-scheduler/
- Kubernetes Scheduling Framework: https://kubernetes.io/docs/concepts/scheduling-eviction/scheduling-framework/
- Kubernetes Device Plugins: https://kubernetes.io/docs/concepts/extend-kubernetes/compute-storage-net/device-plugins/
- Kubernetes GPU Scheduling: https://kubernetes.io/docs/tasks/manage-gpus/scheduling-gpus/
- NVIDIA Kubernetes Device Plugin: https://github.com/NVIDIA/k8s-device-plugin
- NCCL Collective Operations: https://docs.nvidia.com/deeplearning/nccl/user-guide/docs/usage/collectives.html
- Volcano Scheduler Documentation: https://volcano.sh/
- KAI Scheduler: https://github.com/NVIDIA/KAI-Scheduler
