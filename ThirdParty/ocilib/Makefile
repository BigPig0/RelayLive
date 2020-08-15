#./ThirdParty/ocilib/Makefile
SRC_DIR = src/
SOURCES = $(wildcard src/*.c)
OBJS = $(patsubst %.c,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)ocilib/,$(OBJS))

INCLUDE = -I ./include -I ./src

all:chkdir ocilib$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)ocilib/)

ocilib_static:$(OBJS)
	$(AR) $(OUT_DIR)ocilib.a $(OBJSD)

ocilib_shared:$(OBJS) $(UNIX_OBJS)
	$(CC) -shared -fPIC $(OBJSD) -o $(OUT_DIR)ocilib.so

$(OBJS):%.o:$(SRC_DIR)%.c
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $(TMP_DIR)ocilib/$@

clean:
	rm -rf $(TMP_DIR)ocilib/*.o
