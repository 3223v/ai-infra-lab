# 1. make 是什么

`make` 是一个构建工具。

它根据 `Makefile` 里的规则判断：

```makefile
目标: 依赖
	命令
```

含义是：

> 如果“依赖”比“目标”新，或者“目标”不存在，就执行下面的命令生成目标。

例如：

```makefile
main: main.cpp
	g++ main.cpp -o main
```

运行：

```bash
make
```

会生成：

```bash
./main
```

---

# 2. 常用 make 命令

## 基础命令

```bash
make
```

执行 Makefile 中的第一个目标。

---

```bash
make <target>
```

执行指定目标。

例如：

```bash
make clean
make build
make run
make test
```

---

```bash
make -f <file>
```

指定 Makefile 文件名。

```bash
make -f Makefile.dev
```

---

```bash
make -C <dir>
```

进入某个目录执行 make。

```bash
make -C build
make -C third_party/libfoo
```

等价于：

```bash
cd build && make
```

---

## 并行构建

```bash
make -j
```

自动并行构建，使用尽可能多的任务。

更常用：

```bash
make -j4
make -j8
make -j$(nproc)
```

macOS 上可以用：

```bash
make -j$(sysctl -n hw.ncpu)
```

---

## 强制重新构建

```bash
make -B
```

忽略时间戳，强制重新执行规则。

```bash
make -B
make -B run
```

---

## 只打印命令，不执行

```bash
make -n
```

用于查看 make 准备执行什么。

```bash
make -n
make -n clean
```

---

## 显示详细调试信息

```bash
make --debug
```

常用来排查为什么某个目标没有被重新构建。

也可以：

```bash
make --debug=b
make --debug=v
```

---

## 静默执行

```bash
make -s
```

不打印命令本身，只显示命令输出。

Makefile 中也可以用 `@` 静默单行命令：

```makefile
run:
	@./main
```

---

## 忽略错误继续执行

```bash
make -k
```

某个目标失败后，继续构建其他不依赖它的目标。

Makefile 中单条命令前加 `-` 表示忽略该命令错误：

```makefile
clean:
	-rm -f main
```

---

# 3. Makefile 基本结构

一个典型 Makefile：

```makefile
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2

TARGET := app
SRCS := main.cpp foo.cpp bar.cpp
OBJS := $(SRCS:.cpp=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)
```

运行：

```bash
make
make run
make clean
```

---

# 4. Makefile 核心语法

## 变量

```makefile
CXX := g++
CXXFLAGS := -std=c++17 -Wall -O2
TARGET := app
```

使用变量：

```makefile
$(CXX) $(CXXFLAGS) main.cpp -o $(TARGET)
```

推荐使用 `:=`，表示立即展开，行为更稳定。

---

## 目标、依赖、命令

```makefile
app: main.o foo.o
	g++ main.o foo.o -o app
```

含义：

* 目标：`app`
* 依赖：`main.o foo.o`
* 命令：`g++ main.o foo.o -o app`

注意：**命令前必须是 Tab，不是空格。**

这是 Makefile 最常见的坑。

---

## 自动变量

Makefile 有几个非常常用的自动变量：

```makefile
$@
```

表示目标文件。

```makefile
$^
```

表示所有依赖文件。

```makefile
$<
```

表示第一个依赖文件。

例子：

```makefile
$(TARGET): $(OBJS)
	$(CXX) $^ -o $@
```

等价于：

```bash
g++ main.o foo.o bar.o -o app
```

再看编译 `.cpp -> .o`：

```makefile
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
```

如果目标是：

```bash
main.o
```

依赖是：

```bash
main.cpp
```

那么命令展开后是：

```bash
g++ -std=c++17 -Wall -O2 -c main.cpp -o main.o
```

---

# 5. 伪目标 `.PHONY`

像 `clean`、`run`、`test` 这种目标，并不是真的生成一个文件。

应该写成：

```makefile
.PHONY: clean run test
```

否则如果目录里刚好有一个叫 `clean` 的文件，执行：

```bash
make clean
```

可能什么都不做。

常见写法：

```makefile
.PHONY: all clean run test format

all: app

clean:
	rm -f app *.o

run: app
	./app

test: app
	./app --test
```

---

# 6. 常用项目模板

## 单文件 C++ 项目

```makefile
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2

TARGET := main
SRC := main.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
```

---

## 多文件 C++ 项目

目录结构：

```text
.
├── Makefile
├── main.cpp
├── server.cpp
├── server.h
├── block.cpp
└── block.h
```

Makefile：

```makefile
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2

TARGET := app
SRCS := main.cpp server.cpp block.cpp
OBJS := $(SRCS:.cpp=.o)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)
```

