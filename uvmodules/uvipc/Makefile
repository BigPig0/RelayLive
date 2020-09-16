#./thirdParty/uvipc/Makefile
SOURCES = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)uvipc/,$(OBJS))

INCLUDE = -I $(PWD)/thirdparty/libuv/include -I $(PWD)/common/utilc

all:chkdir uvipc$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)uvipc/)

uvipc_static:$(OBJS)
	$(AR) $(OUT_DIR)uvipc.a $(OBJSD)

uvipc_shared:$(OBJS)
	$(CC) -shared -fPIC $(OBJSD) -o $(OUT_DIR)uvipc.so

$(OBJS):%.o:%.c
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $(TMP_DIR)uvipc/$@

clean:
	rm -rf $(TMP_DIR)uvipc/*.o