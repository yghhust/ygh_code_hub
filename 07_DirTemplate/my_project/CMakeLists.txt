# ================================================================================================
# Generic CMakeLists.txt for C/C++ Program  
#  
# License: GPL (General Public License)  
# Author:  yuguohua <ghy_hust@qq.com>  
# Date:    
#          2026/04/06    version 0.9  模块化CMakeLists
# Description:  
# ------------ 
#          顶层CMakeLists.txt
#
# ================================================================================================

cmake_minimum_required(VERSION 3.16)
project(MyApp VERSION 1.0 LANGUAGES CXX)

# ==================== 0. 全局配置 ====================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==================== 1. 项目配置 ====================
# 要排除的目录（相对 CMAKE_SOURCE_DIR 的路径，用分号分隔）
set(EXCLUDE_DIRS 
    "build"
    "model"
    "test"
    #"apps"
)
# 头文件后缀
set(HDR_EXTENSIONS 
    "*.h" "*.hh" "*.hpp" "*.hxx" "*.H"
)
# 编译宏定义
set(MACRO_DEFINES 
    CGRAPH_USER_LOG
)

# ==================== 2. 编译选项配置 ====================
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
# C 编译器选项
set(CFLAGS "-std=c11")
# C++ 编译器选项
set(CXXFLAGS "-std=c++17")
# 链接器选项
set(LDFLAGS "")

# ==================== 3. 链接库配置 ====================
# 需要链接的库（库名或全路径，用分号分隔）
set(LIBS
    # "pthread"
    # "m"           # 数学库
)

# ==================== 4. 公共头文件目录 ====================

# 递归获取 SRC_DIR下面的所有目录（包括子目录）
file(GLOB_RECURSE ALL_DIRS LIST_DIRECTORIES true "${CMAKE_SOURCE_DIR}/*")

# 只保留真正的目录，并应用排除规则
set(ALL_INCLUDES "")
foreach(d IN LISTS ALL_DIRS)
    # 注意需要使用绝对路径进行判断！
	if(NOT IS_DIRECTORY "${d}")
	    continue()
	endif()
	
	# 检查是否在排除目录列表中
	set(excluded FALSE)
	foreach(excl_dir IN LISTS EXCLUDE_DIRS)
	    if("${d}" MATCHES "^${CMAKE_SOURCE_DIR}/${excl_dir}(/.*)?$")
			set(excluded TRUE)
			break()
	    endif()
	endforeach()
	
	if(NOT excluded)
	    list(APPEND ALL_INCLUDES "${d}")
	endif()
endforeach()

# 追加自身目录
list(APPEND ALL_INCLUDES "${CMAKE_SOURCE_DIR}")

# 去重
list(REMOVE_DUPLICATES ALL_INCLUDES)

# ==================== 5. 添加应用子目录 ====================
# 定义一个接口库(用于收集全局编译设置)
add_library(global_settings INTERFACE)
# 全局编译设
target_compile_definitions(global_settings INTERFACE ${MACRO_DEFINES})
target_include_directories(global_settings INTERFACE ${ALL_INCLUDES})
target_compile_options(global_settings INTERFACE 
    ${COMMON_WARNINGS}
    $<$<COMPILE_LANGUAGE:C>:${CFLAGS}>
    $<$<COMPILE_LANGUAGE:CXX>:${CXXFLAGS}>
)
target_link_options(global_settings INTERFACE ${LDFLAGS})
target_link_libraries(global_settings INTERFACE ${LIBS})

add_subdirectory(src)
add_subdirectory(modules)
add_subdirectory(third_party)
add_subdirectory(apps/cli)
add_subdirectory(apps/gui)

# ==================== 6. 输出信息 ====================
message(STATUS "========================================")
message(STATUS "Top Level Build Configuration:")
message(STATUS "  Common Include Dirs: ${ALL_INCLUDES}")
message(STATUS "  Excluded Dirs: ${EXCLUDE_DIRS}")
message(STATUS "  Macro Definitions: ${MACRO_DEFINES}")
message(STATUS "========================================")