---

## 带 include 和 src 目录的项目

目录结构：

```text
.
├── Makefile
├── include
│   └── server.h
└── src
    ├── main.cpp
    └── server.cpp
```

Makefile：

```makefile
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Iinclude

TARGET := app
SRC_DIR := src
BUILD_DIR := build

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
```

---

# 7. 常用内置函数

## wildcard：匹配文件

```makefile
SRCS := $(wildcard src/*.cpp)
```

匹配所有：

```text
src/*.cpp
```

---

## patsubst：替换模式

```makefile
OBJS := $(patsubst src/%.cpp, build/%.o, $(SRCS))
```

把：

```text
src/main.cpp
src/server.cpp
```

变成：

```text
build/main.o
build/server.o
```

---

## 简写替换

```makefile
OBJS := $(SRCS:.cpp=.o)
```

把：

```text
main.cpp foo.cpp
```

变成：

```text
main.o foo.o
```

---

# 8. 调试 Makefile 的方法

## 打印变量

```makefile
print:
	@echo "SRCS=$(SRCS)"
	@echo "OBJS=$(OBJS)"
	@echo "TARGET=$(TARGET)"
```

运行：

```bash
make print
```

---

## 查看执行计划

```bash
make -n
```

只显示命令，不执行。

---

## 强制重新构建

```bash
make clean
make
```

或者：

```bash
make -B
```

---

## 查看为什么不构建

```bash
make --debug=b
```

---

# 9. 常见错误

## 错误 1：命令前用了空格

错误：

```makefile
run:
    ./app
```

正确：

```makefile
run:
	./app
```

命令前必须是 **Tab**。

---

## 错误 2：没有声明 `.PHONY`

错误：

```makefile
clean:
	rm -f app
```

推荐：

```makefile
.PHONY: clean

clean:
	rm -f app
```

---

## 错误 3：头文件修改后没有重新编译

简单 Makefile 里经常只写：

```makefile
%.o: %.cpp
	$(CXX) -c $< -o $@
```

这只监听 `.cpp`，不会正确追踪 `.h`。

更可靠的方式是让编译器生成依赖文件：

```makefile
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -MMD -MP

DEPS := $(OBJS:.o=.d)

-include $(DEPS)
```

完整片段：

```makefile
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -MMD -MP

TARGET := app
SRCS := main.cpp server.cpp block.cpp
OBJS := $(SRCS:.cpp=.o)
DEPS := $(OBJS:.o=.d)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS) $(DEPS)
```

---

# 10. CMake 项目里怎么用 make

你现在的项目如果是 CMake 项目，流程通常是：

```bash
cmake -B build
cmake --build build
```

如果 CMake 生成的是 Unix Makefiles，那么也可以：

```bash
make -C build
```

或者：

```bash
cd build
make
```

但更推荐用：

```bash
cmake --build build
```

原因是它不依赖底层生成器。将来如果换成 Ninja，也不用改命令。

常用 CMake 构建命令：

```bash
cmake -B build
cmake --build build
cmake --build build -j8
cmake --build build --target clean
cmake --build build --target blockserve-sim
```

如果你想看有哪些 target：

```bash
cmake --build build --target help
```

或者在 Makefile 生成器下：

```bash
make -C build help
```

---

# 11. 推荐你记住的最小命令集

日常够用的是这些：

```bash
make
make run
make clean
make -j8
make -n
make -B
make -C build
make -C build help
```

CMake 项目中更推荐：

```bash
cmake -B build
cmake --build build
cmake --build build -j8
cmake --build build --target help
cmake --build build --target clean
```

---

# 12. 推荐 Makefile 模板

以后小型 C++ 项目可以直接用这个：

```makefile
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -MMD -MP

TARGET := app
SRC_DIR := src
BUILD_DIR := build

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

.PHONY: all run clean print

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

print:
	@echo "SRCS=$(SRCS)"
	@echo "OBJS=$(OBJS)"
	@echo "DEPS=$(DEPS)"
	@echo "TARGET=$(TARGET)"
```

对应目录：

```text
.
├── Makefile
├── src
│   ├── main.cpp
│   ├── server.cpp
│   └── block.cpp
└── include
```

如果你要包含 `include`，把 `CXXFLAGS` 改成：

```makefile
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -MMD -MP -Iinclude
```

---

一句话总结：

```bash
make
```

是按 `Makefile` 规则构建项目；

```bash
cmake --build build
```

是按 CMake 生成出来的构建系统构建项目。

你当前这种 CMake 项目，优先用：

```bash
cmake --build build
cmake --build build --target help
```
