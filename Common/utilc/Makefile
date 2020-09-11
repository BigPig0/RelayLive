#./Common/utilc/Makefile
SOURCES = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)utilc/,$(OBJS))

all:chkdir utilc$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)utilc/)

utilc_static:$(OBJS)
	$(AR) $(OUT_DIR)utilc.a $(OBJSD)

utilc_shared:$(OBJS)
	$(CC) -shared -fPIC $(OBJSD) -o $(OUT_DIR)utilc.so

$(OBJS):%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $(TMP_DIR)utilc/$@

clean:
	rm -rf $(TMP_DIR)utilc/*.o