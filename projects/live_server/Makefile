#./projects/live_server/Makefile
SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)live_server/,$(OBJS))

INCLUDE = -I $(PWD)/thirdparty/libuv/include \
		  -I $(PWD)/common/utilc \
		  -I $(PWD)/common/util \
		  -I $(PWD)/common/crypto \
		  -I $(PWD)/uvmodules/uvipc \
          -I $(PWD)/uvmodules/uvlogplus \
		  -I $(PWD)/projects/worker \
		  -I /usr/local/include

all:chkdir live_server$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)live_server/)

live_server_static:$(OBJS)
	$(GG) $(OBJSD) -o $(OUT_DIR)live_server -L $(OUT_DIR) -l:worker.a -l:uvipc.a -l:uvlogplus.a -l:util.a -l:utilc.a -l:crypto.a -l:pugixml.a -l:cjson.a -l:libuv.a -l:zlib.a \
	-L /usr/local/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswscale -lswresample -lrt -lresolv -lssl -lcrypto -pthread -ldl

live_server_shared:$(OBJS)
	$(GG) $(OBJSD) -o $(OUT_DIR)live_server -L $(OUT_DIR) -l:libuv.so -l:util.so -l:utilc.so -l:uvipc.so -l:uvlogplus.so -l:pugixml.so -l:cjson.so -pthread -ldl

$(OBJS):%.o:%.cpp
	$(GG) $(INCLUDE) $(GFLAGS) -c $< -o $(TMP_DIR)live_server/$@

clean:
	rm -rf $(TMP_DIR)live_server/*.o