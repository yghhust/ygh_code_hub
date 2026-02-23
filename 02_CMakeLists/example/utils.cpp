#include "utils.h"
#include <iostream>

namespace Utils {
    void printHello() {
        std::cout << "Hello from Utils!" << std::endl;
    }
    
    int add(int a, int b) {
        return a + b;
    }
}