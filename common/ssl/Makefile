#./common/ssl/Makefile
SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)ssl/,$(OBJS))

all:chkdir ssl$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)ssl/)

ssl_static:$(OBJS)
	$(AR) $(OUT_DIR)ssl.a $(OBJSD)

ssl_shared:$(OBJS)
	$(GG) -shared -fPIC $(OBJSD) -o $(OUT_DIR)ssl.so

$(OBJS):%.o:%.cpp
	$(GG) $(GFLAGS) -c $< -o $(TMP_DIR)ssl/$@

clean:
	rm -rf $(TMP_DIR)ssl/*.o