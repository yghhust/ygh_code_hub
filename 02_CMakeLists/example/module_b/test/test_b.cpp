// 这个文件会被 EXCLUDE_DIRS 中的 "module_b/test" 排除
#include <iostream>
void testFunction() {
    std::cout << "This is a test file, should not be compiled!" << std::endl;
}