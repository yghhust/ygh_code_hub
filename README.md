# 01_Makefile: 开箱即用Makefile
------------------------------------------------------------------ 
Generic Makefile for C/C++ Program
1、拷贝本文件至代码中某以目录；
2、打开本文件，设定源码路径(SRC_ROOT),相对路径或绝对路径均可，如果是相对路径指的是相对本文件所在目录的路径；
3、按需定制设置OBJ_DIR/CPPFLAGS/CFLAGS/LDFLAGS，设置项目差异化选项；
4、按需设置EXT_DIR制定外部头文件搜索路径
5、打开shell终端，在本文件所在路径下运行make，即可开始编译构建


# 02_CMakeLists:开箱即用
------------------------------------------------------------------ 
通用CMakeLists.txt, 按需定制如下配置即可开展cmake构建：
1）项目及目标文件设定
   MY_PROJECT/MY_PROJECT_DESCRIPTION: 项目名称及描述
   MY_BIN: 目标文件名称
2）源码路径设定
   SRC_ROOT: 源文件根目录
   SRCEXTS/HDREXTS: 源码类型
   EXT_FILS: 设置需要正则表达
   EXT_DIRS: 必要时，还可以设置EXT_DIR制定外部头文件搜索路径
3）lib库设定
   EXT_LIBS（暂未实现）
4) 编译链接选项设定
   CPPFLAGS/CFLAGS：编译选项（暂未实现）
   LDFLAGS：链接选项（暂未实现）
