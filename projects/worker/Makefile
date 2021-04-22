#./common/worker/Makefile
SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)worker/,$(OBJS))

INCLUDE = -I $(PWD)/thirdparty/libuv/include \
		  -I $(PWD)/common/utilc \
		  -I $(PWD)/common/util \
          -I $(PWD)/common/crypto \
		  -I $(PWD)/uvmodules/uvipc \
          -I $(PWD)/uvmodules/uvlogplus \
		  -I /usr/local/include

all:chkdir worker$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)worker/)

worker_static:$(OBJS)
	$(AR) $(OUT_DIR)worker.a $(OBJSD)

worker_shared:$(OBJS)
	$(GG) -shared -fPIC $(OBJSD) -o $(OUT_DIR)worker.so

$(OBJS):%.o:%.cpp
	$(GG) $(INCLUDE) $(GFLAGS) -c $< -o $(TMP_DIR)worker/$@

clean:
	rm -rf $(TMP_DIR)worker/*.o