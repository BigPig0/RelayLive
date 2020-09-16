#配置编译参数
DEBUG = 1
SHARED = 0
BITS64 = 1

CC = gcc
GG = g++
AR = ar rc
CFLAGS = -fPIC -Wall -std=gnu11
GFLAGS = -fPIC -Wall -std=c++11
LFLAGS = 
OUT_DIR = $(PWD)/out/
TMP_DIR = $(PWD)/out/tmp/

ifeq ($(BITS64),0)
	CFLAGS += -m32
	GFLAGS += -m32
	LFLAGS += -m32
	OUT_DIR = $(PWD)/out/linux32/
	TMP_DIR = $(PWD)/out/tmp/linux32/
else
	CFLAGS += -m64
	GFLAGS += -m64
	LFLAGS += -m64
	OUT_DIR = $(PWD)/out/linux64/
	TMP_DIR = $(PWD)/out/tmp/linux64/
endif

ifeq ($(DEBUG),0)
	#release
	CFLAGS += -O -DNDEBUG
	GFLAGS += -O -DNDEBUG
else
	CFLAGS += -g
	GFLAGS += -g
endif

ifeq ($(SHARED),0)
	#static
	CFLAGS += -static
	LFLAGS += -static
	TAGTYPE = _static
else
	TAGTYPE = _shared
endif

export CC GG AR CFLAGS GFLAGS LFLAGS OUT_DIR TMP_DIR TAGTYPE

#创建输出目录
$(shell mkdir -p $(OUT_DIR))

SUBDIR =  uvmodules#thirdparty common uvmodules projects

all:$(SUBDIR)
	#
$(SUBDIR):ECHO
	make -C $@

ECHO:  
	@echo $@

.PHONY:clean
clean:CLEANDIR
CLEANDIR:ECHO
	make -C $(SUBDIR) clean