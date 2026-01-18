# ====================================================================
# Generic CMakeLists.txt for C/C++ Program  
#  
# License: GPL (General Public License)  
# Author:  yuguohua <ghy_hust@qq.com>  
# Date:    2026/01/18    version 0.1  initial
#          2026/01/19    version 0.2  新增目录排除、文件排除功能
#          2026/01/19    version 0.3  新增编译链接选项设置
#
# Description:  
# ------------ 
# 通用CMakeLists.txt, 按需定制如下配置即可开展cmake构建：
# 1）项目及目标文件设定
#    MY_PROJECT/MY_PROJECT_DESCRIPTION: 项目名称及描述
#    MY_BIN: 目标文件名称
# 2）源码路径设定
#    SRC_ROOT: 源文件根目录
#    SRCEXTS/HDREXTS: 源码类型
#    EXT_FILS: 设置需要正则表达
#    EXT_DIRS: 必要时，还可以设置EXT_DIR制定外部头文件搜索路径
# 3）lib库设定
#    EXT_LIBS（暂未实现）
# 4) 编译链接选项设定
#    CPPFLAGS/CFLAGS：编译选项（暂未实现）
#    LDFLAGS：
# ====================================================================

#
# 项目定制区：基于项目实际情况按需设置
# --------------------------------------------------------------------
# 1.定义项目名称
set(MY_PROJECT "Demo")
set(MY_PROJECT_DESCRIPTION "CMake for Demo")
set(MY_BIN "my_app")

# 2.设置源码
# 源码路径（相对CMakeLists路径）
set(SRC_ROOT "src") 
# 源码类型
set(SRCEXTS "*.c;*.cc;*.cpp")
set(HDREXTS "*.h;*.hh;*.hpp")
# 排除的文件和目录
set(EXC_FILES "taskEngine.cc;src/utils/log/log.cc")# 例如"xxx/yyy.cc;zzz.cc", 相对与CMakeLists.txts所在位置的相对路径
set(EXC_DIRS "src/utils/log")  		     # 例如"xxx/yyy;zzz", 相对与CMakeLists.txts所在位置的相对路径

# 3.编译链接设定
# CPPFLAGS: C PreProcessor Flags 传递给预处理器（cpp）的选项，如 -I（头文件路径）、-D（宏定义）、-U（取消宏）等。
set(CPPFLAGS "") #set(CPPFLAGS "-Dxxx" "-Dyyy")

#CFLAGS: C Compiler Flags 传递给 C 编译器（如 gcc/clang）的选项，控制 C 代码的编译行为，包括：
#警告选项(-Wall, -Wextra); 优化选项(-O2, -Os); 代码生成选项(-fPIC, -m32); 标准版本(-std=c99)
set(CFLAGS "")  #set(CPPFLAGS "-Wall" "-O2")

#CXXFLAGS: C++ Compiler Flags传递给 C++ 编译器（如 g++/clang++）的选项，作用同 CFLAGS，但专用于 C++ 代码。
set(CXXFLAGS "")  #set(CPPFLAGS "-Wall" "-O2")

#LDFLAGS: Linker Flags 传递给 链接器（ld）的选项，控制链接阶段的行为，如：
# 库路径(-L); 链接库(-lxxx); 链接脚本(-T); 链接时优化(-flto)
set(LDFLAGS "") 

# 定义警告选项
set(COMMON_WARNINGS
    "-Wall" "-Wextra"
    "-Wno-unused-variable"
    "-Wno-unused-parameter"
    "-Wno-pedantic"
    "-Wno-overloaded-virtual"
    "-Wno-return-type"
)

#
# 主体功能实现区(谨慎修改或不要修改！)
# --------------------------------------------------------------------
# 1. cmake通用库
include (CMakeParseArguments)
#include(CMakeLists.template.txt)

