#./ThirdParty/cjson/Makefile
SOURCES = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)cjson/,$(OBJS))

all:chkdir cjson$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)cjson/)

cjson_static:$(OBJS)
	$(AR) $(OUT_DIR)cjson.a $(OBJSD)

cjson_shared:$(OBJS)
	$(CC) -shared -fPIC $(OBJSD) -o $(OUT_DIR)cjson.so

$(OBJS):%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $(TMP_DIR)cjson/$@

clean:
	rm -rf $(TMP_DIR)cjson/*.o