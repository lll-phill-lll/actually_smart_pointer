#pragma once

#include <string>

namespace asp {

template <typename T>
struct control_block {
    T* ptr;
    std::string type_name;

    control_block(T* p, const std::string& type);
    ~control_block();
};

template <typename T>
class actually_smart_pointer {
public:
    actually_smart_pointer(T* ptr);
    actually_smart_pointer(const actually_smart_pointer& other);
    actually_smart_pointer& operator=(const actually_smart_pointer& other);
    actually_smart_pointer(actually_smart_pointer&& other) noexcept;
    actually_smart_pointer& operator=(actually_smart_pointer&& other) noexcept;
    ~actually_smart_pointer();

    T* get() const;
    T& operator*() const;
    T* operator->() const;

private:
    control_block<T>* ctrl_;
    std::string history_;
};

} // namespace asp

