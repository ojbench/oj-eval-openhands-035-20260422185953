#ifndef SIMPLE_STRING_SIMPLESTRING_HPP
#define SIMPLE_STRING_SIMPLESTRING_HPP

#include <stdexcept>
#include <cstring>

class MyString {
private:
    union {
        char* heap_ptr;
        char small_buffer[16];
    };
    // cap_ == 0 means SSO (using small_buffer). Otherwise using heap_ptr with given capacity
    size_t len_{};
    size_t cap_{}; // capacity excluding null terminator when heap is used; 0 denotes SSO

    bool is_sso() const { return cap_ == 0; }
    char* data_ptr() { return is_sso() ? const_cast<char*>(small_buffer) : heap_ptr; }
    const char* data_ptr() const { return is_sso() ? small_buffer : heap_ptr; }

    static size_t grow_capacity(size_t cur_cap, size_t min_needed) {
        size_t new_cap = cur_cap ? cur_cap : 15; // treat SSO cap as 15
        while (new_cap < min_needed) {
            new_cap = new_cap + (new_cap >> 1); // *1.5 growth
            if (new_cap < 16) new_cap = 16; // ensure reasonable jump when leaving SSO
        }
        return new_cap;
    }

    void switch_to_sso_if_possible() {
        if (!is_sso() && len_ <= 15) {
            // move data back to small buffer
            std::memcpy(small_buffer, heap_ptr, len_);
            small_buffer[len_] = '\0';
            delete[] heap_ptr;
            heap_ptr = nullptr;
            cap_ = 0; // now in SSO mode
        }
    }

public:
    MyString() : len_(0), cap_(0) { small_buffer[0] = '\0'; }

    MyString(const char* s) : len_(0), cap_(0) {
        if (!s) {
            small_buffer[0] = '\0';
            return;
        }
        size_t n = std::strlen(s);
        if (n <= 15) {
            std::memcpy(small_buffer, s, n);
            small_buffer[n] = '\0';
            len_ = n;
            cap_ = 0; // SSO
        } else {
            len_ = n;
            size_t new_cap = n; // exact fit is fine
            heap_ptr = new char[new_cap + 1];
            std::memcpy(heap_ptr, s, n);
            heap_ptr[n] = '\0';
            cap_ = new_cap;
        }
    }

    MyString(const MyString& other) : len_(other.len_), cap_(other.is_sso() ? 0 : other.cap_) {
        if (other.is_sso()) {
            std::memcpy(small_buffer, other.small_buffer, other.len_);
            small_buffer[len_] = '\0';
        } else {
            heap_ptr = new char[cap_ + 1];
            std::memcpy(heap_ptr, other.heap_ptr, len_);
            heap_ptr[len_] = '\0';
        }
    }

    MyString(MyString&& other) noexcept : len_(other.len_), cap_(other.cap_) {
        if (other.is_sso()) {
            std::memcpy(small_buffer, other.small_buffer, other.len_);
            small_buffer[len_] = '\0';
        } else {
            heap_ptr = other.heap_ptr;
            // leave other empty
            other.heap_ptr = nullptr;
        }
        other.len_ = 0;
        other.cap_ = 0;
        other.small_buffer[0] = '\0';
    }

    MyString& operator=(MyString&& other) noexcept {
        if (this == &other) return *this;
        if (!is_sso() && heap_ptr) {
            delete[] heap_ptr;
            heap_ptr = nullptr;
        }
        len_ = other.len_;
        cap_ = other.cap_;
        if (other.is_sso()) {
            std::memcpy(small_buffer, other.small_buffer, other.len_);
            small_buffer[len_] = '\0';
        } else {
            heap_ptr = other.heap_ptr;
            other.heap_ptr = nullptr;
        }
        other.len_ = 0;
        other.cap_ = 0;
        other.small_buffer[0] = '\0';
        return *this;
    }

