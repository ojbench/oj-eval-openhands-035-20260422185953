# Problem 035 - skywong的string

**ACMOJ Problem ID**: 2493

## Table of Contents

- [Problem 035 - skywong的string](#problem-035-skywong的string)
  - [Table of Contents](#table-of-contents)
  - [Introduction](#introduction)
    - [Background](#background)
  - [Assignment Description](#assignment-description)
    - [Grade Composition](#grade-composition)
  - [Assignment Requirements](#assignment-requirements)
    - [Input Format](#input-format)
    - [Output Format](#output-format)
    - [Samples](#samples)
    - [Data Constraints](#data-constraints)
  - [Per-Testcase Resource Limits](#per-testcase-resource-limits)
  - [Test Data](#test-data)
  - [Submission Requirements](#submission-requirements)
    - [OJ Git Repository Compilation Process](#oj-git-repository-compilation-process)
    - [Git Configuration Requirements](#git-configuration-requirements)
    - [Submission Guidelines](#submission-guidelines)
    - [Evaluation Notes](#evaluation-notes)
    - [Academic Integrity](#academic-integrity)

## Introduction

### Background

Implement custom string class with SSO, capacity management, COW. Includes append, substr, find, replace.

## Assignment Description

### Grade Composition

| Grading Component | Percentage |
| :--: | :--: |
| Pass **2493. skywong的string** | 80% |
| Code Review | 20% |

Here are several points that need clarification:

- In the Code Review, we will **strictly examine your code style and repository organization structure, etc.**

- This assignment provides some sample data for testing, stored in the `/workspace/data/035/data_test/` directory. Note that these are not the test cases on the Online Judge. Passing all local test cases does not guarantee that you will pass the OJ tests.

- Besides the provided sample data, we also encourage you to design your own test data based on your program logic to assist debugging.

## Assignment Requirements

### Problem Description

`std::string`是cpp中一个非常好用的字母串类，于是skywong想手写一个类似`std::string`（但简化很多）的MyString类来作为练习。但建完仓库的当天skywong突然发现《Leon》重映了，所以他丢给你一份代码模板，剩下的部分交给你来完成。

我们很容易构思出一个很trival的string类实现：根据要存储的字符串的长度，分配恰到好处的空间，然后将字符串逐个字符存储在这个空间中。
但这样的实现问题也很明晰那：append操作时，总需要重新分配空间，然后将原有的字符串复制到新的空间中，导致操作效率很低。 除此之外，也有很多其他的问题，比如：如何处理字符串的拷贝等。

为了更高的效率，还添加了一些小优化，希望你能实现。

- `小对象优化 (SSO)`：如果字符串长度小于等于15，直接将字符串存储在对象内部，不再分配额外的空间。**在本题中，如果字符串长度（不算入'\0'）小于等于15，则启用SSO，直接存储在对象内部。**
- `容量管理 (Capacity Management)`：为了避免频繁的内存重新分配，会维护一个 capacity（容量）值，这个值比实际存储的字符数要大。当字符串长度增加超过容量时，会分配一个更大的内存块（通常是当前容量乘以 1.5 或 2），并将旧的数据复制过去。
- `空间优化 (Space Optimization)`：与上一条类似，当字符串长度减少到一定程度时，会释放一部分内存，减少内存占用。
- `移动语义 (Move Semantics)`：在c++11引入了移动语义后，我们可以通过移动构造函数和移动赋值运算符来避免不必要的内存拷贝。（如果你不了解该功能，非常建议查询和学习相关资料）

此外，skywong还希望你实现以下接口：

- `构造函数`：支持从MyString对象（拷贝和移动），char*构造MyString对象。（默认构造函数为构造一个空字符串）
- 连接（`operator+`）：将两个字符串连接起来
- 对字符的直接操作（`operator[]`, `at`）：可以通过下标访问字符串中的字符
- `size`：返回存储的字符串的长度
- `capacity`：返回当前分配的空间最多可以存储的字符数（不包括'\0'终止符）。**在本题中，因为SSO优化的缘故，如果你的size小于等于15，capacity也应该返回15。**
- `reserve`：分别用于预分配空间。（需要注意的是，用reserve提前开辟的空间不一定要准确，MyString实际开辟的空间可以比输入的空间略大）
- `resize`：分别用于调整字符串的长度，如果新的长度大于原来的长度，那么新的字符应该用'\0'填充，如果新的长度小于原来的长度，那么多余的字符应该被丢弃。

这个MyString类还应有一个简单版的迭代器，支持一些基本操作：

- `begin` 和 `end`：分别返回指向第一个字符和最后一个字符之后的位置的迭代器
- `cbegin` 和 `cend`：分别返回指向第一个字符和最后一个字符之后的位置的常量迭代器
- `operator++` 和 `operator--`：分别将迭代器向前和向后移动一个位置
- `operator==` 和 `operator!=`：判断两个迭代器是否相等（注意：不是判断迭代器指向的字符是否相等）
- `operator*`：返回迭代器指向的元素
  注：在涉及迭代器操作时，**保证每个操作都是合法的**。

未详细说明的部分，可以参考代码模板中的注释。

下面是代码模版，你只需要实现`MyString`类的`TODO`部分。
```cpp
#ifndef SIMPLE_STRING_SIMPLESTRING_HPP
#define SIMPLE_STRING_SIMPLESTRING_HPP

#include <stdexcept> // you can use std::out_of_range
#include <cstring>
// other headers are not allowed

class MyString {
private:
    /**
     * @brief To implement Small String Optimization (SSO), two storage methods are needed.
     *
     * This code demonstrates a possible approach:
     * - A union is used to switch between a small buffer and a heap pointer.
     * - `heap_ptr` is used for larger strings that exceed the buffer capacity.
     * - `small_buffer` is an inline buffer for small strings, enhancing performance by avoiding heap allocation.
     */
    union {
        char* heap_ptr;        // Pointer to heap-allocated memory
        char small_buffer[16]; // Buffer for small string optimization (SSO)
    };
    // TODO: Add other data members here (for length, capacity, etc)
public:
    MyString() {
        // TODO: Implement the default constructor
    }

    /**
     * @brief Constructor from a C-string.
     * @param s Null-terminated character array.
     * @todo Initialize `MyString` with the contents of `s`. Implement SSO if length is below 16.
     */
    MyString(const char* s) {
        // TODO
    }

    MyString(const MyString& other) {
        // TODO: Implement the copy constructor
    }

    MyString(MyString&& other) noexcept {
        // TODO: Implement the move constructor
    }

    MyString& operator=(MyString&& other) noexcept {
        // TODO: Implement the move assignment operator
    }

    MyString& operator=(const MyString& other) {
        // TODO: Implement the copy assignment operator
    }

    ~MyString() {
        // TODO
    }

    const char* c_str() const {
        // TODO: Returns a C-string equivalent of the `MyString`.
    }

    size_t size() const {
        // TODO: Returns the number of characters in the string.
    }

    size_t capacity() const {
        // TODO: Returns the storage capacity of the string.
        // ATTENTION: Capacity excluding null terminator.
    }

    void reserve(size_t new_capacity) {
        // TODO: Reserve storage for the string.
        // ATTENTION: Capacity excluding null terminator.
    }

    void resize(size_t new_size) {
        // TODO: Resize the storage for the string.
        // after this operation, the value of size() should be new_size
    }

    /**
     * @brief Accesses a character by index.
     * @param index The position of the character.
     * @return Reference to the character at `index`.
     * @throw std::out_of_range if `index` is out of bounds.
     */
    char& operator[](size_t index) {
        // TODO
    }

    /**
     * @brief Concatenates two `MyString` instances.
     * @param rhs The `MyString` to concatenate.
     * @return A new `MyString` containing the concatenated result.
     */
    MyString operator+(const MyString& rhs) const {
        // TODO
    }

    /**
     * @brief Appends a C-string to the current `MyString`.
     * @param str Null-terminated character array.
     */
    void append(const char* str) {
        // TODO
    }

    /**
     * @brief Accesses a character by index safely.
     * @param pos The position of the character.
     * @throw std::out_of_range if `pos` is out of bounds.
     */
    const char& at(size_t pos) const {
        // TODO
    }

    class const_iterator;

    class iterator {
    private:
        // TODO: add data members, just add whatever you want.
    public:
        /**
         * @brief Pre-increment iterator.
         * @return Reference to the incremented iterator.
         */
        iterator& operator++() {
            // TODO: Implement pre-increment
        }

        /**
         * @brief Post-increment iterator.
         * @return Copy of iterator before increment.
         */
        iterator operator++(int) {
            // TODO: Implement post-increment
        }

        /**
         * @brief Pre-decrement iterator.
         * @return Reference to the decremented iterator.
         */
        iterator& operator--() {
            // TODO: Implement pre-decrement (move iterator one step back)
        }

        /**
         * @brief Post-decrement iterator.
         * @return Copy of iterator before decrement.
         */
        iterator operator--(int) {
            // TODO: Implement post-decrement
        }

        char& operator*() const {
            // TODO: Dereference operator. (return character at the iterator position)
        }

        bool operator==(const iterator& other) const {
            // TODO: Equality comparison operator.
        }

        bool operator!=(const iterator& other) const {
            // TODO: Inequality comparison operator.
        }

        bool operator==(const const_iterator& other) const {
            // TODO: Equality comparison operator.
        }

        bool operator!=(const const_iterator& other) const {
            // TODO: Inequality comparison operator.
        }
    };

    class const_iterator {
        // TODO: similar to iterator, but for const access, implement the same methods as in iterator
    };

public:
    iterator begin() {
        // TODO: Returns an iterator to the beginning.
    }

    iterator end() {
        // TODO: Returns an iterator to the end.(one past the last character.)
    }

    const_iterator cbegin() const {
        // TODO
    }

    const_iterator cend() const {
        // TODO
    }
};

#endif
```

#### 如果有疑问或有bug反馈，请随时联系助教。

### Input Format

See the problem description above.

### Output Format

See the problem description above.

### Samples

No sample data provided for this problem.

### Data Constraints

See the problem description for constraints.

## Per-Testcase Resource Limits

- **Time Limit (per test case)**: 3000 ms
- **Memory Limit (per test case)**: 256 MiB
- **Disk Usage**: No disk usage is permitted.

## Test Data

The test data for this problem is located at `/workspace/data/035/data_test/`.

## Submission Requirements

### OJ Git Repository Compilation Process

For Git compilation, we will first clone the repository using a command similar to:
```bash
git clone <repo_url> . --depth 1 --recurse-submodules --shallow-submodules --no-local
```

Then we check if there is a `CMakeLists.txt` file. If it exists, we run (if not, a warning message will be displayed):
```bash
cmake .
```

Finally, we check if there is any of `GNUmakefile`/`makefile`/`Makefile` (if cmake was run previously, this will be the generated Makefile). If it exists, we run (if not, a warning message will be displayed):
```bash
make
```

After this process is complete, we will use the `code` file in the project root directory as the compilation result.

A `CMakeLists.txt` file is provided in the project. You can use or modify it as needed. The local environment has gcc-13 and g++-13 available.

### Git Configuration Requirements

**IMPORTANT**: You must create a `.gitignore` file in your project root directory to avoid OJ evaluation conflicts.

The `.gitignore` file should include at least the following entries:

```gitignore
CMakeFiles/
CMakeCache.txt
```

### Submission Guidelines

- The submitted code must be able to compile successfully through the above compilation process
- The compiled executable file name must be `code`
- The program needs to be able to read data from standard input and write results to standard output
- Please ensure the code runs correctly within the given time and space limits
- **You must use C++ or C language** to implement this assignment

### Evaluation Notes

- The evaluation system will test your program using the provided test data
- The program output must exactly match the expected output (including format)
- Exceeding time or memory limits will be judged as the corresponding error type
- Please pay attention to the overall time performance of your code and the time complexity of each part of your algorithm.

### Academic Integrity

If any violations are found during evaluation or code review (including but not limited to using unconventional methods to pass test cases), your final score may be significantly reduced or become **0 points**.