#-------------------------------------------------------------------------------------------------
# @brief      获得待编译源码文件列表
#
# @details    1. 从指定的目录及子目录下查找指定类型的文件列表；
#             2. 查找结果通过出参返回
#
# @param      files_out        出参-满足条件的待编译文件列表，文件携带相对路径（相对CMakeLists文件所在路径）
# @param      curdir           入参-待搜索的根目录（一般是源码所在路径）
# @param      file_extentions  入参-源码类型列表（如：*.c;*.cc;*.cpp）
#
# @sets       
#
# @example     
#
# @note       
#
#-------------------------------------------------------------------------------------------------
function(get_files_recurse files_out curdir file_extentions) 
    list(TRANSFORM file_extentions PREPEND "${curdir}/")
    file(GLOB_RECURSE local_files LIST_DIRECTORIES false RELATIVE ${curdir} ${file_extentions})
    #message(STATUS "get_files_recurse: local_files: ${local_files}")
    file(RELATIVE_PATH rel_path "${CMAKE_CURRENT_SOURCE_DIR}" "${curdir}")
    list(TRANSFORM local_files PREPEND "${rel_path}/")

    set(${files_out} ${local_files} PARENT_SCOPE)
endfunction()
 
 
#-------------------------------------------------------------------------------------------------
# @brief      获取满足条件的目录列表
#
# @details    1. 从指定的目录及子目录下查找是否包含指定类型文件(一般是头文件)
#             2. 当目录中存在指定类型的文件时，该目录加入返回列表
#             3. 递归查找所有目录
#
# @param      dirs_out         出参-满足条件的头文件目录列表，目录携带相对路径（相对CMakeLists文件所在路径）
# @param      curdir           入参-待搜索的根目录（一般是头文件所在路径）
# @param      file_extentions  入参-头文件类型列表（如：*.h;*.hh;*.hpp）
#
# @sets       
#
# @example     
#
# @note       
#
#-------------------------------------------------------------------------------------------------
function(get_dirs_recurse dirs_out curdir file_extentions) 
    set(files "")
    
    # 调用函数并获取返回值
    get_files_recurse(files ${curdir} "${file_extentions}")
    
    if(files)  # 检查 files 是否非空
        foreach(item ${files})
            get_filename_component(item_dir ${item} DIRECTORY)
            # message(STATUS "[get_dirlist_recurse_with_filter]item_dir: ${item_dir}")
            list(APPEND local_dirs ${item_dir})
            # message(STATUS "[get_dirlist_recurse_with_filter]local_dirs: ${local_dirs}")
        endforeach()
        
        # 去重
        if(local_dirs)
            list(REMOVE_DUPLICATES local_dirs)
            # message(STATUS "[get_dirlist_recurse_with_filter]unique dirs: ${local_dirs}")
        endif()
        
        # 返回结果
        set(${dirs_out} ${local_dirs} PARENT_SCOPE)
    else()
        message(WARNING "[get_dirlist_recurse_with_filter]No files found!")
        set(${dirs_out} "" PARENT_SCOPE)
    endif()
endfunction()


#-------------------------------------------------------------------------------------------------
# @brief      从文件列表中排除部分文件
#
# @details    1. 给定一份文件列表和待排除的文件列表
#             2. 排除后的文件列表通过出参返回
#
# @param      files_excl       出参-按照设定规则排除部分文件后，返回剩余的文件列表
# @param      files            入参-原始文件列表
# @param      file_exclutions  入参-待排除文件列表（如：xx.c;yy.cc;zz.cpp）
#
# @sets       
#
# @example     
#
# @note       
#
#-------------------------------------------------------------------------------------------------
function(files_exclusion files_excl files file_exclutions)
    # 获取列表内容
    set(local_list ${files})
    
    # 构建和执行过滤
    string(JOIN "|" pattern ${file_exclutions})
    #list(FILTER local_list EXCLUDE REGEX ".*/(${pattern})$")
    list(FILTER local_list EXCLUDE REGEX "(${pattern})$")
    #message(STATUS "pattern: ${pattern}")
    #message(STATUS "local_list: ${local_list}")
    # 返回结果
    set(${files_excl} ${local_list} PARENT_SCOPE)
