#include "actually_smart_pointer.hpp"
#include <iostream>

int main() {
    asp::actually_smart_pointer<int> p(new int(5));

    std::cout << p.ask("Write a symphony")<< std::endl;
}
