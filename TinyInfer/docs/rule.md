# TinyInfer C++ 编码与命名规范
> 适配现代 C++17，遵循 Google C++ Style 衍生规范，对齐现有源码风格，可直接作为项目 `CODING_GUIDELINES.md`
> 适用：推理引擎、Runtime、底层高性能组件开发

```markdown
# TinyInfer C++ 编码规范

## 基础信息
- C++ 标准：最低 C++17
- 目标：高性能底层推理框架，杜绝未定义行为、资源生命周期隐患
- 核心原则：**一致性优先，可读性优先，安全优先**

> 所有新增代码必须遵守本规范；存量代码逐步改造对齐。

# 一、命名规范
术语约定
- PascalCase：大驼峰 `RequestContext`
- camelCase：小驼峰 `requestContext`
- snake_case：蛇形命名 `request_id`

## 1.1 类型名称（class / struct / enum / enum class / using 别名）
规则：**PascalCase**
```cpp
class InferenceSlot;
struct RequestParam;
enum class ErrorCode;
using Generation = uint64_t;
using SlotCallback = std::function<void(Slot&)>;
```

## 1.2 成员变量（类内私有/保护成员）
规则：**snake_case，尾部以下划线 `_` 结尾**
作用：快速区分「成员变量」与「局部变量/函数参数」
```cpp
private:
  uint64_t generation_;
  ErrorCode last_error_;
  bool is_cancelled_;
```

> 布尔变量建议前缀 `is_ / has_ / enable_`，语义清晰。

## 1.3 局部变量 / 函数入参
规则：**snake_case，无尾下划线**
```cpp
Error SubmitRequest(uint64_t slot_id);

void Func() {
  size_t batch_size = 1;
  auto request_ptr = CreateRequest();
}
```

## 1.4 函数（成员函数 / 全局自由函数）
规则：**PascalCase**
```cpp
[[nodiscard]] ErrorCode GetLastError() const noexcept;
Error SubmitRequest();
void CancelRequest();
```

## 1.5 常量
### 1.5.1 命名空间域 `inline constexpr` 常量
格式：前缀小写 `k` + PascalCase
```cpp
inline constexpr Generation kInvalidGeneration = 0;
inline constexpr size_t kMaxBatchSize = 32;
```

### 1.5.2 enum class 枚举项
与全局常量风格保持一致：`kXxx`
```cpp
enum class ErrorCode {
  kOk = 0,
  kInvalidArgument,
  kQueueFull
};
```

## 1.6 禁止写法（硬性红线）
1. 大小写与下划线混搭：`Request_ID`、`kInvalid_Generation`
2. 小驼峰成员变量：`requestId_`
3. 宏与现代常量混用命名风格
4. 匈牙利命名法（`pBuffer`、`uLen`）
5. 仅依靠大小写区分实体：`Generation generation`

## 1.7 缩写约定
通用缩写统一写法：
- ID → `RequestId`（统一，不要 `RequestID`）
- URL、CPU、GPU 大写保留
自定义缩写团队统一，文档备注，禁止随意简写。

# 二、头文件规范
1. 统一使用 `#pragma once` 头文件保护，不再使用 `#ifndef` 守卫
2. 头文件尽量**前向声明**，减少 `#include`，降低编译依赖
3. 头文件禁止放非 `inline` 的全局变量，防止多重定义
4. 命名空间域常量统一使用 `inline constexpr`（C++17）

```cpp
// 正确
inline constexpr uint64_t kInvalidGeneration = 0;

// 禁止（C++17下多翻译单元链接冲突）
constexpr uint64_t kInvalidGeneration = 0;
```

# 三、类设计与对象语义规范
## 3.1 可拷贝/可移动控制
资源持有类（Tensor、Context、Slot、Handle）严格控制对象语义：
- 禁止拷贝：继承 `private NonCopyable`
- 禁止拷贝且禁止移动：继承 `private NonCopyableNonMovable`
```cpp
struct InferenceContext : private NonCopyable {
  // 可自行实现移动构造/移动赋值开启所有权转移
  InferenceContext(InferenceContext&&) noexcept;
  InferenceContext& operator=(InferenceContext&&) noexcept;
};
```

## 3.2 特殊成员函数显式声明原则
需要禁用拷贝/移动：使用 `= delete`
需要恢复编译器默认实现：使用 `= default`
优先编译期报错，而非老式私有声明不定义。

## 3.3 访问控制顺序
```cpp
class Demo {
 public:
  // 公开接口
 protected:
  // 保护成员
 private:
  // 私有成员变量、内部工具函数
};
```

# 四、现代 C++ 语法使用规范
## 4.1 属性与修饰符顺序
推荐顺序：`inline constexpr`，不推荐 `constexpr inline`
```cpp
inline constexpr size_t kMaxSlotNum = 256;
```

## 4.2 `[[nodiscard]]`
以下函数**强制添加**：
- 返回 `Error`、错误码、状态结果
- 工厂函数、资源创建函数
禁止调用方忽略返回值，防止错误静默丢失。
```cpp
[[nodiscard]] Error SubmitTask();
[[nodiscard]] ErrorCode GetStatus() const noexcept;
```

## 4.3 `noexcept`
满足条件时添加：
- 不抛异常的查询类 const 成员函数
- 移动构造、移动赋值
作用：帮助容器优化、清晰标识异常边界。

## 4.4 const 成员函数
不修改任何成员状态的成员函数，末尾必须加 `const`
```cpp
[[nodiscard]] const std::string& Message() const noexcept {
  return message_;
}
```

返回内部资源引用优先 `const T&`，避免不必要拷贝。

## 4.5 std::string_view
只读接收字符串参数优先使用 `std::string_view`，减少内存拷贝。

# 五、内存与资源管理规范
1. 优先 `std::unique_ptr`（独占所有权），谨慎使用 `std::shared_ptr`
2. 裸指针只用于**非所有权观察**；持有资源禁止裸指针
3. RAII 范式管理显存、句柄、文件、锁等资源
4. 尽量避免手动 new/delete，优先容器与智能指针

# 六、函数编码规范
1. 函数功能单一，禁止超长函数
2. 入参顺序：输入参数在前，输出参数在后
3. 优先值语义 + 移动，避免大量深拷贝
4. 不使用输出参数传递正常结果；错误使用 Error 返回

# 七、注释规范
1. 头文件对外接口使用块注释说明功能、返回错误、前置条件
2. 复杂算法、调度逻辑、世代（Generation）机制添加注释
3. 不要复述代码显而易见的行为；解释「为什么这么设计」
4. 重大优化、临时 workaround 标注原因，预留清理时机

# 八、格式基础约定（可配合 clang-format）
1. 缩进：4空格，不使用 Tab
2. 大括号 Allman 风格：单独换行
```cpp
if (!err.ok())
{
    HandleError(err);
}
```
3. 二元运算符两侧保留空格
4. 命名空间内部不额外缩进

# 九、禁止清单
1. 禁止裸 `#define` 定义常量，优先 `inline constexpr`
2. 禁止老式无作用域 `enum`，新项目一律 `enum class`
3. 禁止内存泄漏风险的浅拷贝设计
4. 禁止忽略 Error/ErrorCode 返回值（配合 [[nodiscard]] 强制约束）
5. 禁止跨线程裸访问成员变量，无同步保护
6. 禁止在析构函数抛出异常
```

## 使用说明
1. 直接复制保存为 `CODING_GUIDELINES.md` 放入项目根目录；
2. 搭配一份 `.clang-format` 格式化文件，自动对齐空格、缩进；
3. 如果你需要，我可以配套输出一份适配这套规范的 **clang-format 配置文件**。