endfunction()

#-------------------------------------------------------------------------------------------------
# @brief      从目录列表中排除目录
#
# @details    1. 给定一份目录列表和待排除的目录列表
#             2. 待排除的目录下面的子目录一并被排除
#             2. 排除后的目录列表通过出参返回
#
# @param      dirs_excl       出参-按照设定规则排除部分目录后，返回剩余的目录列表
# @param      dirs            入参-原始目录列表
# @param      dir_exclutions  入参-待排除目录列表（如：xx/yy;aa/bb/cc）
#
# @sets       
#
# @example     
#
# @note       
#
#-------------------------------------------------------------------------------------------------
function(dirs_exclusion dirs_excl dirs dir_exclutions)
    # 获取列表内容
    set(local_list ${dirs})
    #message(STATUS "dirs_exclusion local_list: ${local_list}")
    #message(STATUS "dirs_exclusion in_dirs: ${in_dirs}")
    
    # 构建和执行过滤
    string(JOIN "|" pattern ${dir_exclutions})
    #message(STATUS "dirs_exclusion pattern: ${pattern}")
    list(FILTER local_list EXCLUDE REGEX "^(${pattern})(/.*)?$")
    #message(STATUS "local_list: ${local_list}")
    set(${dirs_excl} ${local_list} PARENT_SCOPE)
endfunction()

# 2.设置编译相关选项
# cmake版本要求
cmake_minimum_required(VERSION 3.16...3.27)
project(${MY_PROJECT} 
    VERSION 1.0.0
    DESCRIPTION ${MY_PROJECT_DESCRIPTION}
    LANGUAGES CXX
)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置编译选项
# 全局应用到所有目标（C 和 C++）
add_compile_options(${COMMON_WARNINGS})

set(CMAKE_CPP_FLAGS "${CMAKE_CPP_FLAGS} ${CPPLAGS}")

if(CMAKE_C_COMPILER_LOADED)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CFLAGS}")
endif()

if(CMAKE_CXX_COMPILER_LOADED)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXXFLAGS}")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -Wno-pedantic -Wno-overloaded-virtual -Wno-return-type)
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LDFLAGS}")

# 3. 创建可执行文件
# 可执行文件
get_files_recurse(sources "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_ROOT}" "${SRCEXTS}")
#message(STATUS "sources: ${sources}")
# 排除不需要的文件
dirs_exclusion(sources_excl "${sources}" "${EXC_DIRS}")
#message(STATUS "sources_excl: ${sources_excl}")
files_exclusion(sources_excl2 "${sources_excl}" "${EXC_FILES}")
#message(STATUS "sources_excl2: ${sources_excl2}")
add_executable(${PROJECT_NAME} ${sources_excl2})


# 头文件搜索目录
get_dirs_recurse(header_dirs "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_ROOT}" "${HDREXTS}")
# 排除不需要的目录
dirs_exclusion(header_dirs_excl "${header_dirs}" "${EXC_DIRS}")
#message(STATUS "header_dirs_excl: ${header_dirs_excl}")
target_include_directories(${PROJECT_NAME} PRIVATE ${header_dirs_excl})

# 链接库
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
#target_link_libraries(${PROJECT_NAME} PRIVATE
#    phtread
#    utils_lib
#    network_lib
#    json_lib
#)

# 8.设置目标属性
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    OUTPUT_NAME ${MY_BIN}
    CXX_STANDARD 17
)

# 6.安装配置（可选，相对路径是/usr/local/${INSTALL_DIR},通过make install命令实现）
# install(FILES $<TARGET_FILE: ${MY_BIN}> DESTINATION ${INSTALL_DIR}})
