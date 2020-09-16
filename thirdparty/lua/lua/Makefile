#./thirdparty/lua/lua/Makefile
SOURCES = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)lua/,$(OBJS))

all:chkdir lua$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)lua/)

lua_static:$(OBJS)
	$(AR) $(OUT_DIR)lua.a $(OBJSD)

lua_shared:$(OBJS)
	$(CC) -shared -fPIC $(OBJSD) -o $(OUT_DIR)lua.so

$(OBJS):%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $(TMP_DIR)lua/$@

clean:
	rm -rf $(TMP_DIR)lua/*.o