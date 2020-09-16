#./Common/uvnetplus/Makefile
SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)uvnetplus/,$(OBJS))

INCLUDE = -I $(PWD)/thirdparty/libuv/include -I $(PWD)/uvmoudles/uvlogplus -I $(PWD)/common/utilc -I $(PWD)/common/util

all:chkdir uvnetplus$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)uvnetplus/)

uvnetplus_static:$(OBJS)
	$(AR) $(OUT_DIR)uvnetplus.a $(OBJSD)

uvnetplus_shared:$(OBJS)
	$(GG) -shared -fPIC $(OBJSD) -o $(OUT_DIR)uvnetplus.so

$(OBJS):%.o:%.cpp
	$(GG) $(INCLUDE) $(GFLAGS) -c $< -o $(TMP_DIR)uvnetplus/$@

clean:
	rm -rf $(TMP_DIR)uvnetplus/*.o