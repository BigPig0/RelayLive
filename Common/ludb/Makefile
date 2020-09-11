#./Common/ludb/Makefile
SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)ludb/,$(OBJS))

INCLUDE = -I $(PWD)/ThirdParty/ocilib/include
INCLUDE += -I $(PWD)/ThirdParty/libmongoc
INCLUDE += -I $(PWD)/ThirdParty/lua
INCLUDE += -I $(PWD)/ThirdParty/lua/lua
INCLUDE += -I $(PWD)/ThirdParty/libuv/include
INCLUDE += -I $(PWD)/ThirdParty/hiredis_linux
INCLUDE += -I $(PWD)/Common/util
INCLUDE += -I $(PWD)/Common/utilc

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