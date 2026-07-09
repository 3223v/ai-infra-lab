# STL
## vector

## sort

## deque双端队列

### 一、deque 基础概念

`deque` = double-ended queue，**双端队列**

1. 头部、尾部都能高效插入/删除（O(1)）
2. 支持随机访问 `[] / at()`，不像 list 只能遍历
3. 底层分段连续内存，不像 vector 整块连续，扩容代价更小
4. 头文件：

```
#include <deque>
using namespace std; // 或 std::deque
```

定义模板：

```
template<class T, class Allocator = allocator<T>> class deque;
```

### 二、创建与初始化

```
// 1. 空队列
deque<int> dq1;

// 2. n个值初始化
deque<int> dq2(5, 10); // 5个10

// 3. 数组/vector拷贝初始化
int arr[] = {1,2,3};
deque<int> dq3(arr, arr+3);

// 4. 拷贝构造
deque<int> dq4(dq3);

// 5. 初始化列表(C++11)
deque<int> dq5{1,3,5,7};
```

### 三、核心常用API分类

#### 1. 两端增删（deque 特色）

```
deque<int> dq{2,4,6};

// 尾部插入
dq.push_back(8);        // 尾插 8
dq.emplace_back(10);    // 原地构造，效率更高

// 头部插入
dq.push_front(0);       // 头插 0
dq.emplace_front(-2);

// 尾部删除
dq.pop_back();

// 头部删除
dq.pop_front();
```

#### 2. 访问元素

```
deque<int> dq{10,20,30,40};

dq[0];          // 下标随机访问，越界不检查
dq.at(1);       // 带越界检查，抛异常
dq.front();     // 获取队首元素
dq.back();      // 获取队尾元素

// const 版本（只读）
const deque<int>& cdq = dq;
cdq.front();
cdq.back();
```

#### 3. 容量相关

```
dq.empty();      // 是否为空，bool
dq.size();       // 当前元素个数
dq.max_size();   // 最大可容纳数量
dq.resize(n);    // 调整大小，多删少补默认0
dq.resize(n, val);
dq.clear();      // 清空所有元素
```

#### 4. 任意位置插入删除（迭代器）

```
deque<int> dq{1,3,5};
auto it = dq.begin() + 1;

dq.insert(it, 2);          // it前插入2
dq.insert(it, 2, 9);       // it前插入2个9
dq.erase(it);              // 删除it指向元素
dq.erase(dq.begin(), dq.end()); // 删除区间
```

#### 5. 迭代器遍历

```
deque<int> dq{1,2,3};

// 正向遍历
for(auto it = dq.begin(); it != dq.end(); ++it)
    cout << *it;

// 反向遍历
for(auto it = dq.rbegin(); it != dq.rend(); ++it)
    cout << *it;

// 范围for(C++11)
for(auto x : dq) cout << x;
```

#### 6. 交换、赋值

```
deque<int> a{1,2}, b{3,4};
a.swap(b);

a.assign(3, 100);       // 3个100覆盖
a.assign(b.begin(), b.end());
```

### 四、完整函数速查表

| 接口 | 作用 |
| --- | --- |
| `push_front(x)` | 头部插入 |
| `emplace_front(x)` | 头部原地构造 |
| `push_back(x)` | 尾部插入 |
| `emplace_back(x)` | 尾部原地构造 |
| `pop_front()` | 删除头部 |
| `pop_back()` | 删除尾部 |
| `front()` | 返回首元素 |
| `back()` | 返回尾元素 |
| `operator[]` | 随机访问，无越界检查 |
| `at(pos)` | 随机访问，越界抛异常 |
| `begin()/end()` | 正向迭代器 |
| `rbegin()/rend()` | 反向迭代器 |
| `empty()` | 判断空 |
| `size()` | 元素数量 |
| `clear()` | 清空 |
| `resize(n)` | 修改容器大小 |
| `insert(iter, val)` | 指定位置插入 |
| `erase(iter)` | 指定位置删除 |
| `swap(other)` | 交换两个deque |