    MyString& operator=(const MyString& other) {
        if (this == &other) return *this;
        if (other.is_sso()) {
            if (!is_sso() && heap_ptr) {
                delete[] heap_ptr;
                heap_ptr = nullptr;
            }
            len_ = other.len_;
            cap_ = 0;
            std::memcpy(small_buffer, other.small_buffer, other.len_);
            small_buffer[len_] = '\0';
        } else {
            // ensure capacity
            if (is_sso() || cap_ < other.len_) {
                if (!is_sso() && heap_ptr) delete[] heap_ptr;
                cap_ = other.cap_ >= other.len_ ? other.cap_ : other.len_;
                heap_ptr = new char[cap_ + 1];
            }
            len_ = other.len_;
            std::memcpy(heap_ptr, other.heap_ptr, len_);
            heap_ptr[len_] = '\0';
        }
        return *this;
    }

    ~MyString() {
        if (!is_sso() && heap_ptr) {
            delete[] heap_ptr;
            heap_ptr = nullptr;
        }
    }

    const char* c_str() const { return data_ptr(); }

    size_t size() const { return len_; }

    size_t capacity() const { return is_sso() ? 15 : cap_; }

    void reserve(size_t new_capacity) {
        if (new_capacity <= capacity()) return;
        // move to heap with at least new_capacity capacity
        size_t target = new_capacity;
        if (target < len_) target = len_;
        // allow slight over-allocation
        size_t chosen = grow_capacity(is_sso() ? 15 : cap_, target);
        if (is_sso()) {
            char* new_mem = new char[chosen + 1];
            if (len_) std::memcpy(new_mem, small_buffer, len_);
            new_mem[len_] = '\0';
            heap_ptr = new_mem;
            cap_ = chosen;
        } else {
            char* new_mem = new char[chosen + 1];
            if (len_) std::memcpy(new_mem, heap_ptr, len_);
            new_mem[len_] = '\0';
            delete[] heap_ptr;
            heap_ptr = new_mem;
            cap_ = chosen;
        }
    }

    void resize(size_t new_size) {
        if (new_size <= 15) {
            // can use SSO
            if (!is_sso()) {
                // moving from heap to sso
                size_t copy_n = new_size < len_ ? new_size : len_;
                std::memcpy(small_buffer, heap_ptr, copy_n);
                if (copy_n < new_size) {
                    std::memset(small_buffer + copy_n, '\0', new_size - copy_n);
                }
                small_buffer[new_size] = '\0';
                delete[] heap_ptr;
                heap_ptr = nullptr;
                cap_ = 0;
                len_ = new_size;
            } else {
                size_t copy_n = new_size < len_ ? new_size : len_;
                if (new_size > len_) {
                    std::memset(small_buffer + copy_n, '\0', new_size - copy_n);
                }
                small_buffer[new_size] = '\0';
                len_ = new_size;
            }
            return;
        }
        // need heap storage
        if (new_size > capacity()) {
            size_t chosen = grow_capacity(is_sso() ? 15 : cap_, new_size);
            char* new_mem = new char[chosen + 1];
            if (len_) std::memcpy(new_mem, data_ptr(), len_);
            if (new_size > len_) std::memset(new_mem + len_, '\0', new_size - len_);
            new_mem[new_size] = '\0';
            if (!is_sso()) delete[] heap_ptr;
            heap_ptr = new_mem;
            cap_ = chosen;
            len_ = new_size;
        } else {
            // already enough capacity
            if (is_sso()) {
                // moving from SSO to heap because new_size > 15 but capacity() == 15
                size_t chosen = grow_capacity(15, new_size);
                char* new_mem = new char[chosen + 1];
                if (len_) std::memcpy(new_mem, small_buffer, len_);
                if (new_size > len_) std::memset(new_mem + len_, '\0', new_size - len_);
                new_mem[new_size] = '\0';
                heap_ptr = new_mem;
                cap_ = chosen;
                len_ = new_size;
            } else {
                if (new_size > len_) {
                    std::memset(heap_ptr + len_, '\0', new_size - len_);
                }
                heap_ptr[new_size] = '\0';
                len_ = new_size;
            }
        }
    }

    char& operator[](size_t index) {
        if (index >= len_) throw std::out_of_range("index out of range");
        return *(const_cast<char*>(data_ptr()) + index);
    }

