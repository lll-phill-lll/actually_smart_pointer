#include "actually_smart_pointer.hpp"
#include <iostream>

int main() {
    asp::actually_smart_pointer<std::string> p(new std::string("Hello"));
    std::cout << *p << std::endl;

    asp::actually_smart_pointer<std::string> p2 = p;
}
