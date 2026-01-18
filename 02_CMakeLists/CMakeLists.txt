# ====================================================================
# Generic CMakeLists.txt for C/C++ Program  
#  
# License: GPL (General Public License)  
# Author:  yuguohua <ghy_hust@qq.com>  
# Date:    2026/01/18    version 0.1  initial
#          2026/01/19    version 0.2  新增目录排除、文件排除功能
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

# 0.项目定制设定
# 0.1.定义项目名称
set(MY_PROJECT "Demo")
set(MY_PROJECT_DESCRIPTION "CMake for Demo")
set(MY_BIN "my_app")

# 0.2.设置源码
# 源码路径（相对CMakeLists路径）
set(SRC_ROOT "src") 
# 源码类型
set(SRCEXTS "*.c;*.cc;*.cpp")
set(HDREXTS "*.h;*.hh;*.hpp")
# 排除的文件和目录
#set(EXC_FILES "taskEngine/taskEngine.cc;log.cc")  # 例如"src/a.cpp;src/b.cpp", 相对与CMakeLists.txts所在位置的相对路径
set(EXC_FILES "taskEngine.cc;src/utils/log/log.cc")# 例如"xxx/yyy.cc;zzz.cc", 相对与CMakeLists.txts所在位置的相对路径
set(EXC_DIRS "src/utils/log")  		     # 例如"xxx/yyy;zzz", 相对与CMakeLists.txts所在位置的相对路径

# 1. cmake通用库
include (CMakeParseArguments)
#include(CMakeLists.template.txt)
# 获取文件列表
function(get_files_recurse files_out curdir filters) 
    list(TRANSFORM filters PREPEND "${curdir}/")
    file(GLOB_RECURSE local_files LIST_DIRECTORIES false RELATIVE ${curdir} ${filters})
    file(RELATIVE_PATH rel_path "${CMAKE_CURRENT_SOURCE_DIR}" "${curdir}")
    list(TRANSFORM local_files PREPEND "${rel_path}/")

    set(${files_out} ${local_files} PARENT_SCOPE)
endfunction()
 
# 获取目录列表（满足filters的文件列表）
function(get_dirs_recurse dirs_out curdir filters) 
    set(files "")
    
    # 调用函数并获取返回值
    get_files_recurse(files ${curdir} "${filters}")
    
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


# 从in_list里面剔除excls，生成out_list
function(list_exclution out_list in_list excls) 
    string(JOIN "|" reg_exp ${excls})
    list(FILTER in_list EXCLUDE REGEX "${reg_exp}")
    set(${out_list} ${in_list} PARENT_SCOPE)
endfunction()

function(files_exclusion)
    set(options "")                    # 无开关选项
    set(one_value_args "")             # 无单值参数 
    set(multi_value_args 
        list_var                                # 列表1（读写）
        exclusions                               # 列表2（只读）
    )
    
    cmake_parse_arguments(ARG 
        "${options}" 
        "${one_value_args}" 
        "${multi_value_args}" 
        ${ARGN}
    )

    # 获取列表内容
    set(local_list ${ARG_list_var})
    
    # 构建和执行过滤
    string(JOIN "|" pattern ${ARG_exclusions})
    #list(FILTER local_list EXCLUDE REGEX ".*/(${pattern})$")
    list(FILTER local_list EXCLUDE REGEX "(${pattern})$")
    #message(STATUS "pattern: ${pattern}")
    #message(STATUS "local_list: ${local_list}")
    # 原地修改：设置回原变量名
    #set(${list_var} ${local_list} PARENT_SCOPE)
    if("${ARG_list_var}" STREQUAL "${sources}")
	set(sources ${local_list} PARENT_SCOPE)
        #message(STATUS "Modified sources in-place")
    else()
        message(WARNING "files_exclusion: Cannot determine variable name for in-place modification")
    endif()
endfunction()

function(files_exclusion2 files_excl files filter)
    # 获取列表内容
    set(local_list ${files})
    
    # 构建和执行过滤
    string(JOIN "|" pattern ${filter})
    #list(FILTER local_list EXCLUDE REGEX ".*/(${pattern})$")
    list(FILTER local_list EXCLUDE REGEX "(${pattern})$")
    #message(STATUS "pattern: ${pattern}")
    #message(STATUS "local_list: ${local_list}")
    # 原地修改：设置回原变量名
    set(${files_excl} ${local_list} PARENT_SCOPE)
endfunction()

function(dirs_exclusion)
    set(options "")                    # 无开关选项
    set(one_value_args "")             # 无单值参数 
    set(multi_value_args 
        list_var                                # 列表1（读写）
        exclusions                               # 列表2（只读）
    )
    
    cmake_parse_arguments(ARG 
        "${options}" 
        "${one_value_args}" 
        "${multi_value_args}" 
        ${ARGN}
    )

    # 获取列表内容
    set(local_list ${ARG_list_var})
    
    # 构建和执行过滤
    string(JOIN "|" pattern ${ARG_exclusions})
    list(FILTER local_list EXCLUDE REGEX "^(${pattern})(/.*)?$")
    #list(FILTER local_list EXCLUDE REGEX "^(src/utils/log|src/utils/taskEngine)(/.*)?$")
    #message(STATUS "local_list: ${local_list}")
    # 原地修改：设置回原变量名
    if("${ARG_list_var}" STREQUAL "${header_dirs}")
	set(header_dirs ${local_list} PARENT_SCOPE)
        #message(STATUS "Modified sources in-place")
    else()
        message(WARNING "dirs_exclusion : Cannot determine variable name for in-place modification")
    endif()
endfunction()

function(dirs_exclusion2 dirs_excl dirs filter)
    # 获取列表内容
    set(local_list ${dirs})
    #message(STATUS "dirs_exclusion2 local_list: ${local_list}")
    #message(STATUS "dirs_exclusion2 in_dirs: ${in_dirs}")
    
    # 构建和执行过滤
    string(JOIN "|" pattern ${filter})
    #message(STATUS "dirs_exclusion2 pattern: ${pattern}")
    list(FILTER local_list EXCLUDE REGEX "^(${pattern})(/.*)?$")
    #list(FILTER local_list EXCLUDE REGEX "^(src/utils/log|src/utils/taskEngine)(/.*)?$")
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
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -Wno-pedantic -Wno-overloaded-virtual -Wno-return-type)
endif()

# 3. 创建可执行文件
# 可执行文件
get_files_recurse(sources "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_ROOT}" "${SRCEXTS}")
# 排除不需要的文件
dirs_exclusion2(sources_excl "${sources}" "${EXC_DIRS}")
#files_exclusion(sources_excl ${sources} exclusions ${EXC_FILES})
files_exclusion2(sources_excl2 "${sources_excl}" "${EXC_FILES}")
#message(STATUS "sources: ${sources_excl2}")
add_executable(${PROJECT_NAME} ${sources_excl2})


# 包含目录
get_dirs_recurse(header_dirs "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_ROOT}" "${HDREXTS}")
# 排除不需要的目录
#dirs_exclusion(list_var ${header_dirs} exclusions ${EXC_DIRS})
dirs_exclusion2(header_dirs_excl "${header_dirs}" "${EXC_DIRS}")
#message(STATUS "header_dirs_excl: ${header_dirs_excl}")
target_include_directories(${PROJECT_NAME} PRIVATE ${header_dirs_excl})

# 包含链接库
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



