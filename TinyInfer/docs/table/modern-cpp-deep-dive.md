# 现代 C++ 特性深入讲解（C++17 / C++20）

> **每个章节对应速查表中同序号的条目。**
> 阅读前提：已浏览 [现代 C++ 特性速查表](./modern-cpp-cheatsheet.md)。
>
> 本文档面向 **TinyInfer 项目开发者**，重点讲"为什么这样设计"和"内部发生了什么"，而非罗列 API 文档。

---

## 目录

| # | 特性 |
|---|------|
| [1](#1-nodiscard-详解) | `[[nodiscard]]` 详解 |
| [2](#2-noexcept-详解) | `noexcept` 详解 |
| [3](#3-constexpr--inline-constexpr-详解) | `constexpr` / `inline constexpr` 详解 |
| [4](#4-enum-class-详解) | `enum class` 详解 |
| [5](#5--default--delete-详解) | `= default` / `= delete` 详解 |
| [6](#6-stdmove--stdforward-详解) | `std::move` / `std::forward` 详解 |
| [7](#7-explicit-详解) | `explicit` 详解 |
| [8](#8-static_assert--type-traits-详解) | `static_assert` + type traits 详解 |
| [9](#9-stdvariant-详解) | `std::variant` 详解 |
| [10](#10-stdoptional-详解) | `std::optional` 详解 |
| [11](#11-stdstring_view-详解) | `std::string_view` 详解 |
| [12](#12-pragma-once-详解) | `#pragma once` 详解 |
| [13](#13-ctad-详解) | CTAD 详解 |
| [14](#14-stdchrono-详解) | `std::chrono` 详解 |
| [15](#15-raii--scopeguard-详解) | RAII / ScopeGuard 详解 |
| [16](#16-const-成员函数与-const-指针-详解) | const 成员函数与 const 指针 详解 |
| [17](#17-c-四种-cast-详解) | C++ 四种 cast 详解 |
| [18](#18-namespace-与匿名-namespace--详解) | `namespace` 与匿名 `namespace {}` 详解 |
| [19](#19-stddeque-详解) | `std::deque` 详解 |
| [20](#20-stdifstream-详解) | `std::ifstream` 详解 |
| [21](#21-拷贝移动五大函数速查-详解) | 拷贝/移动五大函数速查 详解 |
| [22](#22-stdisspace-详解) | `std::isspace` 详解 |
| [23](#23-error-模式-详解) | Error 模式最佳实践 详解 |

---

## 1. `[[nodiscard]]` 详解

> 📋 [速查表 #1](./modern-cpp-cheatsheet.md#1-nodiscard)

### 为什么需要它

C++ 的返回值没有强制性 —— 调用者完全可以忽略返回值。在"返回值代表错误码"的年代（C 语言风格），这导致了无数难以排查的 bug：

```cpp
// C 经典 bug：忽略了表示失败的返回值
close(fd);           // 返回值被忽略 — 可能静默失败
read(fd, buf, 100);  // 可能读了一半，没人知道

// [[nodiscard]] 让编译器帮你强制检查
[[nodiscard]] Result<int> compute();
compute();  // ⚠️ 编译警告: ignoring return value of 'compute()'
```

### 内部机制

`[[nodiscard]]` 是 C++17 标准属性（attribute），不是关键字。编译器可以选择忽略它（只是"建议"），但主流的 GCC、Clang、MSVC 都会产生警告。

C++20 扩展了两点：
1. **可附加诊断字符串**：`[[nodiscard("reason")]]`
2. **可用于构造函数**：防止创建临时匿名对象后丢弃

### C++20 诊断字符串的实战价值

```cpp
// 不附加字符串：警告信息模糊
[[nodiscard]] Handle acquire();
acquire();  // "warning: ignoring return value"

// 附加字符串：告诉调用者具体问题
[[nodiscard("leaking KV cache handle — must call release()")]]
Handle acquire();
// "warning: ignoring return value of 'acquire()', declared with 'nodiscard':
//  leaking KV cache handle — must call release()"
```

### 何时不加 `[[nodiscard]]`

并非所有返回值的函数都需要。以下情况通常不需要：

```cpp
// operator<< 返回 ostream& 是惯例，无需 nodiscard
std::ostream& operator<<(std::ostream& os, const Foo& f);

// 自增运算符返回引用，常以链式调用方式使用但也可以不用
// 不过实践中也常常忽略其返回值
```

### 与 `Result<T>` 模式的配合

TinyInfer 使用 `Result<T, Error>`（类似于 Rust 的 `Result`）—— 这种类型**几乎永远应该加 `[[nodiscard]]`**：

```cpp
template <typename T>
class [[nodiscard]] Result { /* ... */ };  // 整个类标记

// 或者只标记关键访问函数
[[nodiscard]] const Error& error() const noexcept;
```

---

## 2. `noexcept` 详解

> 📋 [速查表 #2](./modern-cpp-cheatsheet.md#2-noexcept)

### 不是"这个函数可能会抛异常"的检查

很多人误解 `noexcept` 的作用。**`noexcept` 是一个承诺，不是检查**：

```cpp
void f() noexcept {
  throw std::runtime_error("oops");  // 编译通过！但运行时 → std::terminate()
}
```

如果 `noexcept` 函数抛出了异常，运行时会直接调用 `std::terminate()`，而**不是**把异常传播给调用者。这就是为什么不能对可能抛异常的函数加 `noexcept`。

### 为什么析构函数默认 noexcept

```cpp
~Foo() { /* ... */ }  // 隐式 noexcept(true)
```

原因是：如果析构函数在堆栈展开（stack unwinding，即另一个异常正在传播中）时又抛出异常，C++ 运行时会调用 `std::terminate()`。这是双重异常规则（two-exceptions rule）。

因此**析构函数不应该抛异常**，编译器也默认标记为 `noexcept(true)`。

### `noexcept` 运算符的编译期魔法

```cpp
noexcept(expr)   // 返回 bool，编译期常量
```

这个运算符**不执行表达式**，只从类型系统推断该表达式是否可能抛出。例如：

```cpp
noexcept(1 + 1)           // true — 整型运算不抛
noexcept(new int[100])    // false — new 可能抛 std::bad_alloc
noexcept(std::declval<T&>().size())  // 取决于 T::size() 的 noexcept 声明
```

### TinyInfer 中的实际用法

```cpp
// scope_guard.h: "析构的 noexcept 取决于存储的函数是否 noexcept"
~ScopeGuard() noexcept(noexcept(f_()))

// 含义拆解:
//   - 外层的 noexcept(条件)：如果条件为 true，则析构函数是 noexcept
//   - 内层的 noexcept(f_())：编译期检查 f_() 这个表达式是否抛异常
//   - 所以：f_() 不抛 → 析构 noexcept(true)；f_() 可能抛 → 析构 noexcept(false)
```

### 移动构造与 `noexcept`

这是一个**性能问题**。标准库容器在扩容时，如果元素的移动构造不是 `noexcept`，会回退到拷贝：

```cpp
class Foo {
  Foo(Foo&&) = default;  // 如果成员都是 noexcept 移动，这个就是 noexcept
  // 如果没有 noexcept，vector<Foo> 扩容时会拷贝所有元素，而不是移动
};
```

这就是为什么一定要写 `noexcept` 移动构造的原因（或用 `= default` 让编译器推导）。

---

## 3. `constexpr` / `inline constexpr` 详解

> 📋 [速查表 #3](./modern-cpp-cheatsheet.md#3-constexpr--inline-constexpr)

### `const` vs `constexpr` 的本质区别

```cpp
const int x = rand();     // ✅ 运行期确定，但此后不可变
constexpr int y = rand(); // ❌ 编译错误！rand() 不是 constexpr 函数

// const: 承诺"初始化后不变"，初始化可以在运行期
// constexpr: 要求"值在编译期就必须已知"
```

### 为什么头文件里要用 `inline constexpr` 而非 `constexpr`

这是 C++ 的**单一定义规则（ODR, One Definition Rule）**问题：

```cpp
// types.h — 被多个 .cpp 包含
constexpr int kMax = 100;  // 每个包含 types.h 的 .cpp 都会生成一个 kMax
                           // 链接时可能报 "multiple definition" 错误
                           // (具体情况取决于是否 odr-used)

// C++17 解决方案：inline 关键字同时适用于变量
inline constexpr int kMax = 100;  // ✅ 链接器会合并所有定义
```

### `constexpr` 函数的演进

C++11 中 `constexpr` 函数只能包含一条 `return` 语句。从 C++14 起，可以在函数中包含循环、分支和局部变量：

```cpp
// C++14 constexpr 函数
constexpr int factorial(int n) {
  int result = 1;
  for (int i = 2; i <= n; ++i) result *= i;
  return result;
}
static_assert(factorial(5) == 120);  // 编译期计算
```

### C++20 新增：`consteval` 和 `constinit`

```cpp
// consteval：必须在编译期调用（比 constexpr 更严格）
consteval int compile_only(int x) { return x * x; }
int r = compile_only(4);      // ✅ 编译期
// int v = compile_only(argc); // ❌ 编译错误！必须在编译期

// constinit：变量必须在编译期初始化，但此后可以修改
constinit int g_counter = 0;  // 编译期初始化，运行期可改
```

---

## 4. `enum class` 详解

> 📋 [速查表 #4](./modern-cpp-cheatsheet.md#4-enum-class)

### 旧式 `enum` 的三个致命问题

```cpp
// 问题 1：命名空间污染
enum Color { red, green, blue };
enum Traffic { red, yellow, green };  // ❌ red 和 green 被重复定义了

// 问题 2：隐式转 int
Color c = red;
int x = c;          // ✅ 编译通过，但 Color 不应参与算术
if (c == 0) { }     // ✅ 编译通过，但语义诡异 (red == 0?)

// 问题 3：底层类型不确定
enum Flags { A = 1, B = A << 31 };  // 可能溢出（enum 底层可能是 int16）
```

`enum class` 解决了全部三个问题：

```cpp
enum class Color { kRed, kGreen, kBlue };
enum class Traffic { kRed, kYellow, kGreen };  // ✅ 无冲突
// int x = Color::kRed;                         // ❌ 编译错误
int x = static_cast<int>(Color::kRed);          // ✅ 显式转换
enum class Flags : uint32_t { A = 1, B = A << 31 };  // 指定底层类型
```

### 底层类型的内存影响

```cpp
enum class Small : uint8_t { A, B, C };        // sizeof = 1
enum class Medium : uint16_t { A, B, C };      // sizeof = 2
enum class Default { A, B, C };                // sizeof = 4 (默认 int)
```

对于 TinyInfer 的 `ErrorCode` 这种场景，默认 `int` 就够了。但在需要用 `enum class` 构造紧凑数组时，`uint8_t` 能显著节省内存。

### 前向声明

```cpp
// ✅ enum class 可以前向声明（为编译器提供完整类型信息）
enum class ErrorCode : int;   // 指定底层类型即可前向声明

// ❌ 普通 enum 前向声明需要指定类型，但常被忽略
```

---

## 5. `= default` / `= delete` 详解

> 📋 [速查表 #5](./modern-cpp-cheatsheet.md#5--default--delete-详解)

### 编译器自动生成的特殊成员函数

C++ 编译器会在需要时自动生成以下六个特殊成员函数：

```cpp
class Foo {
  Foo();                        // 1. 默认构造
  ~Foo();                       // 2. 析构
  Foo(const Foo&);              // 3. 拷贝构造
  Foo& operator=(const Foo&);   // 4. 拷贝赋值
  Foo(Foo&&);                   // 5. 移动构造 (C++11)
  Foo& operator=(Foo&&);        // 6. 移动赋值 (C++11)
};
```

**一旦你手动声明了其中任何一个，编译器的生成规则就会改变。** 最经典的规则：

```
- 声明了拷贝构造 → 移动构造/赋值不再自动生成
- 声明了移动构造 → 拷贝构造/赋值被 = delete
- 声明了析构函数 → 移动构造/赋值不再自动生成（向后兼容 C++98）
```

### `= default` 的实战意义

```cpp
class MyClass {
 public:
  MyClass() = default;                 // "用编译器版本，但显式说明我想要它"
  ~MyClass() = default;                // 虚析构 + default 是常见组合
  MyClass(const MyClass&) = default;   // 显式声明拷贝构造是安全的
};
```

### `= delete` 不只是用于禁止拷贝

```cpp
// 1. 禁止特定参数类型（阻止隐式转换）
void setAge(int age);
void setAge(double) = delete;    // 阻止 setAge(3.14)
void setAge(const char*) = delete; // 阻止 setAge("hello")

// 2. 禁止堆分配
class StackOnly {
  void* operator new(size_t) = delete;    // 禁止 new
  void* operator new[](size_t) = delete;  // 禁止 new[]
};

// 3. 禁止特定模板实例化
template <typename T>
void process(T* ptr) {
  ptr->run();
}
template <>
void process<void>(void*) = delete;  // 不允许 T=void
```

---

## 6. `std::move` / `std::forward` 详解

> 📋 [速查表 #6](./modern-cpp-cheatsheet.md#6-stdmove--stdforward)

### 值类别体系：不仅仅是"左值"和"右值"

C++ 的值类别实际上有三层结构（C++11 起）：

```
            expression
            /        \
    glvalue          rvalue
    /     \          /    \
lvalue   xvalue   prvalue
```

- **lvalue**：有身份、不可移动（变量名、解引用指针）
- **xvalue**：有身份、可移动（`std::move(x)` 的结果）
- **prvalue**：无身份、可移动（临时对象、字面量）

`glvalue` = lvalue + xvalue（"有身份"）
`rvalue` = prvalue + xvalue（"可移动"）

### `std::move` 的内部实现

```cpp
// std::move 就是一层的 static_cast
template <typename T>
constexpr std::remove_reference_t<T>&& move(T&& t) noexcept {
  return static_cast<std::remove_reference_t<T>&&>(t);
}
```

它不移动任何东西。它只把值类别转成 `xvalue`（右值引用），真正的移动发生在接收方的移动构造/赋值中。

### 完美转发的折叠规则

```cpp
template <typename F>
void wrap(F&& f) {
  // F&& 是转发引用（forwarding reference），不是普通的右值引用
  // 当实参是左值时，F 推导为 T&，   F&& 折叠为 T&
  // 当实参是右值时，F 推导为 T，    F&& = T&&
  g(std::forward<F>(f));
}
```

引用折叠规则：

| 模板参数 | 实参类型 | `F` 推导为 | `F&&` 折叠为 |
|---------|---------|-----------|-------------|
| `F&&` | lvalue of `T` | `T&` | `T& &&` → `T&` |
| `F&&` | rvalue of `T` | `T` | `T&&` |

### 最常见错误

```cpp
// ❌ 错误：对同一个对象多次 move
std::string s = "hello";
std::string a = std::move(s);
std::string b = std::move(s);  // s 处于"已移动"状态，内容未知

// ❌ 错误：对 const 对象 move（不会移动，退化为拷贝）
const std::string s = "hello";
std::string a = std::move(s);  // 调用的是拷贝构造（const T&& 不匹配 T&&）

// ✅ 正确：move 后不再使用
```

---

## 7. `explicit` 详解

> 📋 [速查表 #7](./modern-cpp-cheatsheet.md#7-explicit)

### 隐式转换带来的隐患

```cpp
// 没有 explicit 时，编译器可以"帮你"做惊人的事
class String {
 public:
  String(int capacity);  // 不是 explicit
};

void takeString(const String& s);

takeString(42);  // ✅ 编译通过！42 被隐式转成 String(42)
// 调用方想传一个字符串，结果传了个 int，编译器却默默接受了
```

### `explicit` 的原理

`explicit` 告知编译器："禁止使用这个构造函数/转换运算符做隐式转换"。它只影响以下两种场景：

1. **拷贝初始化**（`=` 语法）：`String s = 42;`
2. **函数参数传递**：`takeString(42);`

不影响：
- **直接初始化**：`String s(42);`
- **显式转换**：`static_cast<String>(42);`
- **列表初始化**（取决于语境）

### C++20 的 `explicit(bool)`

C++20 引入了条件 explicit：

```cpp
// 只有当 T 不是整数类型时，构造函数才是 explicit
template <typename T>
explicit(!std::is_integral_v<T>) S(T value);

S(42);       // implicit（T = int）
S(3.14);     // explicit（T = double，非整数）
```

### TinyInfer 中的应用

```cpp
// scope_guard.h: 单参数构造函数必须 explicit
explicit ScopeGuard(F&& f);

// result.h: operator bool 必须 explicit，阻止出现在算术运算中
explicit operator bool() const noexcept;
```

---

## 8. `static_assert` + type traits 详解

> 📋 [速查表 #8](./modern-cpp-cheatsheet.md#8-static_assert--type-traits)

### 为什么用 `static_assert` 而不是运行期断言

```cpp
// ❌ 运行期才发现问题 — 太晚了
assert(sizeof(MyStruct) == 64);  // 运行时检查，可能永远不会触发

// ✅ 编译期就报错 — 零成本
static_assert(sizeof(MyStruct) == 64, "MyStruct must be 64 bytes");
```

`static_assert` 的检查发生在编译期，不产生任何运行时代码。如果条件为 `false`，编译直接终止。

### Type traits 的实现原理

Type traits 不是编译器魔术 —— 它们大多可以用普通的模板特化实现：

```cpp
// std::is_same 的简化实现
template <typename T, typename U>
struct is_same : std::false_type {};

template <typename T>
struct is_same<T, T> : std::true_type {};

// C++14/17 的 _v 后缀
template <typename T, typename U>
inline constexpr bool is_same_v = is_same<T, U>::value;
```

这意味着你可以写自己的 type traits：

```cpp
// 自定义：检测类型是否有 .size() 方法
template <typename T, typename = void>
struct has_size : std::false_type {};

template <typename T>
struct has_size<T, std::void_t<decltype(std::declval<T>().size())>> : std::true_type {};
```

### `if constexpr`：编译期条件分支

C++17 引入的 `if constexpr` 是 `static_assert` 的最佳搭档：

```cpp
template <typename T>
auto get_value(const T& val) {
  if constexpr (std::is_pointer_v<T>) {
    return *val;           // 编译期确定走这个分支，不要求 T 有 operator*
  } else {
    return val;
  }
}
```

注意：`if constexpr` 的条件必须在编译期可确定，且 `false` 分支的代码**不会**被实例化（discarded statement 规则）。

### 项目中的组合使用

```cpp
// result.h
static_assert(!std::is_reference_v<T>,  "Result<T> does not support references");
static_assert(!std::is_same_v<T, void>, "Use Result<void> pattern");
```

这两个断言在模板实例化时立即检查：如果有人写 `Result<int&>` 或 `Result<void>`，编译失败并显示清晰的错误信息，而不是在几千行后的调用处产生难以理解的模板错误。

---

## 9. `std::variant` 详解

> 📋 [速查表 #9](./modern-cpp-cheatsheet.md#9-stdvariant)

### 与 `union` 的本质区别

```cpp
// C union：不知道当前存的是哪个类型
union OldUnion {
  int i;
  double d;
  std::string s;  // ❌ 直到 C++11 才能放有构造函数的类型
};
// 只能靠外部维护一个 enum 来判断当前类型

// std::variant：类型安全，知道存的是什么
std::variant<int, double, std::string> v;
v = "hello";
// v.index() == 2，库内部记录了当前活跃类型
```

### 内部实现概述

`std::variant` 通常用以下方式实现（简化）：

```cpp
// 概念性实现（并非标准库的实际代码）
template <typename... Ts>
class variant {
  // 一块对齐到最大成员的内存
  alignas(Ts...) unsigned char storage_[max(sizeof(Ts)...)];
  int index_;  // 当前活跃的是第几个类型

  // 类型切换时的构造/析构
  template <typename T>
  variant& operator=(T&& value) {
    if (index_ != invalid) {
      destroy_current();  // 析构旧的
    }
    new (storage_) T(std::forward<T>(value));  // placement new 新的
    index_ = find_index<T>();
    return *this;
  }
};
```

关键点：`sizeof(std::variant<Ts...>)` ≥ `max(sizeof(Ts)...)` + `sizeof(index_t)`。额外开销只是那个小的 index 字段（通常 1-4 字节）+ 对齐填充。

### 访问模式性能排序

```cpp
// 🏆 最快：std::get_if — 指针判空，无异常开销
if (auto* p = std::get_if<int>(&v)) { use(*p); }

// 🥈 中等：std::get — 类型匹配时零开销，不匹配时抛异常
int x = std::get<int>(v);

// 🥉 最灵活但最大代码膨胀：std::visit
std::visit([](auto&& val) { /* ... */ }, v);
```

**TinyInfer 的 `Result<T>` 使用 `std::variant<T, Error>` —— 通过 `ok()` 检查后再调用 `value()`/`error()`，本质上是 `std::get_if` 的安全封装。**

### 空状态问题

```cpp
std::variant<int, std::string> v;
// v 不是空的 — 它会被默认构造成 int{}(即 0)

// 如果真的需要"空"状态
std::variant<std::monostate, int, std::string> v2;  // monostate 是空的占位类型
```

---

## 10. `std::optional` 详解

> 📋 [速查表 #10](./modern-cpp-cheatsheet.md#10-stdoptional)

### 与指针相比的优势

```cpp
// 指针方案：语义模糊
int* maybe_age = nullptr;     // 空是什么意思？未设置？0岁？不合法？
                              // 调用方会忘记判空
void process(int* age) {
  *age;  // 可能空指针解引用
}

// optional 方案：语义清晰
std::optional<int> maybe_age = std::nullopt;
void process(std::optional<int> age) {
  age.value();  // 抛异常 bad_optional_access，不会静默错误
}
```

### 内存布局

`std::optional<T>` 的典型布局：

```cpp
// 概念性实现
template <typename T>
class optional {
  struct empty_byte {};
  union {
    empty_byte empty_;
    T value_;
  };            // sizeof = max(sizeof(T), 1)
  bool has_value_;  // sizeof = 1
  // 实际 sizeof ≥ sizeof(T) + 1 + padding
};
// 例如 sizeof(optional<int>)    = 8   (4 + 1 + 3 padding)
// 例如 sizeof(optional<double>) = 16  (8 + 1 + 7 padding)
```

### 何时用 `optional` vs `variant`

```
optional<T>   → 只有"有值/T"和"无值/nullopt"两种状态
variant<T, Error> → 有值/T、有错误/Error 两种状态，但都能携带数据

TinyInfer 的选择：
  - Result<T> 内部用 variant<T, Error>  — 失败时有错误信息
  - Result<void> 内部用 optional<Error>  — 成功时无值，失败时有错误
```

---

## 11. `std::string_view` 详解

> 📋 [速查表 #11](./modern-cpp-cheatsheet.md#11-stdstring_view)

### 内部结构

```cpp
// string_view 极其轻量 —— 一个指针 + 一个长度
class string_view {
  const char* data_;   // 指向已有数据的指针（不拥有）
  size_t size_;        // 长度
};
// sizeof(string_view) = 16 (64位系统)
```

### 生命周期陷阱（最常见 Bug）

```cpp
// ❌ 返回了悬空 string_view
std::string_view get_name() {
  std::string name = "temporary";  // 局部变量
  return std::string_view(name);   // 返回后 name 析构 → 悬空!
}

// ❌ string 临时对象被绑定到 string_view
std::string_view sv = std::string("hello") + " world";
// 临时 string 在这行结束后析构 → sv 悬空!

// ✅ 正确：string_view 的生命周期必须在原始数据之内
std::string data = std::string("hello") + " world";
std::string_view sv = data;  // OK，data 还活着
```

### 性能分析

```cpp
// 零分配子串
void parse(std::string_view line) {
  auto key = line.substr(0, line.find(':'));    // O(1) — 不分配
  auto val = line.substr(line.find(':') + 1);   // O(1) — 不分配
  // 相比之下 line.substr() 返回 std::string 会分配新内存
}

// 比较也是 O(1) 或 O(n) 但不分配
if (key == "hello") { }  // 从头到尾比较，但无内存分配
```

### 什么时候用 `string_view` 做函数参数

```
✅ string_view   — 函数只读取字符串内容（不拥有、不修改）
✅ const string& — 必须传 std::string 给 C++14 或更低版本
✅ string        — 函数需要持有字符串的拷贝
❌ string_view   — 函数需要字符串的拷贝（会导致二次分配）
❌ string_view   — 需要 null-terminated 字符串传 C API
```

---

## 12. `#pragma once` 详解

> 📋 [速查表 #12](./modern-cpp-cheatsheet.md#12-pragma-once)

### 工作机制

`#pragma once` 告诉预处理器：**这个文件在一个翻译单元中只处理一次**。预处理器遇到第二次 `#include` 同一文件时，直接跳过。

### `#pragma once` vs include guards

```cpp
// 传统 include guard
#ifndef TINYINFER_COMMON_ERROR_H_
#define TINYINFER_COMMON_ERROR_H_
// ...
#endif

// #pragma once
#pragma once
// ...
```

| 维度 | `#pragma once` | include guards |
|------|---------------|----------------|
| 代码量 | 1行 | 3行 |
| 宏名冲突 | 无 | 需要手动保证唯一 |
| 标准 | 事实标准（所有主流编译器支持） | C++ 标准 |
| 硬链接/符号链接 | 可能重复包含同一物理文件 | 不会 |
| 可移植性 | 极边缘平台不支持 | 完全可移植 |

### 什么场景下 `#pragma once` 可能失效

唯一已知的缺陷场景：**硬链接**。如果同一个物理文件通过两个硬链接路径被包含，预处理器可能认为它们是不同的文件。

这是极其边缘的情况，在实际项目中几乎不会遇到。C++ 标准委员会在 C++23 中仍在讨论是否标准化 `#pragma once`。

---

## 13. CTAD 详解

> 📋 [速查表 #13](./modern-cpp-cheatsheet.md#13-ctad)

### 为什么 C++17 之前 `make_*` 函数满天飞

```cpp
// C++14 之前：必须显式写模板参数
auto p = std::pair<int, double>(1, 3.14);
auto v = std::vector<int>{1, 2, 3};

// 于是标准库提供了 make_* 函数来绕过（利用函数模板的推导能力）
auto p = std::make_pair(1, 3.14);         // make_pair 是函数模板，可推导
auto v = std::make_shared<Foo>(arg1);     // make_shared 减少一次 new
auto l = std::make_unique<Foo>(arg1);     // C++14 才有 make_unique

// C++17 CTAD 后：构造函数自己就能推导
std::pair p(1, 3.14);          // 推导为 pair<int, double>
std::vector v{1, 2, 3};        // 推导为 vector<int>
auto u = std::unique_lock(m);  // 推导 mutex 类型
```

### 推导规则的工作原理

CTAD 有两种推导方式：

1. **隐式推导指引**：编译器自动从构造函数中推导
2. **显式推导指引**：开发者自定义

```cpp
// scope_guard.h 中的显式推导指引
template <typename F>
ScopeGuard(F) -> ScopeGuard<F>;
// 含义：如果有人写 ScopeGuard(lambda)，推导出 F = decltype(lambda)
// 如果没有这条指引，编译器需要从构造函数推导
// 显式指引的好处：更精确的控制（比如可以做 decay、remove_ref 等）
```

### 推导失败时

```cpp
// 如果构造函数的参数不足以推导模板参数，CTAD 会失败
template <typename T>
class Foo {
  Foo();  // 无参构造 → 无法推导 T
};

// Foo f;  // ❌ 编译错误：无法推导 T
Foo<int> f;  // ✅ 必须显式指定
```

---

## 14. `std::chrono` 详解

> 📋 [速查表 #14](./modern-cpp-cheatsheet.md#14-stdchrono)

### 为什么不用 `int64_t` 直接存毫秒

```cpp
// ❌ 单位不清，容易出错
void sleep(int64_t ms);          // 毫秒？
void timeout(int64_t us);        // 微秒？
timeout(sleep_ms);               // 编译通过，但传错了单位 → bug

// ✅ std::chrono：类型自带单位
void sleep(std::chrono::milliseconds d);
void timeout(std::chrono::microseconds d);
timeout(sleep_ms);  // ❌ 编译错误：milliseconds 不能隐式转 microseconds
```

### 时钟类型选择

| 时钟 | 特点 | 适用场景 |
|------|------|---------|
| `steady_clock` | 单调递增，不受系统时间调整影响 | 性能测量、超时（**TinyInfer 的选择**） |
| `system_clock` | 表示墙上时间，可调 | 日志时间戳 |
| `high_resolution_clock` | 通常就是 steady_clock 的别名 | 微基准测试 |

```cpp
// TinyInfer 的 Clock 类就基于 steady_clock
// 因为调度器的超时检查不能受系统时间调整的影响
using TimePoint = std::chrono::steady_clock::time_point;
```

### 时长转换的危险操作

```cpp
// truncating_cast：会丢失精度
auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::nanoseconds(1'500'000));  // = 1ms（丢弃了 500'000ns）

// 如果需要舍入而非截断，需要 C++17 的 round
auto ms2 = std::chrono::round<std::chrono::milliseconds>(
    std::chrono::nanoseconds(1'500'000));  // = 2ms（四舍五入）
```

### C++20 新增

C++20 的 `<chrono>` 增加了日历、时区和格式化支持。但本项目的底层系统不依赖这些，仍使用 C++17 子集。

---

## 15. RAII / ScopeGuard 详解

> 📋 [速查表 #15](./modern-cpp-cheatsheet.md#15-raii--scopeguard)

### RAII 为什么是 C++ 最重要的惯用法

```cpp
// C 风格错误处理：goto cleanup 模式
int do_work() {
  void* res1 = acquire1(); if (!res1) goto err1;
  void* res2 = acquire2(); if (!res2) goto err2;
  void* res3 = acquire3(); if (!res3) goto err3;
  // ...
  return 0;
err3: release2(res2);
err2: release1(res1);
err1: return -1;
}
// 18行，4个标签，容易写错释放顺序

// C++ RAII：0个标签，0个 goto
int do_work() {
  auto res1 = Acquire1();
  auto res2 = Acquire2();
  auto res3 = Acquire3();
  return 0;  // 自动逆序析构！
}
// 6行，不可能写错释放顺序
```

RAII 的核心原理：**C++ 保证局部对象的析构函数在离开作用域时一定被调用**（包括通过 `return`、异常和 `break` 离开）。

### ScopeGuard 的实现细节

```cpp
template <typename F>
class ScopeGuard {
  F f_;
  bool active_;

  // 移动构造：把源 guard 的 active_ 置为 false
  ScopeGuard(ScopeGuard&& other) noexcept
      : f_(std::move(other.f_)), active_(other.active_) {
    other.dismiss();  // 关键！防止源 guard 析构时也调用 f_
  }
  // 如果不这样处理，move 后两个 guard 析构都会调用 f_
};

// 使用模式
auto g = ScopeGuard([&] {
  kv_cache_manager.release(handle);  // 无论如何保证释放
});
// ... 可能抛异常的代码 ...
g.dismiss();  // 成功时不释放（已正常完成）
```

### 不能替代完整的所有权系统

ScopeGuard 适合 ad-hoc 的一次性清理。对于需要跨函数传递所有权的资源（如 `KVCacheHandle`），应该用完整的 RAII 类：

```cpp
class KVCacheGuard {
  KVCacheManager& mgr_;
  KVCacheHandle handle_;
 public:
  ~KVCacheGuard() { if (valid_) mgr_.release(handle_); }
  void dismiss() { valid_ = false; }
  // 移动构造/赋值...
};
```

---

## 16. const 成员函数与 const 指针 详解

> 📋 [速查表 #16](./modern-cpp-cheatsheet.md#16-const-成员函数与-const-指针)

### 后置 const：对调用者的契约

```cpp
const Request* front() const;
//                     ↑↑↑↑↑
```

函数末尾的 `const` 是**对调用者的承诺**："调用我不会修改你的对象"。

编译器的实际保证：
1. 函数体内部 `this` 指针的类型是 `const T*`（不是 `T*`）
2. 不能修改任何非 `mutable` 的成员变量
3. 不能调用同类中没有后置 `const` 的非静态成员函数

```cpp
class Container {
  std::vector<int> data_;
 public:
  size_t size() const { return data_.size(); }  // ✅ const 函数

  void clear() { data_.clear(); }               // 非 const 函数

  size_t broken() const {
    // clear();     // ❌ 编译错误！const 函数不能调用非 const 函数
    return data_.size();
  }
};
```

### const 对象只能调用 const 函数

这是最常见的"为什么编译报错"原因：

```cpp
const Container c;
c.size();   // ✅ size() 是 const 的
// c.clear(); // ❌ clear() 不是 const 的
```

### 何时提供 const / 非 const 重载

标准库容器经常成对提供：

```cpp
// 普通对象调用 → 返回可修改引用
Request* front();

// const 对象调用 → 返回只读指针
const Request* front() const;
```

编译器根据 `this` 的 const 属性自动选择正确的重载。

### 返回值前 const 的三种位置

```cpp
// ① const T*：指向内容不可改，指针本身可改
const Request* p = get();
p->data = 100;  // ❌ 内容只读
p = nullptr;     // ✅

// ② T* const：指针不可改，指向内容可改
Request* const p = &req;
p = nullptr;     // ❌ 指针是 const

// ③ const T* const：都不可改
const Request* const p = get();
```

---

## 17. C++ 四种 cast 详解

> 📋 [速查表 #17](./modern-cpp-cheatsheet.md#17-c-四种-cast)

### 为什么需要四种而不是一种

C 风格的 `(Type)value` 太粗暴，什么都能转，无法区分"安全的整数转换"和"危险的指针重解释"。

C++ 四种 cast 让你**在代码中表达转换意图**，便于代码审查和工具检查。

### `static_cast`：编译期合理转换

```cpp
// 能做：
static_cast<int>(3.14);                    // 数值类型转换
static_cast<unsigned char>(ch);            // 有符号 → 无符号
static_cast<int*>(void_ptr);              // void* 转回原类型
static_cast<Base*>(&derived);             // 向上转换（安全）
static_cast<int>(ErrorCode::kOk);         // enum ↔ int

// 不能做：
// static_cast<int*>(float_ptr);           // ❌ 完全不相关的指针
// static_cast<Derived*>(&base);           // ❌ 危险的向下转换（用 dynamic_cast）
```

### `dynamic_cast`：多态向下转型（运行时检查）

```cpp
Base* b = get_base();
if (auto* d = dynamic_cast<Derived*>(b)) {
  // 确实是 Derived，安全使用
}
// 失败返回 nullptr（指针）或抛 std::bad_cast（引用）
```

代价：依赖 RTTI（Run-Time Type Information），有运行时开销。大型推理框架有时会禁用 RTTI 以减小二进制体积，此时 `dynamic_cast` 不可用。

### `const_cast`：只改 const 属性

```cpp
const std::string& getData() const;
// 需要传给一个接受 string& 的遗留 API 时
// const_cast<std::string&>(getData());  // ⚠️ 危险！仅用于遗留兼容
```

**90% 的情况下不需要 `const_cast`。** 如果你觉得需要它，先检查是否设计有问题。

### `reinterpret_cast`：比特重解释（最后手段）

```cpp
int x = 42;
float* fp = reinterpret_cast<float*>(&x);  // 把 int 的比特当 float 读
```

**在 TinyInfer 中永远不要用 `reinterpret_cast`。** 这属于底层驱动/序列化的领域。

### 选择决策树

```
需要转换类型？
├── 数值/父类指针/enum → static_cast
├── 多态向下转型 → dynamic_cast
├── 只去除 const → const_cast（再想想是不是真的需要）
└── 以上都不是 → reinterpret_cast（再想想是不是设计有误）
```

---

## 18. `namespace` 与匿名 `namespace {}` 详解

> 📋 [速查表 #18](./modern-cpp-cheatsheet.md#18-namespace-与匿名-namespace-)

### 具名 namespace：代码组织

```cpp
namespace tinyinfer {
  // 外部通过 tinyinfer::Error 访问
  class Error { /* ... */ };
}
```

### 匿名 namespace：文件级私有

```cpp
namespace {
  // 仅当前 .cpp 可见 — 等效于 C 语言标记为 static 的全局函数
  std::string trim(const std::string& s) { /* ... */ }
}
```

C++ 标准规定：匿名 namesapce 中的内容具有**内部链接**（internal linkage），其他翻译单元无法引用。

### 为什么不直接用 `static` 全局函数

```cpp
// C 风格
static std::string trim(const std::string& s) { /* ... */ }

// C++ 风格（推荐）
namespace {
  std::string trim(const std::string& s) { /* ... */ }
}
```

匿名 namespace 的优势：
1. 可以放类、枚举、typedef，不只是函数
2. 语义更统一（"namespace 控制可见性"）
3. C++ 标准推荐匿名 namespace 替代文件级 `static`

### 头文件中绝对禁止

```cpp
// ❌❌❌ 头文件中千万不要写这个！
// 每个包含此头文件的 .cpp 都会得到独立的匿名命名空间
// 函数会被复制多份，增大二进制体积
namespace {
  inline void helper() { /* ... */ }
}
```

---

## 19. `std::deque` 详解

> 📋 [速查表 #19](./modern-cpp-cheatsheet.md#19-stddeque)

### 底层结构

`deque` 不是一整块连续内存，而是**分段连续**：由一个指针数组指向多块固定大小的内存块（chunk）。

```
 deque<int> dq{1,2,3,4,5,6,7,8};
 
 map ──→ [ptr0] [ptr1] [ptr2]
           │      │      │
           ▼      ▼      ▼
         [1 2 3] [4 5 6] [7 8]
```

### vs vector 的取舍

```cpp
// 选 vector 当：
//   - 只需要在尾部增删
//   - 需要连续内存（传 C API）
//   - 需要更好的缓存局部性

// 选 deque 当：
//   - 需要在头部插入/删除（vector 的 push_front 是 O(n)！）
//   - 元素很大且不想扩容时整体搬迁
//   - 实现工作窃取队列、BFS 队列等
```

### push_back 不会让引用失效

与 `vector` 不同，`deque` 在头尾插入时**不会搬迁已有元素**，所以已有元素的引用和指针保持有效（只要不是插入在中间）。

---

## 20. `std::ifstream` 详解

> 📋 [速查表 #20](./modern-cpp-cheatsheet.md#20-stdifstream)

### 构造与打开

```cpp
#include <fstream>

// 方式 1：构造时打开
std::ifstream file("data.txt");

// 方式 2：先构造，后打开
std::ifstream file;
file.open("data.txt");
```

### 状态检查

```cpp
if (!file.is_open()) { /* 打开失败 */ }
if (!file) {           /* 流错误（等价） */ }
if (file.fail()) {     /* 逻辑错误（如格式不匹配） */ }
if (file.eof()) {      /* 到达文件尾 */ }
```

### 为什么不需要手动 close

`std::ifstream` 是 RAII 类——析构函数自动调用 `close()`。

```cpp
void parse(const std::string& path) {
  std::ifstream file(path);
  // ...
}  // ← file 析构，自动关闭，即使中间抛异常也会关闭
```

### 读取模式

```cpp
// 逐行
std::string line;
while (std::getline(file, line)) { /* ... */ }

// 逐词
std::string word;
while (file >> word) { /* ... */ }

// 一次性读全部
std::stringstream buffer;
buffer << file.rdbuf();
std::string content = buffer.str();
```

---

## 21. 拷贝/移动五大函数速查 详解

> 📋 [速查表 #21](./modern-cpp-cheatsheet.md#21-拷贝移动五大函数速查)

### 编译器自动生成规则

编译器在你不手写时自动生成最多五个函数：

```
① 默认构造 T()
② 拷贝构造 T(const T&)
③ 拷贝赋值 T& operator=(const T&)
④ 移动构造 T(T&&)           ← C++11
⑤ 移动赋值 T& operator=(T&&)  ← C++11
⑥ 析构函数 ~T()
```

### 关键连锁规则

```
规则 1：如果你手动定义了 ②/③/⑥ 中任意一个
        → 编译器不再生成 ④/⑤（移动构造/赋值）

规则 2：如果你手动定义了 ④/⑤
        → 编译器 delete 掉 ②/③（拷贝构造/赋值）

规则 3：默认生成的拷贝永远是浅拷贝（逐成员 bitwise copy）
```

### 为什么规则 1 存在

向后兼容 C++98。C++98 代码中手动实现拷贝构造/析构的类，如果 C++11 自动生成移动构造，可能破坏原有行为。

### 浅拷贝的灾难（为什么需要 NonCopyable）

```cpp
struct Buffer {
  char* data;
  Buffer(size_t n) : data(new char[n]) {}
  ~Buffer() { delete[] data; }
  // 拷贝构造是编译器默认生成 → 浅拷贝！
};

Buffer a(1024);
Buffer b = a;  // b.data == a.data → 双重释放 → UB
```

这就是 TinyInfer 中 `NonCopyable` 的完整动机：**持有堆/显存/内核资源的类，浅拷贝直接灾难，必须 `= delete` 禁止。**

---

## 22. `std::isspace` 详解

> 📋 [速查表 #22](./modern-cpp-cheatsheet.md#22-stdisspace)

### 未定义行为根源

```cpp
// char 在大多数平台是有符号类型（-128 ~ 127）
char ch = '中';  // 多字节字符，某个字节可能 > 127

// std::isspace 的签名：int isspace(int ch);
// 参数必须是 unsigned char 转换来的 int 或 EOF
// 如果传入负数 → 访问负索引 → UB!
std::isspace(ch);  // ❌ UB

// 正确写法
std::isspace(static_cast<unsigned char>(ch));  // ✅
```

### 为什么标准这样设计

`<cctype>` 的函数来自 C 标准库。历史上这些函数的实现是查表（array lookup），传入负数就访问了数组外的内存。C 标准明确写了："参数必须能表示为 `unsigned char` 或等于 `EOF`"。

### 项目中何时用到

解析字符串、trim 空白、配置文件解析等场景。**永远记住：传 `char` 给 `<cctype>` 的任何函数前，先 `static_cast<unsigned char>`。**

---

## 23. Error 模式 详解

> 📋 [速查表 #23](./modern-cpp-cheatsheet.md#23-error-模式)

### 为什么不直接用异常

TinyInfer 的 Scheduler 运行在单线程循环中，一个请求的错误不应该让整个服务崩溃。用 `Result<T, Error>` 模式：

- 错误是**显式的**（函数签名告诉你会失败）
- 错误是**局部的**（一个请求失败不影响其他请求）
- 性能是可预测的（不依赖异常展开栈的零成本抽象）

### 错误码分段

```cpp
enum class ErrorCode : int {
  kOk = 0,
  // 客户端错误 1xx
  // 资源错误 2xx
  // 模型错误 3xx
  // 内部错误 4xx
};
```

分段的好处：`code >= 400` 表示内部 bug，应该记录 Fatal 日志；`code < 200` 是用户错误，只记录 Info。

### 工厂方法 vs 直接构造

```cpp
// 直接构造 — 调用者需要写完整的 ErrorCode
return Error(ErrorCode::kInvalidArgument, "shape mismatch");

// 工厂方法 — 更简洁、更不容易写错
return Error::InvalidArgument("shape mismatch");
```

工厂方法还允许集中修改错误消息格式，比如自动添加文件名和行号。

### 为什么 message 参数用 `std::move`

```cpp
Error(ErrorCode code, std::string message)
    : code_(code), message_(std::move(message)) {}
```

`message` 已经是按值传参（会拷贝一份），然后用 `std::move` 把这份拷贝的所有权转移到成员中，避免了**第二次拷贝**。

如果不用 `std::move`：
```
调用方 → 临时 string（第 1 次拷贝，实参到形参）
形参 → 成员变量 message_（第 2 次拷贝，不用 move 的话）
```

用 `std::move` 后：
```
调用方 → 临时 string（第 1 次拷贝，实参到形参）
形参 → 成员变量 message_（O(1) 移动，指针交换）
```

### `const std::string&` 返回

```cpp
[[nodiscard]] const std::string& message() const noexcept;
```

返回 `const` 引用，调用者不拷贝字符串就能读取错误信息。`const` 防止外部改写内部状态，破坏 Error 对象的不可变性。

---

## 附录：推荐阅读顺序

如果你是第一次接触这些概念，建议按以下顺序学习（难度从低到高）：

```
第1遍（语法）：>
  4 enum class → 5 =default/=delete → 7 explicit → 12 #pragma once → 1 [[nodiscard]]
  → 16 const成员 → 18 namespace → 17 四种cast

第2遍（语义）：>
  2 noexcept → 11 string_view → 3 constexpr → 10 optional → 15 RAII
  → 20 ifstream → 19 deque → 22 isspace

第3遍（深入）：>
  6 move/forward → 9 variant → 8 static_assert+traits → 13 CTAD → 14 chrono
  → 21 五大函数速查 → 23 Error模式
```

---

> 📋 **速查表入口：** [现代 C++ 特性速查表](./modern-cpp-cheatsheet.md) — 快速查阅语法和用法。
