#./projects/ipc_server/Makefile
SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)ipc_server/,$(OBJS))

INCLUDE = -I $(PWD)/thirdparty/libuv/include \
		  -I $(PWD)/common/utilc \
		  -I $(PWD)/common/util \
		  -I $(PWD)/common/pm \
		  -I $(PWD)/uvmodules/uvipc \
          -I $(PWD)/uvmodules/uvlogplus \

all:chkdir ipc_server$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)ipc_server/)

ipc_server_static:$(OBJS)
	$(GG) $(OBJSD) -o $(OUT_DIR)ipc_server -L $(OUT_DIR) -l:uvipc.a -l:uvlogplus.a -l:util.a -l:utilc.a -l:pugixml.a -l:cjson.a -l:libuv.a -l:zlib.a -lrt -lresolv -lssl -lcrypto -pthread -ldl

ipc_server_shared:$(OBJS)
	$(GG) $(OBJSD) -o $(OUT_DIR)ipc_server -L $(OUT_DIR) -l:libuv.so -l:util.so -l:utilc.so -l:uvipc.so -l:uvlogplus.so -l:pugixml.so -l:cjson.so -pthread -ldl

$(OBJS):%.o:%.cpp
	$(GG) $(INCLUDE) $(GFLAGS) -c $< -o $(TMP_DIR)ipc_server/$@

clean:
	rm -rf $(TMP_DIR)ipc_server/*.o