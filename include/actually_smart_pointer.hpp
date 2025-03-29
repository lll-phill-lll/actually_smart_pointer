#pragma once

#include <string>

namespace asp {

template <typename T>
struct control_block {
    T* ptr;
    std::string history;

    control_block(T* p);
    ~control_block();

    void append_interaction(const std::string& method, const std::string& reply);
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
};

} // namespace asp
