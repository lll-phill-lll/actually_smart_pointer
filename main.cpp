#include "actually_smart_pointer.hpp"
#include <iostream>

int main() {
    using asp::actually_smart_pointer;

    std::cout << "Creating smart pointer p...\n";
    actually_smart_pointer<std::string> p(new std::string("hello"));

    std::cout << "*p = " << *p << "\n";

    {
        std::cout << "Copying p into q...\n";
        actually_smart_pointer<std::string> q = p;

        std::cout << "*q = " << *q << "\n";

        std::cout << "Leaving inner scope...\n";
    }

    std::cout << "Back in outer scope...\n";
    std::cout << "Now letting p go out of scope...\n";

    return 0;
}

