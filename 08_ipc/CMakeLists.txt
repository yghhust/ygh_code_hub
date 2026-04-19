# ================================================================================================
#  
# License: GPL (General Public License)  
# Author:  yuguohua <ghy_hust@qq.com>  
# Date:    
#          2026/04/06    version 0.9  CMakeLists module
# Description:  
# ------------ 
#          Top CMakeLists.txt
#
# ================================================================================================

# 1. 基本配置
cmake_minimum_required(VERSION 3.15)
project(MyProject VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 2. 全局编译选项
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

# 3. 添加模块子目录
add_subdirectory(third_party EXCLUDE_FROM_ALL)
add_subdirectory(ipc)
add_subdirectory(example)
