# ==============================================================================
# 编译器警告选项
# ==============================================================================

function(set_project_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE
      /W4           # 最高警告等级
      /permissive-  # 标准一致性
      /w14242       # 标识符转换
      /w14254       # 空控制语句
      /w14263       # 基类方法隐藏
      /w14265       # 悬空 else
      /w14287       # 可能有符号/无符号不匹配
      /we4289       # 非平凡循环变量
      /w14296       # 非标准扩展
      /w14311       # 变量遮蔽
      /w14545       # 表达式无效果
      /w14546       # 函数调用参数未评估
      /w14547       # 运算符优先
      /w14549       # 运算符歧义
      /w14555       # 布尔运算优先级
      /w14619       # #pragma 警告
      /w14640       # 线程不安全的静态初始化
      /w14826       # 从 'X' 到 'Y' 的转换符号扩展
      /w14905       # 窄隐式强制转换
      /w14906       # 字符串字面量到单字符转换
      /w14928       # 非法的复制初始化
    )
  else()
    target_compile_options(${target} PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Wconversion
      -Wsign-conversion
      -Wshadow
      -Wnon-virtual-dtor
      -Wold-style-cast
      -Wcast-align
      -Wunused
      -Woverloaded-virtual
      -Wnull-dereference
      -Wdouble-promotion
      -Wformat=2
      -Wimplicit-fallthrough
      -Wmisleading-indentation
      -Wduplicated-cond
      -Wduplicated-branches
      -Wlogical-op
      -Wuseless-cast
      -Wno-unknown-warning-option
    )
  endif()
endfunction()
