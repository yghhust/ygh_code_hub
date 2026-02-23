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
# ================================================================================================

cmake_minimum_required(VERSION 3.10)
project(MyApp)

# ==================== 1. 项目配置 ====================

# 项目名称
set(MY_PROJECT "MyApp")
set(MY_BIN "a.out")

# 源码根目录(相对CMakeList.txt所在目录路径)
set(SRC_DIR ".")

# 要排除的目录（相对 SRC_DIR 的路径）
# 注意：这里的路径是相对于 SRC_DIR 的，不包含开头的 "./"
set(EXCLUDE_DIRS 
    "build"              # 排除 ./build 目录
    "module_a/legacy"    # 排除 ./module_a/legacy 目录
    "module_b/test"      # 排除 ./module_b/test 目录
)

# 要排除的文件（相对 SRC_DIR 的路径）
set(EXCLUDE_FILES 
    "CMakeCXXCompilerId.cpp"
    "CMakeCCompilerId.c"
)

# ==================== 2. 文件后缀配置 ====================

# 源文件后缀
set(SRC_EXTENSIONS 
    "*.cpp" "*.cc" "*.cp" "*.cxx" "*.c" "*.C"
)

# 头文件后缀
set(HDR_EXTENSIONS 
    "*.h" "*.hh" "*.hpp" "*.hxx" "*.H"
)

# ==================== 3. 编译选项配置 ====================

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

# ==================== 4. 链接库配置 ====================

# 需要链接的库（库名或全路径，用分号分隔）
set(LIBS
    "pthread"
    # "m"           # 数学库
    # "dl"          # 动态链接库
    # "rt"          # 实时库
)

# ==================== 5. 获取源文件 ====================

# 将 SRC_DIR 转换为绝对路径
#get_filename_component(SRC_DIR_ABS "${SRC_DIR}" ABSOLUTE)

# 递归获取所有源文件
set(ALL_SOURCES "")
foreach(ext IN LISTS SRC_EXTENSIONS)
    file(GLOB_RECURSE files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SRC_DIR}/${ext}")
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

# ==================== 6. 获取头文件目录 ====================

# 递归获取 SRC_DIR下面的所有目录（包括子目录）
file(GLOB_RECURSE ALL_DIRS LIST_DIRECTORIES true RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${SRC_DIR}/*")

# 只保留真正的目录，并应用排除规则
set(ALL_INCLUDES "")
foreach(d IN LISTS ALL_DIRS)
        # 注意需要使用绝对路径进行判断！
	if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${d}")
	    continue()
	endif()

	# 计算相对于 SRC_DIR_ABS 的路径
	#file(RELATIVE_PATH rel_d "${SRC_DIR_ABS}" "${d}")
	
	# 检查是否在排除目录列表中
	set(excluded FALSE)
	foreach(excl_dir IN LISTS EXCLUDE_DIRS)
	    if("${d}" MATCHES "^${excl_dir}(/.*)?$")
		set(excluded TRUE)
		break()
	    endif()
	endforeach()
	
	if(NOT excluded)
	    list(APPEND ALL_INCLUDES "${d}")
	endif()
endforeach()

# 追加自身目录
list(APPEND ALL_INCLUDES "${SRC_DIR}")

# 去重
list(REMOVE_DUPLICATES ALL_INCLUDES)

# ==================== 7. 创建可执行文件 ====================

add_executable(${MY_BIN} ${ALL_SOURCES})
target_include_directories(${MY_BIN} PRIVATE ${ALL_INCLUDES})

# 设置编译选项
target_compile_options(${MY_BIN} PRIVATE 
    ${COMMON_WARNINGS}
    $<$<COMPILE_LANGUAGE:C>:${CFLAGS}>
    $<$<COMPILE_LANGUAGE:CXX>:${CXXFLAGS}>
)

# 设置链接选项
target_link_options(${MY_BIN} PRIVATE ${LDFLAGS})

# 链接库
target_link_libraries(${MY_BIN} PRIVATE ${LIBS})

# ==================== 8. 输出信息 ====================

message(STATUS "========================================")
message(STATUS "Build Configuration:")
message(STATUS "  Project: ${MY_PROJECT}")
message(STATUS "  Binary:  ${MY_BIN}")
message(STATUS "  Source Dir: ${SRC_DIR}")
message(STATUS "  Sources: ${ALL_SOURCES}")
message(STATUS "  Include Dirs: ${ALL_INCLUDES}")
message(STATUS "  Excluded Dirs: ${EXCLUDE_DIRS}")
message(STATUS "  Excluded Files: ${EXCLUDE_FILES}")
message(STATUS "  Libraries: ${LIBS}")
message(STATUS "========================================")
