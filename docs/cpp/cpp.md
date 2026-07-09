# const Request* front() const;
## 1. 末尾的 `const`（成员函数后置const）
```cpp
const Request* front() const;
                         ↑
```
### 含义：
这个成员函数是**常量成员函数**，承诺：**调用该函数不会修改当前对象的任何成员变量**。

### 约束规则
1. 函数内部不能修改类的成员变量；
2. 不能调用同类里**没有后置const**的普通成员函数；
3. **只有 `const` 类型的对象，才能调用带后置 `const` 的函数**。

示例对比：
```cpp
// 普通函数：可修改对象，const对象不能调用
Request* front();

// const函数：只读，const对象/普通对象都能调用
const Request* front() const;
```

```cpp
// 举例
const Container c;
c.front(); // 只能调用末尾带const的版本，否则编译报错
```

## 2. 函数返回值前面的 `const`
```cpp
const Request* front() const;
↑
```
表示：返回一个**指向常量 Request 对象的指针**。
- 通过这个指针**不能修改 Request 对象内部数据**；
- 指针本身可以重新赋值（指针变量可改，指向内容不可改）。

### 区分容易混淆的写法
```cpp
// 1. const Request* p：内容不可改，指针可改
const Request* p = c.front();
p->data = 100; // 报错，对象只读
p = nullptr;   // 合法

// 2. Request* const p：指针不可改，内容可改
Request* const p = ...;
p->data = 100; // 合法
p = nullptr;   // 报错

// 3. const Request* const p：指针、内容全都不可改
const Request* const p = ...;
```

## 整行完整总结
```cpp
// 返回指向只读Request的指针，且该函数不会修改容器本身
const Request* front() const;
```
1. 尾部 `const`：函数只读容器，不改动容器；
2. 开头 `const`：拿到的头部元素不允许被外部修改。

## 配套重载（标准容器常见成对实现）
标准库容器一般同时提供两个版本，满足读写需求：
```cpp
// 普通对象调用，可修改首元素
Request* front();

// const对象调用，仅读取首元素，不可修改
const Request* front() const;
```

# namespace

## 1. 外层 `blockserve` + 内层无名 `namespace {}` 是什么？

```
namespace blockserve {
    namespace { // 匿名命名空间（unnamed namespace）
        std::string trim(...);
    }
}
```

### 1.1 `namespace blockserve`

普通具名命名空间，作用：把 `blockserve` 相关代码打包，避免全局名字冲突，外面使用要写 `blockserve::xxx`。

### 1.2 内部嵌套 `namespace {}` 匿名命名空间（重点）

C++ 匿名命名空间规则：
**里面定义的所有变量/函数，仅当前 `.cpp` 文件可见，其他源文件无法访问，等同于文件内私有工具函数。**

作用：

1. `trim` 只是内部工具函数，不需要对外暴露，不用写 `static` 修饰函数，靠匿名空间实现**文件作用域私有化**；
2. 不会污染 `blockserve` 对外接口，外部代码 `blockserve::trim` 找不到这个函数；
3. 相比 C 语言 `static` 函数，匿名命名空间可以包裹一整段工具代码，更适合批量私有工具。

简单说：这个匿名 namespace 是用来**藏内部工具函数，不让外部调用**。

# std::isspace

## 头文件依赖

需要 `#include <cctype>`，标准库字符判断函数。

## 功能

判断一个字符是否为**空白字符**，返回 `int`（非0=是空白，0=不是）。
空白包含：

- 空格 `' '`
- 制表符 `\t`
- 换行 `\n`
- 回车 `\r`
- 垂直制表 `\v`、换页 `\f`

## 原型简化

```
int isspace(int ch);
```

⚠️ 关键限制：传入参数**必须是 `unsigned char` 转成的 int**，或者 `EOF`；
如果直接传负数 char（如中文、扩展ASCII，char 有符号时值为负），会触发未定义行为 UB，程序崩溃/乱判断。
这就是代码里写 `static_cast<unsigned char>` 的根本原因。

## `static_cast<unsigned char>(text[end - 1])` 含义

### 分层拆解

1. `text[end - 1]`：取字符串末尾待判断的字符，类型是 `char`；
2. `static_cast<unsigned char>(...)`：强制类型转换，把有符号 char 转为无符号 unsigned char；
3. 再丢进 `std::isspace()` 保证参数合法，规避负数 UB。

### 为什么必须转？

很多编译器 `char` 是**有符号类型**，字符编码值超过 127 时，char 会变成负数。
`isspace` 参数接收 `int`，如果你直接传负 char，底层查表会访问非法内存，产生未定义行为。
规范写法永远是：