    MyString operator+(const MyString& rhs) const {
        size_t n1 = len_, n2 = rhs.len_;
        size_t total = n1 + n2;
        MyString res;
        if (total <= 15) {
            if (n1) std::memcpy(res.small_buffer, data_ptr(), n1);
            if (n2) std::memcpy(res.small_buffer + n1, rhs.data_ptr(), n2);
            res.small_buffer[total] = '\0';
            res.len_ = total;
            res.cap_ = 0;
        } else {
            size_t chosen = total; // exact fit acceptable
            res.cap_ = chosen;
            res.heap_ptr = new char[chosen + 1];
            if (n1) std::memcpy(res.heap_ptr, data_ptr(), n1);
            if (n2) std::memcpy(res.heap_ptr + n1, rhs.data_ptr(), n2);
            res.heap_ptr[total] = '\0';
            res.len_ = total;
        }
        return res;
    }

    void append(const char* str) {
        if (!str) return;
        size_t add = std::strlen(str);
        if (add == 0) return;
        size_t new_len = len_ + add;
        if (new_len <= 15) {
            // stay in SSO
            std::memcpy(small_buffer + len_, str, add);
            len_ = new_len;
            small_buffer[len_] = '\0';
            return;
        }
        if (new_len > capacity()) {
            size_t chosen = grow_capacity(is_sso() ? 15 : cap_, new_len);
            char* new_mem = new char[chosen + 1];
            if (len_) std::memcpy(new_mem, data_ptr(), len_);
            std::memcpy(new_mem + len_, str, add);
            new_mem[new_len] = '\0';
            if (!is_sso()) delete[] heap_ptr;
            heap_ptr = new_mem;
            cap_ = chosen;
            len_ = new_len;
        } else {
            // enough capacity
            if (is_sso()) {
                // move from SSO to heap
                char* new_mem = new char[cap_ + 1]; // but cap_==0 here, so handle specially
                // However in SSO, capacity() is 15 and new_len > 15, so allocate
                size_t chosen = grow_capacity(15, new_len);
                new_mem = new char[chosen + 1];
                if (len_) std::memcpy(new_mem, small_buffer, len_);
                std::memcpy(new_mem + len_, str, add);
                new_mem[new_len] = '\0';
                heap_ptr = new_mem;
                cap_ = chosen;
                len_ = new_len;
            } else {
                std::memcpy(heap_ptr + len_, str, add);
                len_ = new_len;
                heap_ptr[len_] = '\0';
            }
        }
    }

    const char& at(size_t pos) const {
        if (pos >= len_) throw std::out_of_range("index out of range");
        return *(data_ptr() + pos);
    }

    class const_iterator;

    class iterator {
    private:
        char* ptr{};
    public:
        explicit iterator(char* p = nullptr) : ptr(p) {}
        iterator& operator++() { ++ptr; return *this; }
        iterator operator++(int) { iterator tmp(*this); ++ptr; return tmp; }
        iterator& operator--() { --ptr; return *this; }
        iterator operator--(int) { iterator tmp(*this); --ptr; return tmp; }
        char& operator*() const { return *ptr; }
        bool operator==(const iterator& other) const { return ptr == other.ptr; }
        bool operator!=(const iterator& other) const { return ptr != other.ptr; }
        bool operator==(const const_iterator& other) const { return ptr == other.ptr; }
        bool operator!=(const const_iterator& other) const { return ptr != other.ptr; }
    };

    class const_iterator {
    private:
        const char* ptr{};
    public:
        explicit const_iterator(const char* p = nullptr) : ptr(p) {}
        const_iterator& operator++() { ++ptr; return *this; }
        const_iterator operator++(int) { const_iterator tmp(*this); ++ptr; return tmp; }
        const_iterator& operator--() { --ptr; return *this; }
        const_iterator operator--(int) { const_iterator tmp(*this); --ptr; return tmp; }
        const char& operator*() const { return *ptr; }
        bool operator==(const const_iterator& other) const { return ptr == other.ptr; }
        bool operator!=(const const_iterator& other) const { return ptr != other.ptr; }
        friend class MyString::iterator;
    };

public:
    iterator begin() { return iterator(const_cast<char*>(data_ptr())); }
    iterator end() { return iterator(const_cast<char*>(data_ptr()) + len_); }
    const_iterator cbegin() const { return const_iterator(data_ptr()); }
    const_iterator cend() const { return const_iterator(data_ptr() + len_); }
};

#endif
