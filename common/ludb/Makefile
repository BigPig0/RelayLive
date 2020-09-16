#./common/ludb/Makefile
SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)ludb/,$(OBJS))

INCLUDE = -I $(PWD)/thirdparty/ocilib/include
INCLUDE += -I $(PWD)/thirdparty/libmongoc
INCLUDE += -I $(PWD)/thirdparty/lua
INCLUDE += -I $(PWD)/thirdparty/lua/lua
INCLUDE += -I $(PWD)/thirdparty/libuv/include
INCLUDE += -I $(PWD)/thirdparty/hiredis_linux
INCLUDE += -I $(PWD)/common/util
INCLUDE += -I $(PWD)/common/utilc

all:chkdir ludb$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)ludb/)

ludb_static:$(OBJS)
	$(AR) $(OUT_DIR)ludb.a $(OBJSD)

ludb_shared:$(OBJS)
	$(GG) -shared -fPIC $(OBJSD) -o $(OUT_DIR)ludb.so

$(OBJS):%.o:%.cpp
	$(GG) $(INCLUDE) $(GFLAGS) -c $< -o $(TMP_DIR)ludb/$@

clean:
	rm -rf $(TMP_DIR)ludb/*.o