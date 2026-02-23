// 这个文件会被 EXCLUDE_DIRS 中的 "module_a/legacy" 排除
#include <iostream>
void oldFunction() {
    std::cout << "This should not be compiled!" << std::endl;
}