# Day 7｜2026-07-15｜第 1 周收敛

## 主题

清理、验证、文档。确保整个工程骨架可以交付。

## 任务

1. 清理 CMake 和目录
   - 确认所有 `.cpp` 文件都在 CMakeLists.txt 中列出
   - 确认 `-Wall -Wextra -Wpedantic` 零警告
2. 从零重编验证
   ```bash
   rm -rf build && cmake -B build && cmake --build build
   cd build && ctest --output-on-failure
   ```
3. 确认所有工具脚本可运行
   - `tools/inspect_hf_model.py`
   - `tools/export_debug_model.py`
   - `tools/reference_forward.py`
4. 更新 README
   - 构建方式
   - 导出 debug model
   - 运行 CLI
5. 写 `docs/week1_review.md`
   - 本周完成了什么
   - 学到了什么
   - 遇到的坑

## 验收

- 从空 build 目录可重新编译，零警告
- `ctest` 全绿
- debug model 可导出并被 C++ 读取
- README 足够让别人跑起来
