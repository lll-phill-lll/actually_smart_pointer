#pragma once

#include <string>

namespace asp {

template <typename T>
struct control_block {
    T* ptr;
    size_t ref_count;
    void* model;
    void* context;
    std::string type_name;

    control_block(T* p, const std::string& type);
    ~control_block();

    bool should_destroy();
    void retain();
    void release();
};

template <typename T>
class actually_smart_pointer {
public:
    actually_smart_pointer(T* ptr);
    actually_smart_pointer(const actually_smart_pointer& other);
    actually_smart_pointer& operator=(const actually_smart_pointer& other);
    ~actually_smart_pointer();

    T* get() const;
    T& operator*() const;
    T* operator->() const;

private:
    control_block<T>* ctrl_;
};

std::string ask_llm(const std::string& event, const std::string& type);

} // namespace asp

