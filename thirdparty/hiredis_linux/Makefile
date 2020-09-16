#./thirdparty/hiredis/Makefile
SOURCES = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)hiredis/,$(OBJS))

all:chkdir hiredis$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)hiredis/)

hiredis_static:$(OBJS)
	$(AR) $(OUT_DIR)hiredis.a $(OBJSD)

hiredis_shared:$(OBJS)
	$(CC) -shared -fPIC $(OBJSD) -o $(OUT_DIR)hiredis.so

$(OBJS):%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $(TMP_DIR)hiredis/$@

clean:
	rm -rf $(TMP_DIR)hiredis/*.o