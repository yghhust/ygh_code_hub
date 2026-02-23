#include <iostream>
#include "utils.h"
#include "module_a/a1.h"
#include "module_b/b1.h"

int main() {
    std::cout << "=== Demo Application ===" << std::endl;
    
    Utils::printHello();
    
    A1::doSomething();
    B1::doSomething();
    
    std::cout << "=== Done ===" << std::endl;
    return 0;
}