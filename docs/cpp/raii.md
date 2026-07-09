RAII 是 C++ 里非常核心的资源管理思想

**Resource Acquisition Is Initialization**

直译是：**资源获取即初始化**。

更实用的理解是：

> 把资源的申请放在对象构造函数里，把资源的释放放在对象析构函数里。
> 这样对象生命周期结束时，资源会自动释放。

---

## 1. RAII 解决什么问题？

C/C++ 里经常要手动管理资源，比如：

```cpp
int* p = new int(10);

// 中间很多逻辑

delete p;
```

问题是，如果中间出错、提前 return、抛异常，就可能忘记释放：

```cpp
void f() {
    int* p = new int(10);

    if (some_error()) {
        return; // 忘记 delete，内存泄漏
    }

    delete p;
}
```

RAII 的目标就是避免这种问题。

---

## 2. RAII 的核心机制

C++ 对象有确定的生命周期：

```cpp
void f() {
    MyObject obj;
} // 离开作用域时，obj 的析构函数自动调用
```

所以可以把资源绑定到对象上：

```cpp
class IntHolder {
public:
    IntHolder() {
        p = new int(10);
    }

    ~IntHolder() {
        delete p;
    }

private:
    int* p;
};
```

使用时：

```cpp
void f() {
    IntHolder holder;

    if (some_error()) {
        return;
    }
} // holder 析构，自动释放 p
```

即使提前 return，析构函数也会执行。

---

## 3. RAII 管理的不只是内存

很多人误以为 RAII 只是智能指针，其实不是。

RAII 可以管理所有“需要申请和释放”的资源：

| 资源类型   | 获取           | 释放            |
| ------ | ------------ | ------------- |
| 内存     | `new`        | `delete`      |
| 文件     | `open`       | `close`       |
| 锁      | `lock`       | `unlock`      |
| Socket | `socket`     | `close`       |
| 数据库连接  | `connect`    | `disconnect`  |
| GPU 显存 | `cudaMalloc` | `cudaFree`    |
| 线程     | `start`      | `join/detach` |

---

## 4. 最常见 RAII 示例：智能指针

### 不推荐的写法

```cpp
void f() {
    int* p = new int(10);

    // 复杂逻辑

    delete p;
}
```

### RAII 写法

```cpp
#include <memory>

void f() {
    auto p = std::make_unique<int>(10);

    // 不需要 delete
} // 自动释放
```

`std::unique_ptr` 本质就是一个 RAII 类。

它在构造时持有资源，在析构时释放资源。

---

## 5. RAII 管理文件

不推荐：

```cpp
FILE* file = fopen("data.txt", "r");

if (!file) {
    return;
}

// 读取文件

fclose(file);
```

RAII 风格：

```cpp
#include <fstream>

void read_file() {
    std::ifstream file("data.txt");

    if (!file.is_open()) {
        return;
    }

    // 使用 file
} // 自动关闭文件
```

`std::ifstream` 的析构函数会自动关闭文件。

---

## 6. RAII 管理锁

这是 RAII 非常经典的应用。

不推荐：

```cpp
mutex.lock();

// 临界区

mutex.unlock();
```

如果中间 return 或抛异常，就会死锁。

RAII 写法：

```cpp
#include <mutex>

std::mutex m;

void f() {
    std::lock_guard<std::mutex> lock(m);

    // 临界区
} // 自动 unlock
```

`std::lock_guard` 构造时加锁，析构时解锁。

---

## 7. 自己写一个 RAII 类

例如管理一个 C 风格文件：

```cpp
#include <cstdio>
#include <stdexcept>

class File {
public:
    explicit File(const char* path) {
        file_ = std::fopen(path, "r");
        if (!file_) {
            throw std::runtime_error("failed to open file");
        }
    }

    ~File() {
        if (file_) {
            std::fclose(file_);
        }
    }

    FILE* get() const {
        return file_;
    }

private:
    FILE* file_;
};
```

使用：

```cpp
void f() {
    File file("data.txt");

    // 使用 file.get()
} // 自动 fclose
```

---

## 8. RAII 类通常要禁止拷贝

上面的 `File` 类有一个隐藏问题。

如果允许拷贝：

```cpp
File a("data.txt");
File b = a;
```

那么 `a` 和 `b` 都持有同一个 `FILE*`。

