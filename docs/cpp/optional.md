
# std::optional

## 1. 是什么

头文件：`<optional>`，C++17 引入
`std::optional<T>` 是**可选值模板容器**，用来表达「**可能有值，也可能没有值**」的场景。

解决两个传统痛点：

1. 用特殊值（-1、nullptr、空字符串）代表“无数据”，容易混淆；
2. 函数返回指针代表失败，存在内存泄漏、空指针风险。

`optional<T>` 内部只会存两样之一：

- 一个 `T` 类型有效值；
- 无任何值（空状态 `nullopt`）。

## 2. 核心底层特点

1. 不分配堆内存，全部栈上存储；
2. 不管理动态内存，不存在内存泄漏；
3. 有值时直接存放完整 `T` 对象，不是指针；
4. 可拷贝、可移动（取决于T是否可拷贝/移动）。

## 3. 基础用法示例

### 3.1 构造与赋值

```
#include <optional>
#include <iostream>
#include <string>
using namespace std;

// 1. 空 optional
optional<int> opt1;
optional<int> opt2 = nullopt;

// 2. 直接存入有效值
optional<int> opt3(100);
optional<string> opt4{"hello"};

// 3. 原地构造（高效，避免拷贝）
optional<pair<int, int>> opt5{in_place, 1, 2};

// 赋值修改
opt3 = 200;
opt3 = nullopt; // 清空，变为无值
```

### 3.2 判断是否存在值

```
optional<int> opt = 5;

// 方式1：if 判断（最常用）
if (opt) {
    // 有值
}

// 方式2：has_value()
if (opt.has_value()) {
}

// 判断空
if (!opt || !opt.has_value())
```

### 3.3 获取内部值

```
optional<int> opt = 99;

// 1. value() 有值才可用，无值会抛 std::bad_optional_access 异常
int a = opt.value();

// 2. 解引用 * ，无值UB（未定义行为，自己保证一定有值再用）
int b = *opt;

// 3. value_or(默认值)：无值返回默认，最安全推荐
int c = opt.value_or(0);
```

## 4. 常用成员API

| 接口 | 作用 |
| --- | --- |
| `has_value()` | bool，是否持有有效值 |
| `operator bool()` | 等价 `has_value()` |
| `value()` | 返回内部T，空则抛异常 |
| `operator*()` | 返回T引用，空直接UB |
| `operator->()` | 访问内部对象成员，如 opt->size() |
| `value_or(T default)` | 有值返回自身，无值返回默认 |
| `reset()` | 清空，变为空（等价 = nullopt） |
| `emplace(参数...)` | 原地构造T，替换原有值 |

### -> 用法举例

```
optional<string> s = "test";
cout << s->size(); // 等价 (*s).size()
```

## 5. 典型使用场景：函数返回值

### 场景1：查找函数，可能找不到

```
// 找到返回对应int，找不到返回空optional
optional<int> find_val(vector<int>& vec, int target) {
    for (int x : vec) {
        if (x == target) return x;
    }
    return nullopt;
}

// 调用
auto res = find_val({1,2,3}, 5);
if (res) {
    cout << *res;
} else {
    cout << "未找到";
}
// 给默认值写法
int val = res.value_or(-1);
```

### 场景2：替代指针做失败返回（无内存风险）

以前写法（危险）：

```
// 容易空指针、堆内存泄漏
int* func() {
    if (ok) return new int(10);
    return nullptr;
}
```

optional 安全写法：

```
optional<int> func() {
    if (ok) return 10;
    return nullopt;
}
```

## 6. 带 const 重载（结合你之前问的const成员函数）

`optional` 的取值接口同样分常量/非常量版本：

```
T& operator*();
const T& operator*() const;

T& value();
const T& value() const;
```

如果是 `const optional<T>`，只能调用 const 版本，拿到只读引用，不能修改内部对象。

## 7. 禁止误区

1. ❌ 不要在无值时调用 `*opt` / `opt.value()`
   - `*opt`：直接未定义行为，程序崩溃
   - `value()`：抛出异常
2. ❌ optional 不是指针，存的是完整T对象，不是地址
3. ❌ 不能存引用 `optional<int&>`，标准不允许

## 8. 和 std::variant / std::any 简单区分

- `optional<T>`：二选一，有T / 无值
- `variant<A,B,C>`：多种类型中存一个
- `any`：任意类型，但运行时开销大