```
std::isspace(static_cast<unsigned char>(c))
```

# static_cast 

## 一、它不是「模板范型」，是**C++ 显式类型转换运算符**

语法格式：

```
static_cast<目标类型>(待转换值)
```

`<>` 里放类型，看起来像模板，但它是编译器内置关键字，不属于 template 泛型。

## 二、static_cast 能做哪些转换

### 1. 基础算术类型转换（数值之间）

整数 ↔ 浮点数、char ↔ int、unsigned ↔ signed

```
char ch = 0xFF;
auto uch = static_cast<unsigned char>(ch);
```

你代码里就是这个用途：把有符号 char 转无符号 unsigned char，避免负数传入 `isspace` 出UB。

### 2. 指针/void\* 互相转换

- 任意指针转 `void*`
- `void*` 转回原类型指针（安全场景）

```
int a = 10;
void* p = &a;
int* pa = static_cast<int*>(p);
```

### 3. 父子类指针向上/向下转换（无运行时检查）

```
class Base {};
class Derived : public Base {};
Derived d;
Base* b = static_cast<Base*>(&d);    // 向上转换，安全
Derived* d2 = static_cast<Derived*>(b); // 向下转换，不做校验，错了UB
```

> 
> 向下转型不安全，需要运行时校验用 `dynamic_cast`。

### 4. 枚举 ↔ 整数转换

```
enum E { A=1 };
int x = static_cast<int>(A);
E e = static_cast<E>(2);
```

### 5. 隐式允许的转换，写成显式更清晰

原本编译器能自动转的，用 static\_cast 明确告诉阅读者这里有类型转换。

## 三、和另外三种cast对比区分

1. **static\_cast**
静态编译期转换，无运行时开销；基础类型、合法父子指针、void\*互转；不做安全检查。
2. **dynamic\_cast**
带运行时RTTI检查，只用于多态父子类向下转型，失败返回nullptr；开销大。
3. **const\_cast**
只用来增删 `const/volatile`，不能改底层类型。
4. **reinterpret\_cast**
粗暴二进制比特重解释，跨类型指针强转（int\* ↔ float\*），极不安全，底层驱动才用。

# std::ifstream

## 1. 头文件依赖

必须包含：

```
#include <fstream>
```

`std::ifstream` = input file stream，**文件输入流**，专门用来从磁盘文件读取内容。

## 2. 语句拆分

```
std::ifstream file(path);
```

- `std::ifstream`：类名，文件读流类型；
- `file`：自定义变量名，代表这个文件流对象；
- `path`：`std::string` 类型，磁盘上文件的路径；
这一行是**构造函数初始化**，等价于两步简写：

```
std::ifstream file;
file.open(path);
```

## 3. 构造时发生了什么

1. 操作系统根据传入路径打开文件；
2. 建立程序与文件之间的读取数据流；
3. 默认打开模式：只读，文本模式（`std::ios::in`）
   - 不能写文件；
   - 会自动处理系统换行符（Windows `\r\n` / Linux `\n`）。

## 4. 怎么判断文件是否打开成功

打开失败常见场景：文件不存在、权限不足、路径写错、是目录不是文件。

```
std::ifstream file(path);
if (!file.is_open()) {
    // 打开失败
}
```

等价写法（流对象重载了bool判断）：

```
if (!file) {
    // 打开失败
}
```

## 5. 常用读取配套函数

### std::getline(file, line)

循环读取文件**一整行**到字符串 `line`，自动丢弃换行符，就是你解析jsonl每行的逻辑：

```
std::string line;
while (std::getline(file, line)) {
    // 处理单行
}
```

循环结束条件：读到文件末尾 EOF。

## 6. 生命周期与自动关闭

`file` 是局部变量，离开作用域（函数结束/循环结束）时，**析构函数自动调用 close()**，不用手动关文件。
手动关闭写法（一般不需要）：

```
file.close();
```

## 7. 打开模式扩展（可选参数）

构造函数第二个参数可指定打开模式，例如二进制读：

```
// 二进制模式读取，不转换换行符
std::ifstream bin_file("data.bin", std::ios::in | std::ios::binary);
```

## 8. 场景结合

```
std::ifstream file(path);
if(!file.is_open()) {
    result.errors.push_back("Failed to open file: " + path);
    return result;
}
```

- path 是传入的负载jsonl文件路径；
- 文件打不开就把错误存入结果，直接返回，不再解析；
- 打开成功后用 `std::getline` 逐行读取每条负载任务。

## 9. 对比区分三个文件流

1. `std::ifstream`：只读文件（现在用的）
2. `std::ofstream`：只写文件
3. `std::fstream`：可读可写