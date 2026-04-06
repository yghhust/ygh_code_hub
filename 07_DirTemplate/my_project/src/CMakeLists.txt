# ================================================================================================
# Generic CMakeLists.txt for C/C++ Program  
#  
# License: GPL (General Public License)  
# Author:  yuguohua <ghy_hust@qq.com>  
# Date:    2026/04/06    version 0.9  模块化CMakeLists
#
# Description:  
# ------------ 
#          CMake模块化设置（下层CMakeLists）
# ================================================================================================

cmake_minimum_required(VERSION 3.16)

# ==================== 1. 项目配置 ====================
# 项目名称
set(MY_LIB "infra")

# 要排除的目录（相对CMAKE_CURRENT_SOURCE_DIR的路径，用分号分隔）
set(EXCLUDE_DIRS 
    #"../../build"
)

# 要排除的文件（用分号分隔）
set(EXCLUDE_FILES 
    #"CMakeCXXCompilerId.cpp"
    #"CMakeCCompilerId.c"
)

# 源文件后缀
set(SRC_EXTENSIONS 
    "*.cpp" "*.cc" "*.cp" "*.cxx" "*.c" "*.C"
)

# 编译宏定义（用分号分隔）
set(MACRO_DEFINES 
    # xxxx
)

# ==================== 2. 链接库配置 ====================

# 需要链接的库（库名或全路径，用分号分隔）
set(LIBS
    "third"
    # "m"           # 数学库
    # "dl"          # 动态链接库
    # "rt"          # 实时库
)

# ==================== 3. 获取源文件 ====================
# 递归获取所有源文件
set(ALL_SOURCES "")
foreach(ext IN LISTS SRC_EXTENSIONS)
    file(GLOB_RECURSE files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/${ext}")
    list(APPEND ALL_SOURCES ${files})
endforeach()

# 排除指定文件
foreach(excl IN LISTS EXCLUDE_FILES)
    list(FILTER ALL_SOURCES EXCLUDE REGEX ".*/${excl}$")
endforeach()

# 排除指定目录下的所有文件
foreach(excl_dir IN LISTS EXCLUDE_DIRS)
    list(FILTER ALL_SOURCES EXCLUDE REGEX ".*${excl_dir}/.*")
endforeach()

# 去重
list(REMOVE_DUPLICATES ALL_SOURCES)

# ==================== 4. 创建目标文件 ====================
add_library(${MY_LIB} ${ALL_SOURCES})

# 继承全局定义
target_link_libraries(${MY_LIB} PRIVATE global_settings)

# 链接库(私有)
target_link_libraries(${MY_LIB} PRIVATE ${LIBS})

# 宏定义(私有)
target_compile_definitions(${MY_LIB} PRIVATE ${MACRO_DEFINES})


# ==================== 5. 输出信息 ====================

message(STATUS "========================================")
message(STATUS "Build Configuration:")
message(STATUS "  Binary:  ${MY_LIB}")
message(STATUS "  Source Dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "  Sources: ${ALL_SOURCES}")
message(STATUS "  Excluded Dirs: ${EXCLUDE_DIRS}")
message(STATUS "  Excluded Files: ${EXCLUDE_FILES}")
message(STATUS "  Libraries: ${LIBS}")
message(STATUS "  Macro Definitions: ${MACRO_DEFINES}")
message(STATUS "========================================")
