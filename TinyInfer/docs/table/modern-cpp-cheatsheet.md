# 现代 C++ 特性速查表（C++17 / C++20）

> **TinyInfer 项目快速参考。** 每条末尾的 `📖` 链接跳转到 [深入讲解](./modern-cpp-deep-dive.md) 对应章节。

---

## 目录 · 点击跳转

### 属性与修饰

| [1. `[[nodiscard]]` — 忽略返回值就编译警告](#1-nodiscard) |
| [2. `noexcept` — 标记函数不抛异常](#2-noexcept) |
| [3. `constexpr` / `inline constexpr` — 编译期常量，头文件安全](#3-constexpr--inline-constexpr) |
| [7. `explicit` — 阻止隐式类型转换](#7-explicit) |

### 类型系统

| [4. `enum class` — 有作用域的强类型枚举](#4-enum-class) |
| [8. `static_assert` + type traits — 编译期类型断言](#8-static_assert--type-traits) |
| [9. `std::variant` — 类型安全的 union](#9-stdvariant) |
| [10. `std::optional` — 可能缺失的值](#10-stdoptional) |
| [11. `std::string_view` — 零拷贝字符串引用](#11-stdstring_view) |

### 构造与生命周期

| [5. `= default` / `= delete` — 显式声明/禁用编译器生成函数](#5--default--delete) |
| [6. `std::move` / `std::forward` — 移动语义与完美转发](#6-stdmove--stdforward) |
| [13. CTAD — 类模板实参自动推导](#13-ctad) |
| [15. RAII / ScopeGuard — 析构自动释放资源](#15-raii--scopeguard) |
| [16. const 成员函数与 const 指针 — 只读承诺](#16-const-成员函数与-const-指针) |
| [21. 拷贝/移动五大函数速查 — 何时生成、何时禁用](#21-拷贝移动五大函数速查) |

### 标准库容器与工具

| [12. `#pragma once` — 头文件只被包含一次](#12-pragma-once) |
| [14. `std::chrono` — 类型安全的时间库](#14-stdchrono) |
| [17. C++ 四种 cast — `static_cast` / `dynamic_cast` / `const_cast` / `reinterpret_cast`](#17-c-四种-cast) |
| [18. `namespace` 与匿名 `namespace {}` — 作用域与文件私有化](#18-namespace-与匿名-namespace-) |
| [19. `std::deque` — 双端队列](#19-stddeque) |
| [20. `std::ifstream` — 文件读取](#20-stdifstream) |
| [22. `std::isspace` — 字符空白判断](#22-stdisspace) |

### 模式与惯用法

| [23. Error 模式 — 错误码 + 工厂方法最佳实践](#23-error-模式) |

---

## 1. `[[nodiscard]]`

> 📖 [详解 #1](./modern-cpp-deep-dive.md#1-nodiscard-详解)

### 一句话

**告诉编译器"这个函数的返回值不应该被丢弃"**——调用者不接收返回值时产生编译警告。

### 语法

```cpp
// 函数前面（最常见）
[[nodiscard]] bool ok() const noexcept;

// 函数后面
int compute() [[nodiscard]];

// C++20: 附诊断信息
[[nodiscard("leaking cache handle — must call release()")]]
KVCacheHandle acquire();
```

### 典型场景

```cpp
err.code();        // ⚠️ 编译警告：ignoring return value
(void)err.code();  // 显式抑制
```

### 项目位置

| 文件 | 示例 |
|------|------|
| [error.h](../../TinyInfer/tinyinfer/include/tinyinfer/common/error.h) | `[[nodiscard]] ErrorCode code() const noexcept` |
| [result.h](../../TinyInfer/tinyinfer/include/tinyinfer/common/result.h) | `[[nodiscard]] T& value() noexcept` |

---

## 2. `noexcept`

> 📖 [详解 #2](./modern-cpp-deep-dive.md#2-noexcept-详解)

### 一句话

**承诺函数不抛异常。** 若在 `noexcept` 函数中抛异常 → `std::terminate()`。

### 语法

```cpp
void cleanup() noexcept;                               // 无条件承诺
void swap(T& a, T& b) noexcept(is_nothrow_swappable);  // 有条件

// noexcept 运算符：编译期检查表达式会不会抛
static_assert(noexcept(42 + 1));         // true
static_assert(!noexcept(new int[100]));  // false — new 可能抛

// 传播：析构的 noexcept 取决于成员
~ScopeGuard() noexcept(noexcept(f_()));  // f_() 不抛 → 析构也不抛
```

### 加不加 `noexcept`？

```
✅ 必须：析构函数、swap、移动构造/赋值
✅ 建议：简单 getter（不可能抛）
❌ 不要：可能抛 std::bad_alloc、IO、网络、锁
```

### 移动构造与性能

**如果移动构造不是 `noexcept`，`std::vector` 扩容时回退到拷贝！**

```cpp
class Foo {
  Foo(Foo&&) = default;  // 成员都是 noexcept → 自动是 noexcept
};
```

---

## 3. `constexpr` / `inline constexpr`

> 📖 [详解 #3](./modern-cpp-deep-dive.md#3-constexpr--inline-constexpr-详解)

### 一句话

**`constexpr`：值必须在编译期确定。**`inline constexpr`（C++17）：头文件中安全定义全局常量，不会"重定义"链接报错。

### 关键区分

```cpp
const int x = rand();         // ✅ 运行期确定，此后不可变
constexpr int y = rand();     // ❌ rand() 不是 constexpr

// 头文件全局常量：必须加 inline！
inline constexpr int kMax = 128;       // ✅ C++17 推荐
// constexpr int kMax = 128;           // ❌ 多个 .cpp 包含可能报重定义
```

| 关键字 | 含义 | 头文件全局变量 |
|--------|------|--------------|
| `const` | 运行期不可变 | ❌ 链接冲突 |
| `constexpr` | 编译期常量 | ❌ ODR 问题 |
| `inline constexpr` | 编译期常量 + 跨 TU 合并 | ✅ |

### 项目位置

```cpp
// types.h
inline constexpr TokenId kInvalidTokenId = -1;
inline constexpr size_t kMaxGeneratedTokens = 65536;
```

---

## 4. `enum class`

> 📖 [详解 #4](./modern-cpp-deep-dive.md#4-enum-class-详解)

### 一句话

**有作用域的强类型枚举。** 不能隐式转 `int`，必须写 `ErrorCode::kOk`，不同枚举的值不会命名冲突。

### 对比

```cpp
// ❌ 旧式：命名冲突，隐式转 int
enum OldColor { red, green, blue };
enum OldLight { red, yellow, green };  // 冲突！
int x = red;                            // 隐式转换 — 不安全

// ✅ enum class：作用域隔离
enum class Color { kRed, kGreen, kBlue };
enum class Light { kRed, kYellow, kGreen };  // OK，不冲突
// int y = Color::kRed;               // ❌ 编译错误
int y = static_cast<int>(Color::kRed); // ✅ 显式

// 指定底层类型
enum class ErrorCode : int { kOk = 0, kInvalidArgument = 100 };
```

### 命名规范

`k前缀`（Google C++ Style）：`kOk`、`kInvalidArgument`、`kTimeout`。

---

## 5. `= default` / `= delete`

> 📖 [详解 #5](./modern-cpp-deep-dive.md#5--default--delete-详解)

### 一句话

**`= default`：让编译器生成默认实现。** **`= delete`：禁止调用某函数（编译期报错）。**

### 语法

```cpp
class NonCopyable {
 public:
  NonCopyable() = default;                      // 编译器生成默认构造
  NonCopyable(const NonCopyable&) = delete;     // 禁止拷贝
  NonCopyable& operator=(const NonCopyable&) = delete;
  NonCopyable(NonCopyable&&) = delete;          // 禁止移动
  NonCopyable& operator=(NonCopyable&&) = delete;
};
```

### 编译器自动生成规则（重要）

```
1. 手动定义 拷贝构造/拷贝赋值/析构 任意一个
   → 编译器不再生成 移动构造、移动赋值
2. 手动定义 移动构造/移动赋值
   → 编译器 delete 掉 拷贝构造、拷贝赋值
3. 默认生成的拷贝是浅拷贝（逐成员 bitwise copy）
```

这就是为什么框架中 Tensor、Handle 类大量使用 `NonCopyable`：
浅拷贝直接灾难 → 直接 `= delete` 禁止拷贝。

### 更多 `= delete` 场景

```cpp
void setAge(double) = delete;     // 阻止 setAge(3.14)
void* operator new(size_t) = delete;  // 禁止堆分配
```

---

## 6. `std::move` / `std::forward`

> 📖 [详解 #6](./modern-cpp-deep-dive.md#6-stdmove--stdforward-详解)

### 一句话

- **`std::move`**：无条件转成右值引用，语义是"所有权转移"，O(1) 不拷贝堆数据。
- **`std::forward`**：模板中保持原始值类别（左值保持左值、右值保持右值），语义是"透传"。

### 核心对比

```cpp
// std::move — 掏空源对象
std::string a = "hello";
std::string b = std::move(a);  // a 被掏空，内容转移到 b
// 此后 a 处于"合法但未指定"状态

// std::forward — 模板中完美转发
template <typename F>
explicit ScopeGuard(F&& f) : f_(std::forward<F>(f)) {}
// 左值传入 → 拷贝，右值传入 → 移动
```

### 三条黄金规则

```
1. std::move 不搬运任何内存，它只是 static_cast<T&&>
2. 移动后源对象不要读取（处于"被掏空"状态）
3. 对 const 对象 move → 退化为拷贝（const T&& 不匹配 T&&）
```

### 移动构造 vs 拷贝构造性能

```
拷贝构造：分配新堆内存 + memcpy → O(n)
移动构造：只复制指针变量 + 源置空 → O(1)

// 移动构造实现模板
Buffer(Buffer&& other) noexcept {
  data = other.data;        // 抢夺指针
  len = other.len;
  other.data = nullptr;     // 源对象置空！防止双重释放
  other.len = 0;
}
```

### 项目位置

```cpp
// error.h — 移动语义避拷贝
Error(ErrorCode code, std::string message)
    : code_(code), message_(std::move(message)) {}

// scope_guard.h — 完美转发
ScopeGuard(F&& f) : f_(std::forward<F>(f)) {}
```

---

## 7. `explicit`

> 📖 [详解 #7](./modern-cpp-deep-dive.md#7-explicit-详解)

### 一句话

**禁止编译器用这个构造函数/转换运算符做隐式类型转换。**

### 什么情况必须加

```cpp
// 单参数构造函数 → 必须 explicit
explicit Error(ErrorCode code);

// operator bool → 必须 explicit
explicit operator bool() const noexcept;
// 加 explicit 后只能用于 if/while/?:/! 等布尔上下文
```

### 不加的灾难

```cpp
class String {
  String(int capacity);  // 没加 explicit
};
void takeString(const String& s);
takeString(42);  // ✅ 编译通过！42 → String(42)，不是你想表达的
```

---

## 8. `static_assert` + type traits

> 📖 [详解 #8](./modern-cpp-deep-dive.md#8-static_assert--type-traits-详解)

### 一句话

**编译期断言。** 不满足条件时编译直接失败，零运行时开销。

### 常用 type traits

```cpp
#include <type_traits>

static_assert(!std::is_reference_v<T>,  "Result<T> does not support references");
static_assert(!std::is_same_v<T, void>, "Use Result<void> pattern");
static_assert(std::is_nothrow_move_constructible_v<T>);
```

| Trait | 含义 |
|-------|------|
| `std::is_same_v<A, B>` | A 和 B 相同？ |
| `std::is_reference_v<T>` | T 是引用？ |
| `std::is_base_of_v<Base, Derived>` | Base 是 Derived 基类？ |
| `std::is_nothrow_move_constructible_v<T>` | 安全 noexcept 移动？ |
| `std::is_convertible_v<From, To>` | From 能隐式转 To？ |

### `if constexpr`（C++17）

```cpp
template <typename T>
auto get_value(const T& val) {
  if constexpr (std::is_pointer_v<T>) {
    return *val;   // T 是指针走这分支，否则不实例化
  } else {
    return val;
  }
}
```

---

## 9. `std::variant`

> 📖 [详解 #9](./modern-cpp-deep-dive.md#9-stdvariant-详解)

### 一句话

**类型安全的 union。** 同一块内存，一次只存多种类型中的一种，访问时编译器强制检查类型。

### 核心 API

```cpp
#include <variant>
using Storage = std::variant<int, std::string, Error>;

Storage s = 42;                           // 存 int
std::holds_alternative<int>(s);           // true
int v = std::get<int>(s);                 // 类型匹配 → OK

// ⭐ 最快最安全
if (auto* p = std::get_if<int>(&s)) { use(*p); }

// 遍历
std::visit([](auto&& val) {
  using T = std::decay_t<decltype(val)>;
  if constexpr (std::is_same_v<T, int>) { /* ... */ }
}, s);
```

### 项目位置

```cpp
// result.h
std::variant<T, Error> storage_;  // 有值 T → 成功；有 Error → 失败
```

---

## 10. `std::optional`

> 📖 [详解 #10](./modern-cpp-deep-dive.md#10-stdoptional-详解)

### 一句话

**"可能有值，也可能没有"的安全容器。** 栈上存储，不分配堆内存，不会忘记判空。

### 核心 API

```cpp
#include <optional>

std::optional<int> opt = 42;
std::optional<int> empty = std::nullopt;

// 判断
if (opt) { }              // 隐式转 bool
if (opt.has_value()) { }  // 等价显式写法

// 取值
int v = opt.value();        // 无值时抛 bad_optional_access
int v = *opt;               // 无值 → UB！
int v = opt.value_or(-1);   // ⭐ 最安全：无值时返回默认值

// 修改
opt = 100;                  // 设新值
opt = std::nullopt;         // 清空
opt.reset();                // 清空
opt.emplace(1, 2);          // 原地构造（避免拷贝）
```

### vs 指针

```cpp
// ❌ 指针：语义模糊、可能忘记判空、可能内存泄漏
int* maybe_age = nullptr;

// ✅ optional：语义清晰、栈上存储、强制判空
std::optional<int> maybe_age = std::nullopt;
```

### 项目位置

```cpp
// result.h — Result<void> 用 optional<Error> 表示"没有值但有错误"
std::optional<Error> error_;
```

---

## 11. `std::string_view`

> 📖 [详解 #11](./modern-cpp-deep-dive.md#11-stdstring_view-详解)

### 一句话

**不拥有内存的只读字符串引用。** 传参、截取、比较都不分配新内存。内部结构 = 指针 + 长度。

### 核心用法

```cpp
#include <string_view>

std::string_view sv = "hello";       // 来自字面量
std::string s = "world";
std::string_view sv2 = s;            // 来自 string — 不拷贝！

// 子串 O(1) — 仅改内部指针
auto sub = sv.substr(0, 3);          // "hel"，零分配

// ⚠️ 悬空陷阱
std::string_view bad() {
  std::string tmp = "temp";
  return std::string_view(tmp);      // ❌ tmp 已析构！
}
```

### 规则

```
✅ 函数参数用 string_view（只读，不修改）
✅ 返回类型用 string（你有所有权）
❌ 确保 string_view 引用的原始数据活得足够久
```

---

## 12. `#pragma once`

> 📖 [详解 #12](./modern-cpp-deep-dive.md#12-pragma-once-详解)

### 一句话

**本头文件每个翻译单元只解析一次。** 等效于传统 `#ifndef/#define/#endif`，但一行搞定且无宏名冲突风险。

### 对比

```cpp
// 传统（3行 + 需保证宏名唯一）
#ifndef TINYINFER_COMMON_ERROR_H_
#define TINYINFER_COMMON_ERROR_H_
// ...
#endif

// 现代（1行）
#pragma once
// ...
```

### 项目中所有头文件都使用

```cpp
#pragma once  // ← 每个 .h 文件第一行
```

---

## 13. CTAD

> 📖 [详解 #13](./modern-cpp-deep-dive.md#13-ctad-详解)

### 一句话

**C++17 起，编译器根据构造函数参数自动推导模板类型。** 不需要写 `ScopeGuard<SomeLongType>`。

### 效果

```cpp
// 没有 CTAD（C++14）
auto g = ScopeGuard<std::function<void()>>([]{ cleanup(); });

// CTAD（C++17）
auto g = ScopeGuard([]{ cleanup(); });  // 自动推导 lambda 类型

// 自定义推导指引
template <typename F>
ScopeGuard(F) -> ScopeGuard<F>;   // 告诉编译器怎么推
```

---

## 14. `std::chrono`

> 📖 [详解 #14](./modern-cpp-deep-dive.md#14-stdchrono-详解)

### 一句话

**类型安全的时间库。** 时长自带单位，不会把毫秒当成纳秒。

### 核心

```cpp
#include <chrono>
using Clock = std::chrono::steady_clock;  // 单调时钟（不受系统调时影响）

auto t1 = Clock::now();
auto diff = Clock::now() - t1;
int64_t ns = std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count();
int64_t ms = ns / 1'000'000;
```

### 时钟选择

| 时钟 | 特点 | TinyInfer 用 |
|------|------|-------------|
| `steady_clock` | 单调递增，不受系统时间调整影响 | ✅ 超时、性能测量 |
| `system_clock` | 墙上时间，可被 NTP 调整 | 日志时间戳 |

---

## 15. RAII / ScopeGuard

> 📖 [详解 #15](./modern-cpp-deep-dive.md#15-raii--scopeguard-详解)

### 一句话

**资源获取即初始化（RAII）：构造函数获取资源，析构函数释放资源。** ScopeGuard 是轻量 ad-hoc 版：存 lambda，析构时自动调用。

### RAII 核心思想

```
C 风格（goto cleanup 地狱）：
  res1 = acquire() → if !res1 goto err1
  res2 = acquire() → if !res2 goto err2
  // 18行 + 3个标签

C++ RAII：
  auto res1 = Acquire1();  // 构造即获取
  auto res2 = Acquire2();
  return 0;                // 析构自动释放，逆序！
  // 6行，不可能出错
```

### RAII 管理的不只是内存

| 资源 | 获取 | 释放 |
|------|------|------|
| 内存 | `new` / `make_unique` | `delete`（自动） |
| 文件 | `std::ifstream(path)` | 析构自动 `close()` |
| 锁 | `std::lock_guard(m)` | 析构自动 `unlock()` |
| GPU 显存 | `cudaMalloc` | `cudaFree` |

### ScopeGuard 用法

```cpp
auto guard = ScopeGuard([&] { release_resource(); });
// ... 可能抛异常的代码 ...
guard.dismiss();  // 成功了，不执行清理
// } ← 未 dismiss 则析构时一定执行
```

### RAII 类通常禁止拷贝

```cpp
class FileHandle {
  FILE* file_;
 public:
  explicit FileHandle(const char* path) : file_(fopen(path, "r")) {}
  ~FileHandle() { if (file_) fclose(file_); }
  FileHandle(const FileHandle&) = delete;            // 禁止拷贝
  FileHandle& operator=(const FileHandle&) = delete;
  FileHandle(FileHandle&& other) noexcept {          // 允许移动
    file_ = other.file_;
    other.file_ = nullptr;
  }
};
```

---

## 16. const 成员函数与 const 指针

> 📖 [详解 #16](./modern-cpp-deep-dive.md#16-const-成员函数与-const-指针-详解)

### 一句话

**函数末尾 `const`：承诺不修改成员变量。返回值前 `const`：返回内容不允许被修改。**

### 后置 const（成员函数）

```cpp
const Request* front() const;
//                     ↑↑↑↑↑
//  承诺：调用该函数不会修改对象任何成员变量

// 约束：
// 1. 函数内不能改成员变量
// 2. 不能调用同类非 const 成员函数
// 3. const 对象只能调用 const 成员函数
```

### 返回值前 const（指针）

```cpp
// 区分三种写法：
const Request* p;      // ① 内容不可改，指针可改  ← 最常用
p->data = 100;  // ❌
p = nullptr;     // ✅

Request* const p;     // ② 指针不可改，内容可改
const Request* const p; // ③ 都不可改

// 常配对重载：
Request* front();              // 普通对象 → 可修改元素
const Request* front() const;  // const 对象 → 只读
```

---

## 17. C++ 四种 cast

> 📖 [详解 #17](./modern-cpp-deep-dive.md#17-c-四种-cast-详解)

### 速查表

| cast | 用途 | 安全检查 | 运行时开销 |
|------|------|---------|-----------|
| `static_cast` | 基础类型转换、父子指针、enum↔int | 无 | 无 |
| `dynamic_cast` | 多态父子类向下转型 | 有（RTTI） | 有 |
| `const_cast` | 增删 `const`/`volatile` | 无 | 无 |
| `reinterpret_cast` | 粗暴比特重解释（`int*`↔`float*`） | 无 | 无 |

### static_cast 可以做

```cpp
// 1. 基础类型转换
static_cast<unsigned char>(ch);   // char → unsigned char

// 2. void* 互转
void* p = &x;
int* pi = static_cast<int*>(p);

// 3. 父子指针
Base* b = static_cast<Base*>(&derived);  // 向上 — 安全

// 4. enum ↔ 整数
int v = static_cast<int>(ErrorCode::kOk);
```

### 关键规则

```
static_cast  — 默认首选，编译期完成
dynamic_cast — 多态向下转型才用（失败返回 nullptr）
const_cast   — 只改 const，不改底类型
reinterpret_cast — 底层/驱动代码，极不安全，新项目禁止
```

---

## 18. `namespace` 与匿名 `namespace {}`

> 📖 [详解 #18](./modern-cpp-deep-dive.md#18-namespace-与匿名-namespace--详解)

### 一句话

**具名 namespace：代码组织与隔离。匿名 namespace：文件级私有（替代 C 的 `static` 全局函数）。**

### 用法

```cpp
// 具名 namespace — 外部可通过 tinyinfer::xxx 访问
namespace tinyinfer {
  class Error { /* ... */ };
}

// 匿名 namespace — 仅当前 .cpp 可见
namespace {
  std::string trim(const std::string& s) { /* ... */ }
  // 外部无法访问，等效于 C 的 static 函数
}
```

### 规则

```
✅ 匿名 namespace → 文件作用域私有工具函数（推荐替代 static）
✅ 具名 namespace → 对外 API 组织
❌ using namespace std; → 头文件中永远禁止
```

---

## 19. `std::deque`

> 📖 [详解 #19](./modern-cpp-deep-dive.md#19-stddeque-详解)

### 一句话

**双端队列。** 头尾插入/删除 O(1)，支持随机访问 `[]`，底层分段连续内存。

### 核心 API

```cpp
#include <deque>
std::deque<int> dq{2, 4, 6};

// 两端操作（deque 特色）
dq.push_back(8);        // 尾插
dq.push_front(0);       // 头插
dq.emplace_back(10);    // 尾原地构造（更高效）
dq.pop_back();          // 尾删
dq.pop_front();         // 头删

// 访问
dq[0];                  // 随机访问（无越界检查）
dq.at(1);               // 随机访问（越界抛异常）
dq.front();             // 首元素
dq.back();              // 尾元素

// 容量
dq.empty();             // 是否为空
dq.size();              // 元素个数
dq.clear();             // 清空
```

### vs vector

| | `vector` | `deque` |
|---|---------|---------|
| 内存 | 整块连续 | 分段连续 |
| `push_front` | ❌ O(n) | ✅ O(1) |
| `push_back` | ✅ O(1)* | ✅ O(1) |
| 缓存友好 | 更好 | 略差 |

---

## 20. `std::ifstream`

> 📖 [详解 #20](./modern-cpp-deep-dive.md#20-stdifstream-详解)

### 一句话

**文件输入流，从磁盘文件读取内容。** RAII 自动管理文件句柄，析构自动 close。

### 核心用法

```cpp
#include <fstream>

// 构造 = 打开
std::ifstream file("data.txt");
if (!file.is_open()) {  // 或 if (!file)
  // 打开失败
}

// 逐行读取
std::string line;
while (std::getline(file, line)) {
  // 处理每行
}

// 离开作用域自动 close()，不用手动
```

### 三个文件流对比

| | `std::ifstream` | `std::ofstream` | `std::fstream` |
|---|----------------|-----------------|----------------|
| 模式 | 只读 | 只写 | 可读可写 |

---

## 21. 拷贝/移动五大函数速查

> 📖 [详解 #21](./modern-cpp-deep-dive.md#21-拷贝移动五大函数速查-详解)

### 一张表分清四者

| 函数 | 触发场景 | 目标对象状态 | 开销 | 行为 |
|------|---------|------------|------|------|
| 拷贝构造 `T(const T&)` | `T b = a;` | 新建 | O(n) | 深拷贝/浅拷贝 |
| 拷贝赋值 `operator=(const T&)` | `b = a;` | 已存在 | O(n) | 释放旧 + 复制新 |
| 移动构造 `T(T&&)` | `T b = std::move(a);` | 新建 | O(1) | 偷走资源，源置空 |
| 移动赋值 `operator=(T&&)` | `b = std::move(a);` | 已存在 | O(1) | 释放旧 + 偷新 |

### 拷贝构造 vs 拷贝赋值核心区别

```
拷贝构造：目标对象还不存在（正在创建）
拷贝赋值：目标对象已存在，有旧资源要先清理
```

### 常见误区

```cpp
// 误区 1：std::move 搬运内存 → ❌ 只是类型转换
// 误区 2：移动后源对象不能用 → 可赋值、可析构；不能读
// 误区 3：右值一定触发移动 → RVO 会直接消除临时对象
```

---

## 22. `std::isspace`

> 📖 [详解 #22](./modern-cpp-deep-dive.md#22-stdisspace-详解)

### 一句话

**判断字符是否为空白。** 必须先把 `char` 转成 `unsigned char`，否则负数 char 触发未定义行为！

### 正确用法

```cpp
#include <cctype>
//                                  ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 必须转！
std::isspace(static_cast<unsigned char>(ch));

// ❌ 错误：有符号 char 为负数 → UB
std::isspace(ch);
```

### 识别的空白字符

空格 `' '`、`\t`、`\n`、`\r`、`\v`、`\f`

---

## 23. Error 模式

> 📖 [详解 #23](./modern-cpp-deep-dive.md#23-error-模式-详解)

### 一句话

**错误码枚举 + 不可变 Error 对象 + static 工厂方法 = 安全、可追踪的错误处理。**

### 模式组成

```cpp
// ① 强类型错误码
enum class ErrorCode : int {
  kOk = 0,
  kInvalidArgument = 100,
  kInvariantViolation = 404,
};

// ② 不可变 Error 对象
class Error {
 public:
  Error(ErrorCode code, std::string message)
      : code_(code), message_(std::move(message)) {}  // 移动避拷贝

  [[nodiscard]] ErrorCode code() const noexcept;  // 返回值不能丢 + 不抛异常 + 不修改对象
  [[nodiscard]] const std::string& message() const noexcept;  // 返回引用，不拷贝

  // ③ static 工厂方法：不用每次写 ErrorCode::xxx
  static Error InvalidArgument(std::string msg) {
    return Error(ErrorCode::kInvalidArgument, std::move(msg));
  }
};

// 调用
auto err = Error::InvalidArgument("shape mismatch");
if (!err.ok()) {
  log(err.code(), err.message());
}
```

### 技术点分解

| 写法 | 含义 |
|------|------|
| `enum class ErrorCode` | 强类型枚举 |
| `[[nodiscard]]` | 返回值不能忽略 |
| `const noexcept` | 不修改对象 + 不抛异常 |
| `std::move(message)` | 移动字符串避堆拷贝 |
| `const std::string&` 返回 | 返回引用不拷贝，外部只读 |
| `static` 工厂 | 无需 Error 实例就能调用 |
| 初始化列表 `: code_(code)` | 成员直接初始化，比函数体赋值更高效 |

---

## 附录 A：头文件速查

```cpp
#include <string>          // std::string
#include <string_view>     // std::string_view
#include <optional>        // std::optional
#include <variant>         // std::variant
#include <type_traits>     // std::is_same_v, std::is_reference_v, ...
#include <utility>         // std::move, std::forward
#include <chrono>          // std::chrono
#include <cassert>         // assert
#include <cstdint>         // int64_t, uint32_t
#include <cctype>          // std::isspace
#include <fstream>         // std::ifstream
#include <deque>           // std::deque
```

## 附录 B：项目构建命令

```bash
# 配置（首次）
cmake --preset debug-core -B build/debug-core

# 编译
cmake --build build/debug-core -j$(nproc)

# 运行测试
ctest --test-dir build/debug-core --output-on-failure

# 运行 CLI
./build/debug-core/tinyinfer_cli --version
```

> **继续阅读：** [现代 C++ 特性深入讲解](./modern-cpp-deep-dive.md)

> **CMake 命令手册：** [CMake 快捷指令参考](./cmake-quick-reference.md)
