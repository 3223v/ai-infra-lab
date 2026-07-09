#         项目启动指令
  - 编译：cmake -B build && cmake --build build
  - 运行：./build/blockserve/blockserve [workload路径]
  - 测试：cd build && ctest --output-on-failure
  - 单独跑测试二进制：./build/blockserve/blockserve_tests -s

#         干净 build 目录重编译

  - rm -rf build && cmake -B build && cmake --build build && cd build && ctest