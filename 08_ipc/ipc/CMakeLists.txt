# ================================================================================================
# Generic CMakeLists.txt for C/C++ Program  
#  
# License: GPL (General Public License)  
# Author:  yuguohua <ghy_hust@qq.com>  
# Date:    2026/01/18    version 0.1  initial
#          2026/01/19    version 0.2  新增目录排除、文件排除功能
#          2026/01/19    version 0.3  新增编译链接选项设置
#          2026/01/24    version 0.4  支持将源码所在目录设置为CMakeLists文件所在的根目录
#          2026/01/28    version 0.5  排除文件为空时bug修复
#          2026/02/23    version 0.6  全面优化：修复bug、增强可读性、增加功能
#          2026/02/23    version 0.7  修复RELATIVE_PATH路径错误、排除规则生效
#          2026/03/01    version 0.8  支持编译宏定义
#
# Description:  
# ------------ 
# 通用CMakeLists.txt, 按需定制如下配置即可开展cmake构建：
# 1）项目及目标文件设定
#    MY_PROJECT/MY_BIN: 项目名称及输出二进制名称
# 2）源码路径设定
#    SRC_DIR: 源文件根目录（相对或绝对路径）
#    EXC_FILES/EXC_DIRS: 排除的文件和目录（相对SRC_DIR的路径）
# 3）编译链接选项设定
#    COMMON_WARNINGS: 通用警告选项
#    CFLAGS/CXXFLAGS: 编译器选项
#    LDFLAGS: 链接器选项
#    LIBS: 需要链接的库
# 4）编译宏设定
#    MACRO_DEFINES: 编译宏
# ================================================================================================

cmake_minimum_required(VERSION 3.10)
project(MyApp)

# ==================== 1. 项目配置 ====================

# 项目名称
set(MY_LIB "ipc")

# 源码根目录（改成你的路径，支持相对或绝对路径，相对路径是相对CMakeList.txt的路径）
set(SRC_DIR ".")
# 源文件后缀
set(SRC_EXTENSIONS "*.cpp" "*.cc" "*.cp" "*.cxx" "*.c" "*.C")

# ==================== 3. 编译宏定义 ====================
set(MACRO_DEFINES 
   # AUTOREG_LOG_LEVEL=4
)

# ==================== 4. 编译选项配置 ====================
# 通用警告选项
set(COMMON_WARNINGS
    "-Wall"
    "-Wextra"
    "-Wno-unused-variable"
    "-Wno-unused-parameter"
    "-Wno-pedantic"
    "-Wno-overloaded-virtual"
    "-Wno-return-type"
)

# 编译链接选项
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==================== 5. 链接库配置 ====================

# 需要链接的库（库名或全路径，用分号分隔）
set(LIBS
    "third"
    # "m"           # 数学库
)

# ==================== 6. 获取源文件 ====================
# 递归获取所有源文件
set(ALL_SOURCES "")
foreach(ext IN LISTS SRC_EXTENSIONS)
    file(GLOB_RECURSE files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SRC_DIR}/${ext}")
    list(APPEND ALL_SOURCES ${files})
endforeach()

# ==================== 8. 创建可执行文件 ====================

add_library(${MY_LIB} ${ALL_SOURCES})

target_include_directories(${MY_LIB} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/interface
    ${CMAKE_CURRENT_SOURCE_DIR}/zmq
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(${MY_LIB} PUBLIC ${LIBS})

# ==================== 9. 输出信息 ====================

message(STATUS "========================================")
message(STATUS "Build Configuration:")
message(STATUS "  Binary:  ${MY_LIB}")
message(STATUS "  Source Dir: ${SRC_DIR}")
message(STATUS "  Sources: ${ALL_SOURCES}")
message(STATUS "  Libraries: ${LIBS}")
message(STATUS "  Macro Definitions: ${MACRO_DEFINES}")
message(STATUS "========================================")
