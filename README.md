## 01_Makefile: 开箱即用Makefile  
Generic Makefile for C/C++ Program  
### 1、下载通用makefile
将本文件下载后，放置到代码的某一目录下，推荐单独创建一个build目录，并将本文件放置其中；
### 2、按需定制修改makefile
- SRC_ROOT：源文件根目录(相对路径或绝对路径均可，如果是相对路径指的是相对本文件所在目录的路径)
- OBJ: 目标文件放置路径(相对路径或绝对路径均可）
- BIN: 二进制可执行文件放置路径
- EXT_DIR: 外部头文件搜索路径（按需）
- CPPLFLAGS/CFLAGS/LDFLAGS：差异化编译链接选项（按需）
### 3、启动编译：  
```shell
> cd build //假设makefile放置中build目录下
> make
```

--- 
## 02_CMakeLists:开箱即用CMakeLists
通用 CMakeLists.txt，按需定制如下配置即可开展 cmake 构建  
### 1、下载通用CMakeLists
将本文件下载后，放置到代码的某一目录下，推荐单独创建一个build目录，并将本文件放置其中；
### 2、按需定制修改CMakeLists
- MY_PROJECT/MY_PROJECT_DESCRIPTION：项目名称及描述
- MY_BIN：目标文件名称
- SRC_ROOT：源文件根目录
- SRCEXTS/HDREXTS：源码类型
- EXC_FILES：需要排除不参与编译的源文件列表
- EXC_DIRS：需要排除的源文件目录（做SRC_ROOT之下的子目录）
- EXT_LIBS（暂未实现）
- CPPFLAGS/CFLAGS：编译选项
- LDFLAGS：链接选项
### 3、启动编译
```shell
> cd build //假设CMakeLists.txt放置中build的上一级目录下
> cmake ..
> make
```
