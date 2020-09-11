#./Common/util/Makefile
SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)util/,$(OBJS))

all:chkdir util$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)util/)

util_static:$(OBJS)
	$(AR) $(OUT_DIR)util.a $(OBJSD)

util_shared:$(OBJS)
	$(GG) -shared -fPIC $(OBJSD) -o $(OUT_DIR)util.so

$(OBJS):%.o:%.c
	$(GG) $(GFLAGS) -c $< -o $(TMP_DIR)util/$@

clean:
	rm -rf $(TMP_DIR)util/*.o