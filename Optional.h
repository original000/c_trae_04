#ifndef OPTIONAL_H
#define OPTIONAL_H

#include <utility>
#include <type_traits>

// 简单的optional类型实现，用于替代C++17的std::optional
template <typename T>
class Optional {
public:
    // 默认构造函数：创建一个空的optional
    Optional() noexcept : has_value_(false) {}
    
    // 构造函数：从值创建optional
    Optional(const T& value) : has_value_(true) {
        new (&storage_) T(value);
    }
    
    Optional(T&& value) noexcept : has_value_(true) {
        new (&storage_) T(std::move(value));
    }
    
    // 拷贝构造函数
    Optional(const Optional& other) : has_value_(other.has_value_) {
        if (has_value_) {
            new (&storage_) T(*other.ptr());
        }
    }
    
    // 移动构造函数
    Optional(Optional&& other) noexcept : has_value_(other.has_value_) {
        if (has_value_) {
            new (&storage_) T(std::move(*other.ptr()));
            other.reset();
        }
    }
    
    // 析构函数
    ~Optional() {
        if (has_value_) {
            ptr()->~T();
        }
    }
    
    // 拷贝赋值运算符
    Optional& operator=(const Optional& other) {
        if (this != &other) {
            if (has_value_) {
                ptr()->~T();
            }
            
            has_value_ = other.has_value_;
            if (has_value_) {
                new (&storage_) T(*other.ptr());
            }
        }
        return *this;
    }
    
    // 移动赋值运算符
    Optional& operator=(Optional&& other) noexcept {
        if (this != &other) {
            if (has_value_) {
                ptr()->~T();
            }
            
            has_value_ = other.has_value_;
            if (has_value_) {
                new (&storage_) T(std::move(*other.ptr()));
                other.reset();
            }
        }
        return *this;
    }
    
    // 赋值运算符：从值赋值
    Optional& operator=(const T& value) {
        if (has_value_) {
            *ptr() = value;
        } else {
            new (&storage_) T(value);
            has_value_ = true;
        }
        return *this;
    }
    
    Optional& operator=(T&& value) noexcept {
        if (has_value_) {
            *ptr() = std::move(value);
        } else {
            new (&storage_) T(std::move(value));
            has_value_ = true;
        }
        return *this;
    }
    
    // 检查是否有值
    explicit operator bool() const noexcept {
        return has_value_;
    }
    
    bool has_value() const noexcept {
        return has_value_;
    }
    
    // 获取值（如果没有值，行为未定义）
    T& value() & noexcept {
        return *ptr();
    }
    
    const T& value() const& noexcept {
        return *ptr();
    }
    
    T&& value() && noexcept {
        return std::move(*ptr());
    }
    
    const T&& value() const&& noexcept {
        return std::move(*ptr());
    }
    
    // 重置为无值状态
    void reset() noexcept {
        if (has_value_) {
            ptr()->~T();
            has_value_ = false;
        }
    }
    
    // 交换两个optional
    void swap(Optional& other) noexcept {
        if (has_value_ && other.has_value_) {
            using std::swap;
            swap(*ptr(), *other.ptr());
        } else if (has_value_) {
            new (&other.storage_) T(std::move(*ptr()));
            ptr()->~T();
            other.has_value_ = true;
            has_value_ = false;
        } else if (other.has_value_) {
            new (&storage_) T(std::move(*other.ptr()));
            other.ptr()->~T();
            has_value_ = true;
            other.has_value_ = false;
        }
    }
    
private:
    // 获取指向存储的指针
    T* ptr() noexcept {
        return reinterpret_cast<T*>(&storage_);
    }
    
    const T* ptr() const noexcept {
        return reinterpret_cast<const T*>(&storage_);
    }
    
    bool has_value_;  // 是否包含值
    alignas(T) unsigned char storage_[sizeof(T)];  // 存储值的内存
};

// 辅助函数：创建optional
template <typename T>
Optional<T> make_optional(T&& value) {
    return Optional<T>(std::forward<T>(value));
}

// 辅助函数：交换两个optional
template <typename T>
void swap(Optional<T>& lhs, Optional<T>& rhs) noexcept {
    lhs.swap(rhs);
}

// 比较运算符
template <typename T>
bool operator==(const Optional<T>& lhs, const Optional<T>& rhs) {
    if (lhs.has_value() && rhs.has_value()) {
        return lhs.value() == rhs.value();
    }
    return lhs.has_value() == rhs.has_value();
}

template <typename T>
bool operator!=(const Optional<T>& lhs, const Optional<T>& rhs) {
    return !(lhs == rhs);
}

template <typename T>
bool operator<(const Optional<T>& lhs, const Optional<T>& rhs) {
    if (rhs.has_value()) {
        if (lhs.has_value()) {
            return lhs.value() < rhs.value();
        }
        return true;  // 空optional小于任何非空optional
    }
    return false;  // 非空optional不小于空optional
}

template <typename T>
bool operator>(const Optional<T>& lhs, const Optional<T>& rhs) {
    return rhs < lhs;
}

template <typename T>
bool operator<=(const Optional<T>& lhs, const Optional<T>& rhs) {
    return !(lhs > rhs);
}

template <typename T>
bool operator>=(const Optional<T>& lhs, const Optional<T>& rhs) {
    return !(lhs < rhs);
}

// 与值的比较
template <typename T>
bool operator==(const Optional<T>& opt, const T& value) {
    return opt.has_value() && opt.value() == value;
}

template <typename T>
bool operator==(const T& value, const Optional<T>& opt) {
    return opt == value;
}

template <typename T>
bool operator!=(const Optional<T>& opt, const T& value) {
    return !(opt == value);
}

template <typename T>
bool operator!=(const T& value, const Optional<T>& opt) {
    return !(opt == value);
}

template <typename T>
bool operator<(const Optional<T>& opt, const T& value) {
    return !opt.has_value() || opt.value() < value;
}

template <typename T>
bool operator<(const T& value, const Optional<T>& opt) {
    return opt.has_value() && value < opt.value();
}

template <typename T>
bool operator>(const Optional<T>& opt, const T& value) {
    return opt.has_value() && opt.value() > value;
}

template <typename T>
bool operator>(const T& value, const Optional<T>& opt) {
    return !opt.has_value() || value > opt.value();
}

template <typename T>
bool operator<=(const Optional<T>& opt, const T& value) {
    return !opt.has_value() || opt.value() <= value;
}

template <typename T>
bool operator<=(const T& value, const Optional<T>& opt) {
    return opt.has_value() && value <= opt.value();
}

template <typename T>
bool operator>=(const Optional<T>& opt, const T& value) {
    return opt.has_value() && opt.value() >= value;
}

template <typename T>
bool operator>=(const T& value, const Optional<T>& opt) {
    return !opt.has_value() || value >= opt.value();
}

#endif // OPTIONAL_H