离开作用域时会发生两次 `fclose`，这是严重错误。

所以 RAII 类通常要这样写：

```cpp
class File {
public:
    explicit File(const char* path) {
        file_ = std::fopen(path, "r");
        if (!file_) {
            throw std::runtime_error("failed to open file");
        }
    }

    ~File() {
        if (file_) {
            std::fclose(file_);
        }
    }

    File(const File&) = delete;
    File& operator=(const File&) = delete;

private:
    FILE* file_;
};
```

这就是所谓的：

```cpp
File(const File&) = delete;
File& operator=(const File&) = delete;
```

意思是：禁止拷贝。

---

## 9. 支持移动语义的 RAII 类

有时候资源不能拷贝，但可以转移所有权。

例如：

```cpp
File a("data.txt");
File b = std::move(a);
```

这表示 `b` 接管文件，`a` 不再拥有文件。

完整写法：

```cpp
#include <cstdio>
#include <stdexcept>
#include <utility>

class File {
public:
    explicit File(const char* path) {
        file_ = std::fopen(path, "r");
        if (!file_) {
            throw std::runtime_error("failed to open file");
        }
    }

    ~File() {
        if (file_) {
            std::fclose(file_);
        }
    }

    File(const File&) = delete;
    File& operator=(const File&) = delete;

    File(File&& other) noexcept {
        file_ = other.file_;
        other.file_ = nullptr;
    }

    File& operator=(File&& other) noexcept {
        if (this != &other) {
            if (file_) {
                std::fclose(file_);
            }

            file_ = other.file_;
            other.file_ = nullptr;
        }

        return *this;
    }

    FILE* get() const {
        return file_;
    }

private:
    FILE* file_ = nullptr;
};
```

重点是移动构造函数：

```cpp
File(File&& other) noexcept {
    file_ = other.file_;
    other.file_ = nullptr;
}
```

它把资源从 `other` 转移到当前对象，然后把 `other.file_` 置空，避免重复释放。

---

## 10. RAII 和异常安全

RAII 的一个重要价值是：**异常安全**。

例如：

```cpp
void f() {
    std::lock_guard<std::mutex> lock(m);

    do_something(); // 这里可能抛异常
} // 即使抛异常，也会自动 unlock
```

只要对象已经构造成功，离开作用域时析构函数就会执行。

这让 C++ 代码可以不用到处写：

```cpp
try {
    ...
} catch (...) {
    cleanup();
    throw;
}
```

---

## 11. RAII 的本质规则

写 C++ 时可以记住这几条：

1. **谁申请资源，谁负责释放资源。**
2. **资源应该被对象拥有，而不是裸指针拥有。**
3. **构造函数获取资源。**
4. **析构函数释放资源。**
5. **有所有权的对象通常禁止拷贝。**
6. **需要转移所有权时，实现移动语义。**
7. **优先使用标准库 RAII 类型。**

---

## 12. 常见标准库 RAII 类型

你日常 C++ 里会经常遇到这些：

```cpp
std::unique_ptr
std::shared_ptr
std::vector
std::string
std::fstream
std::ifstream
std::ofstream
std::lock_guard
std::unique_lock
std::jthread
```

比如：

```cpp
std::vector<int> v;
```

不需要手动申请数组、不需要手动释放数组，因为 `std::vector` 已经用 RAII 管理了内部内存。

---

## 13. 实际意义
比如：

### 管理文件

```cpp
std::ifstream input(path);
```

### 管理内存

```cpp
std::vector<TokenBlock> blocks;
std::unique_ptr<Model> model;
```

### 管理锁

```cpp
std::lock_guard<std::mutex> guard(mutex_);
```

### 管理线程

```cpp
std::jthread worker([&] {
    run_loop();
});
```

### 管理 socket

可以自己封装：

```cpp
class Socket {
public:
    explicit Socket(int fd) : fd_(fd) {}

    ~Socket() {
        if (fd_ >= 0) {
            close(fd_);
        }
    }

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

private:
    int fd_;
};
```

---

## 14. 总结

RAII 就是：

> 用 C++ 对象的生命周期管理资源生命周期。
> 不要让资源裸奔。
> 把资源塞进类里，让构造函数负责拿资源，让析构函数负责还资源。

这是现代 C++ 必须掌握的基础。