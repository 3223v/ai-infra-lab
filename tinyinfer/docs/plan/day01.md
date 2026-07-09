# Day 1｜2026-07-09｜项目骨架

## 主题

冻结旧模拟器，创建真实 runtime 项目骨架。

## 任务

1. 在 `tinyinfer/` 下创建新项目目录结构
2. 搭建 CMake 构建系统（OBJECT library + CLI + 测试）
3. 创建所有子目录：`include/tinyinfer/`、`src/`、`tools/`、`models/`、`tests/`
4. 用注释标注每个文件的作用和后续要添加的内容

## 产出

- `CMakeLists.txt` — 构建配置
- `README.md` — 项目说明
- `.gitignore`
- 4 个头文件骨架：`tensor.hpp`、`status.hpp`、`model_config.hpp`、`cli.hpp`
- 4 个源文件骨架：`cli_main.cpp`、`tensor.cpp`、`status.cpp`、`model_config.cpp`
- 4 个 Python 工具骨架
- 2 个测试文件骨架
- `docs/current_scope.md` — 8 周目标

## 验收

```bash
# 未实现，纯骨架
```
