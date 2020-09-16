#./thirdparty/libcstl/Makefile
SOURCES = $(wildcard src/*.c)
OBJS = $(patsubst %.c,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)libcstl/,$(OBJS))

INCLUDE = -I $(PWD)/thirdparty/libcstl

all:chkdir libcstl$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)libcstl/)

libcstl_static:$(OBJS)
	$(AR) $(OUT_DIR)libcstl.a $(OBJSD)

libcstl_shared:$(OBJS)
	$(CC) -shared -fPIC $(OBJSD) -o $(OUT_DIR)libcstl.so

$(OBJS):%.o:src/%.c
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $(TMP_DIR)libcstl/$@

clean:
	rm -rf $(TMP_DIR)libcstl/*.o
