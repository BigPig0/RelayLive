#./ThirdParty/libuv/Makefile
SRC_DIR = src/
SOURCES =   fs-poll.c \
            inet.c \
            threadpool.c \
            uv-data-getter-setters.c \
            uv-common.c \
            version.c
OBJS = $(patsubst %.c,%.o,$(SOURCES))
OBJSD = $(addprefix $(TMP_DIR)libuv/,$(OBJS))

SRC_UNIX_DIR = src/unix/
UNIX_SOURCES = async.c \
             core.c \
             dl.c \
             fs.c \
             getaddrinfo.c \
             getnameinfo.c \
             loop-watcher.c \
             loop.c \
             pipe.c \
             poll.c \
             process.c \
             signal.c \
             stream.c \
             tcp.c \
             thread.c \
             timer.c \
             tty.c \
             udp.c \
			 linux-core.c \
             linux-inotify.c \
             linux-syscalls.c \
             procfs-exepath.c \
             proctitle.c \
             sysinfo-loadavg.c \
             sysinfo-memory.c
UNIX_OBJS = $(patsubst %.c,%.o,$(UNIX_SOURCES))
UNIX_OBJSD = $(addprefix $(TMP_DIR)libuv/,$(UNIX_OBJS))

INCLUDE = -I ./include -I ./src -I ./src/unix

all:chkdir libuv$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)libuv/)

libuv_static:$(OBJS) $(UNIX_OBJS)
	$(AR) $(OUT_DIR)libuv.a $(OBJSD) $(UNIX_OBJSD)

libuv_shared:$(OBJS) $(UNIX_OBJS)
	$(CC) -shared -fPIC $(OBJSD) $(UNIX_OBJSD) -o $(OUT_DIR)libuv.so

$(OBJS):%.o:$(SRC_DIR)%.c
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $(TMP_DIR)libuv/$@
$(UNIX_OBJS):%.o:$(SRC_UNIX_DIR)%.c
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $(TMP_DIR)libuv/$@


clean:
	rm -rf $(TMP_DIR)libuv/*.o
