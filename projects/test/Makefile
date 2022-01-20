#./projects/test/Makefile
SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)test/,$(OBJS))

INCLUDE = -I $(PWD)/thirdparty/libuv/include \
		  -I $(PWD)/common/utilc \
		  -I $(PWD)/common/util \
		  -I $(PWD)/common/crypto \
		  -I $(PWD)/common/pm \
		  -I $(PWD)/uvmodules/uvipc \
          -I $(PWD)/uvmodules/uvlogplus \

all:chkdir test$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)test/)

test_static:$(OBJS)
	$(GG) $(OBJSD) -o $(OUT_DIR)test -L $(OUT_DIR) -l:uvipc.a -l:uvlogplus.a -l:util.a -l:utilc.a -l:crypto.a -l:pugixml.a -l:cjson.a -l:libuv.a -l:zlib.a -lrt -lresolv -lssl -lcrypto -pthread -ldl

test_shared:$(OBJS)
	$(GG) $(OBJSD) -o $(OUT_DIR)test -L $(OUT_DIR) -l:libuv.so -l:util.so -l:utilc.so -l:uvipc.so -l:uvlogplus.so -l:pugixml.so -l:cjson.so -pthread -ldl

$(OBJS):%.o:%.cpp
	$(GG) $(INCLUDE) $(GFLAGS) -c $< -o $(TMP_DIR)test/$@

clean:
	rm -rf $(TMP_DIR)test/*.o