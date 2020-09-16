#./common/uvlogplus/Makefile
SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)uvlogplus/,$(OBJS))

INCLUDE = -I $(PWD)/thirdparty/libuv/include -I $(PWD)/thirdparty/pugixml -I $(PWD)/thirdparty/cjson -I $(PWD)/common/utilc -I $(PWD)/common/util

all:chkdir uvlogplus$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)uvlogplus/)

uvlogplus_static:$(OBJS)
	$(AR) $(OUT_DIR)uvlogplus.a $(OBJSD)

uvlogplus_shared:$(OBJS)
	$(GG) -shared -fPIC $(OBJSD) -o $(OUT_DIR)uvlogplus.so

$(OBJS):%.o:%.cpp
	$(GG) $(INCLUDE) $(GFLAGS) -c $< -o $(TMP_DIR)uvlogplus/$@

clean:
	rm -rf $(TMP_DIR)uvlogplus/*.o