#./common/pm/Makefile
SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)pm/,$(OBJS))

INCLUDE = -I $(PWD)/thirdparty/libuv/include -I $(PWD)/thirdparty/cjson -I $(PWD)/common/util -I $(PWD)/common/utilc

all:chkdir pm$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)pm/)

pm_static:$(OBJS)
	$(AR) $(OUT_DIR)pm.a $(OBJSD)

pm_shared:$(OBJS)
	$(GG) -shared -fPIC $(OBJSD) -o $(OUT_DIR)pm.so

$(OBJS):%.o:%.cpp
	$(GG) $(INCLUDE) $(GFLAGS) -c $< -o $(TMP_DIR)pm/$@

clean:
	rm -rf $(TMP_DIR)pm/*.o