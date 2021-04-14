#./projects/sip_server/Makefile
SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)sip_server/,$(OBJS))

INCLUDE = -I $(PWD)/thirdparty/libuv/include \
		  -I $(PWD)/thirdparty/lua \
		  -I $(PWD)/thirdparty/lua/lua \
		  -I $(PWD)/thirdparty/exosip \
		  -I $(PWD)/thirdparty/pugixml \
		  -I $(PWD)/thirdparty/cjson \
		  -I $(PWD)/common/utilc \
		  -I $(PWD)/common/util \
		  -I $(PWD)/common/ludb \
		  -I $(PWD)/uvmodules/uvipc \
          -I $(PWD)/uvmodules/uvlogplus \

all:chkdir sip_server$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)sip_server/)

sip_server_static:$(OBJS)
	$(GG) $(OBJSD) -o $(OUT_DIR)sip_server -L $(OUT_DIR) -l:ludb.a -l:uvipc.a -l:uvlogplus.a -l:util.a -l:uvlogplus.a -l:utilc.a -l:libuv.a -l:exosip.a -l:hiredis.a -l:lua.a -l:cjson.a -l:pugixml.a -l:libmariadb.a -l:ocilib.a -l:sqlite3.a -l:zlib.a -lrt -lresolv -lssl -lcrypto -pthread -ldl

sip_server_shared:$(OBJS)
	$(GG) $(OBJSD) -o $(OUT_DIR)sip_server -L $(OUT_DIR) -l:libuv.so -l:exosip.so -l:hiredis.so -l:lua.so -l:cjson.so -l:pugixml.so -l:util.so -l:utilc.so -l:ludb.so -l:uvipc.so -l:uvlogplus.so -pthread -ldl

$(OBJS):%.o:%.cpp
	$(GG) $(INCLUDE) $(GFLAGS) -c $< -o $(TMP_DIR)sip_server/$@

clean:
	rm -rf $(TMP_DIR)sip_server/*.o