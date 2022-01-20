#./projects/hikisup_sdk/Makefile
SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)hikisup_sdk/,$(OBJS))

INCLUDE = -I $(PWD)/thirdparty/libuv/include \
          -I $(PWD)/thirdparty/hiksdk/include \
		  -I $(PWD)/common/utilc \
		  -I $(PWD)/common/util \
		  -I $(PWD)/common/crypto \
		  -I $(PWD)/common/ludb \
		  -I $(PWD)/uvmodules/uvipc \
          -I $(PWD)/uvmodules/uvlogplus \
		  -I $(PWD)/uvmodules/uvnetplus \
		  -I $(PWD)/projects/worker \
		  -I /usr/local/include

all:chkdir hikisup_sdk$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)hikisup_sdk/)

hikisup_sdk_static:$(OBJS)
	$(GG) $(OBJSD) -o $(OUT_DIR)hikisup_sdk -L $(OUT_DIR) -l:worker.a -l:uvipc.a -l:uvnetplus.a -l:uvlogplus.a -l:ludb.a -l:util.a -l:utilc.a -l:crypto.a -l:pugixml.a -l:cjson.a -l:libuv.a -l:mbedtls.a -l:libmariadb.a -l:ocilib.a -l:sqlite3.a -l:zlib.a -l:lua.a \
	-lHCISUPCMS -lHCISUPAlarm -lHCISUPStream \
	-L /usr/local/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswscale -lswresample -lrt -lresolv -lssl -lcrypto -pthread -ldl

hikisup_sdk_shared:$(OBJS)
	$(GG) $(OBJSD) -o $(OUT_DIR)hikisup_sdk -L $(OUT_DIR) -l:libuv.so -l:mbedtls.so -l:libmariadb.so -l:ocilib.so -l:sqlite3.so -l:zlib.so -l:lua.so -l:ludb.ao -l:util.so -l:utilc.so -l:uvipc.so -l:uvnetplus.so -l:uvlogplus.so -l:pugixml.so -l:cjson.so -pthread -ldl

$(OBJS):%.o:%.cpp
	$(GG) $(INCLUDE) $(GFLAGS) -c $< -o $(TMP_DIR)hikisup_sdk/$@

clean:
	rm -rf $(TMP_DIR)hikisup_sdk/*.o