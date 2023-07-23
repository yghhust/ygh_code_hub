# ====================================================================
# Generic Makefile for C/C++ Program  
#  
# License: GPL (General Public License)  
# Author:  yuguohua <ghy_hust@qq.com>  
# Date:    2023/07/15 (version 0.1)  initial
#          2023/07/15 (version 0.2)  set path for object and bin file  
#          2023/07/22 (version 0.3)  set path for external header file 
#
# Description:  
# ------------ 
# 通用Makefile, 按需设置SRC_ROOT/OBJ_DIR/CPPFLAGS/CFLAGS/LDFLAGS即可
# 必要时，还可以设置EXT_DIR制定外部头文件搜索路径
# ====================================================================

# SHELL = /bin/bash

##1. Customizable Section: adapt those variables to suit your program.  
# =======================================================================
BIN 		:= a.out 

# 源码根目录
SRC_ROOT	:= ../ 	

# 目标文件目录
OBJ_DIR	:= obj
#OBJ_DIR	:= .

# 库文件列表及目录
LIBRARY	:= -L./../lib
#LIBRARY	:= -L./../lib -L/usr/local/lib
LIBRARY	+= -lavrocpp_s -rdynamic -Wl,-rpath,/usr/local/lib/libarrow.so.1300.0.0

# 宏定义
MACRO		:= -DDEBUG

# 外部头文件目录列表（多个目录之间通过空格隔开）
EXT_DIR := 
#EXT_DIR := ~/study/third/avro/lang/c++/api /usr/local

## 2. Implicit Section: change the following only when necessary. 
# =======================================================================
# 编译选项（注意使用=号赋值）
CPPFLAGS	= $(INCLUDE) $(MACRO)
CFLAGS		= -g -Wall -O2 
CXXFLAGS	= -g -Wall -O2 -std=gnu++1z
LDFLAGS		= $(LIBRARY)

# 源码文件后缀
SRCEXTS 	:= .c .C .cc .cpp .CPP .c++ .cxx .cp  
HDREXTS 	:= .h .H .hh .hpp .HPP .h++ .hxx .hp  

# 排除的目录
EXCL_DIR	:= ".vscode\|.svn\|.git"

# 编译器
CC		:= gcc
CXX 		:= g++

## 3. Stable Section: usually no need to be changed. But you can add more. 
# =======================================================================
# 参数检查
check_param = $(if $(strip $($1)),,$(error $1 is not set))
$(call check_param,BIN) 
$(call check_param,SRC_ROOT) 
$(call check_param,OBJ_DIR) 

DIRS 		:= $(shell find $(SRC_ROOT) -type d |grep -v $(EXCL_DIR))
FILES 		:= $(foreach d, $(DIRS), $(shell find $(d) -maxdepth 1 -type f))

# 源文件列表及目录
SRC_FILE	:= $(foreach n, $(SRCEXTS), $(filter %$(n), $(FILES)))
SRC_DIR	:= $(sort $(dir $(SRC_FILE)))
# 头文件列表、目录及外部目录
HDR_FILE	:= $(foreach n, $(HDREXTS), $(filter %$(n), $(FILES)))
HDR_DIR	:= $(sort $(dir $(HDR_FILE)))
EXT_DIR 	:= $(foreach n, $(EXT_DIR), $(shell find $(EXT_DIR) -type d |grep -v $(EXCL_DIR)))

# 目标文件列表
#OBJS    	:= $(subst $(SRC_ROOT),$(OBJ_DIR), $(addsuffix .o, $(basename $(SRC_FILE))))
OBJS    	:= $(addsuffix .o, $(basename $(SRC_FILE)))
OBJS			:= $(addprefix $(OBJ_DIR)/, $(notdir $(OBJS)))

# 外部头文件目录
INCLUDE		+= $(foreach n, $(HDR_DIR), -I$(n))
INCLUDE		+= $(foreach n, $(EXT_DIR), -I$(n))

# 设置搜索路径
#define add_vpath
#  vpath %$1 %$2
#endef
#$(foreach d, $(SRC_DIR), $(foreach ext, $(SRCEXTS), $(eval $(call add_vpath, $(ext) $(d)))))
#$(foreach d, $(HDR_DIR), $(foreach ext, $(HDREXTS), $(eval $(call add_vpath, $(ext) $(d)))))
VPATH	:= $(DIRS) 

# 编译
all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)
	@echo Type ./$@ to execute the program.

objs:$(OBJS)  

$(OBJS): | $(OBJ_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)
    
$(OBJ_DIR)/%.o: %.c $(HDR_FILE)
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS)
$(OBJ_DIR)/%.o: %.C $(HDR_FILE)
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS)
$(OBJ_DIR)/%.o: %.cc $(HDR_FILE)
	$(CXX) -c $< -o $@ $(CPPFLAGS) $(CXXFLAGS)
$(OBJ_DIR)/%.o: %.cpp $(HDR_FILE)
	$(CXX) -c $< -o $@ $(CPPFLAGS) $(CXXFLAGS)
$(OBJ_DIR)/%.o: %.CPP $(HDR_FILE)
	$(CXX) -c $< -o $@ $(CPPFLAGS) $(CXXFLAGS)
$(OBJ_DIR)/%.o: %.cxx $(HDR_FILE)
	$(CXX) -c $< -o $@ $(CPPFLAGS) $(CXXFLAGS)
$(OBJ_DIR)/%.o: %.cp $(HDR_FILE)
	$(CXX) -c $< -o $@ $(CPPFLAGS) $(CXXFLAGS)

.PHONY:	clean

clean:
	rm -f $(BIN) $(OBJ_DIR)/*.o

# Makefile帮助与调试
# Show help. 
help:  
	@echo 'Generic Makefile for C/C++ Programs (gcmakefile) '  
	@echo 'Copyright (C) 2023 ghy_hust <ghy_hust@qq.com>'  
	@echo  
	@echo 'Usage: make [TARGET]'  
	@echo 'TARGETS:'  
	@echo '  all       (=make) compile and link.'  
	@echo '  objs      compile only (no linking).'   
	@echo '  clean     clean objects and the executable file.'  
	@echo '  show      show variables (for debug use only).'  
	@echo '  help      print this message.'  
	@echo  
	@echo 'Report bugs to <ghy_hust@qq.com>.'  
	
# Show variables (for debug use only.)  
show:
	@echo  'CPPFLAGS: $(CPPFLAGS)'
	@echo  'LDFLAGS: $(LDFLAGS)'
	@echo  'DIRS: $(DIRS)'
	@echo  'FILES: $(FILES)'
	@echo  'SRC_FILE: $(SRC_FILE)'
	@echo  'SRC_DIR: $(SRC_DIR)'
	@echo  'HDR_FILE: $(HDR_FILE)'
	@echo  'HDR_DIR: $(HDR_DIR)'
	@echo  'EXT_DIR: $(EXT_DIR)'
	@echo  'INCLUDE: $(INCLUDE)'
	@echo  'OBJS: $(OBJS)'
