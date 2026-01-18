# ====================================================================
# Generic CMakeLists.txt for C/C++ Program  
#  
# License: GPL (General Public License)  
# Author:  yuguohua <ghy_hust@qq.com>  
# Date:    2026/01/19 (version 0.1)  initial
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
# 0.1.设置源码
# 源码路径（相对CMakeLists路径）
set(SRC_ROOT "src") 
# set(INSTALL_DIR "bin")
# 源码类型
set(SRCEXTS "*.c;*.cc;*.cpp")
set(HDREXTS "*.h;*.hh;*.hpp")

# 1. cmake通用库
#include(CMakeLists.template.txt)
# 获取文件列表
function(get_files_recurse files_out curdir filters)
    set(globbing_exp "")
    foreach(item ${filters})
        message(STATUS "[get_filslist_recurse]item: ${item}")
        list(APPEND globbing_exp "${curdir}/${item}")
    endforeach()
    #message(STATUS "[get_filslist_recurse]globbing_exp: ${globbing_exp}")
    
    #file(GLOB_RECURSE local_files LIST_DIRECTORIES false RELATIVE ${curdir} ${globbing_exp})
    file(GLOB_RECURSE local_files LIST_DIRECTORIES false ${globbing_exp})
    #file(GLOB_RECURSE local_files LIST_DIRECTORIES false ${filters})
    #message(STATUS "[get_filslist_recurse]local_files: ${local_files}")

    set(${files_out} ${local_files} PARENT_SCOPE)
endfunction()

function(get_files_recurse2 files_out curdir filters)    
    file(GLOB_RECURSE local_files LIST_DIRECTORIES false *)
    list(FILTER local_files INCLUDE REGEX ${filters})
    message(STATUS "[get_filslist_recurse]local_files: ${local_files}")

    set(${files_out} ${local_files} PARENT_SCOPE)
endfunction()

# 获取目录列表
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
#list(FILTER sources EXCLUDE REGEX ".cpp|.cc")
#get_files_recurse2(sources "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_ROOT}" ".*\\.(cpp|cxx|cc)$")
#get_files_recurse2(sources "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_ROOT}" "(.cpp)|(.cxx)|(.cc)")

message(STATUS "xxxx: ${sources}")
# 排除特定模式的文件


#list(FILTER SOURCES EXCLUDE REGEX ".*_perf\\.cpp$")
#list(FILTER SOURCES EXCLUDE REGEX ".*/experimental/.*")
add_executable(${PROJECT_NAME} ${sources})


# 包含目录
get_dirs_recurse(header_dirs "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_ROOT}" "${HDREXTS}")
target_include_directories(${PROJECT_NAME} PRIVATE
    ${header_dirs}
)

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



