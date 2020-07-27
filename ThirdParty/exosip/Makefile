#./ThirdParty/exosip/Makefile
CARES_SRC_DIR = c-ares/
CARES_SOURCES = $(wildcard c-ares/*.c)
CARES_OBJS = $(patsubst %.c,%.o,$(notdir $(CARES_SOURCES)))
CARES_OBJSD = $(addprefix $(TMP_DIR)exosip/,$(CARES_OBJS))
CARES_FLAG = -DHAVE_CONFIG_H \
			 -I $(PWD)/ThirdParty/exosip \
			 -I $(PWD)/ThirdParty/exosip/c-ares 

OSIPPARSER2_SRC_DIR = osipparser2/
OSIPPARSER2_SOURCES = $(wildcard osipparser2/*.c)
OSIPPARSER2_OBJS = $(patsubst %.c,%.o,$(notdir $(OSIPPARSER2_SOURCES)))
OSIPPARSER2_OBJSD = $(addprefix $(TMP_DIR)exosip/,$(OSIPPARSER2_OBJS))
OSIPPARSER2_FLAG = -DHAVE_SYS_TYPES_H -DHAVE_SYS_STAT_H -DHAVE_FCNTL_H -DHAVE_CTYPE_H \
                   -DHAVE_UNISTD_H -DHAVE_SYSLOG_H -DHAVE_PTH_PTHREAD_H -DHAVE_SYS_SELECT_H \
                   -I $(PWD)/ThirdParty/exosip 

OSIP2_SRC_DIR = osip2/
OSIP2_SOURCES = $(wildcard osip2/*.c)
OSIP2_OBJS = $(patsubst %.c,%.o,$(notdir $(OSIP2_SOURCES)))
OSIP2_OBJSD = $(addprefix $(TMP_DIR)exosip/,$(OSIP2_OBJS))
OSIP2_FLAG = -DHAVE_PTHREAD -DHAVE_SEMAPHORE_H -DHAVE_STRUCT_TIMEVAL -DHAVE_SYS_TIME_H \
             -I $(PWD)/ThirdParty/exosip 

EXOSIP2_SRC_DIR = eXosip2/
EXOSIP2_SOURCES = $(wildcard eXosip2/*.c)
EXOSIP2_OBJS = $(patsubst %.c,%.o,$(notdir $(EXOSIP2_SOURCES)))
EXOSIP2_OBJSD = $(addprefix $(TMP_DIR)exosip/,$(EXOSIP2_OBJS))
EXOSIP2_FLAG = -DHAVE_CONFIG_H \
               -I $(PWD)/ThirdParty/exosip \
			   -I $(PWD)/ThirdParty/exosip/eXosip2

EXOSIP_FLAG = -DHAVE_SYS_TYPES_H -DHAVE_SYS_STAT_H -DHAVE_FCNTL_H -DHAVE_CTYPE_H -DHAVE_STRING_H \
              -DHAVE_TIME_H -DHAVE_SYS_TIME_H -DHAVE_STRUCT_TIMEVAL -DHAVE_PTHREAD -DHAVE_SEMAPHORE_H \
			  -DHAVE_LIMITS_H -DHAVE_CARES_H \
			  -DCARES_STATICLIB -DCARES_BUILDING_LIBRARY -DHAVE_INET_NTOP -DHAVE_CARES_H -DSRV_RECORD -DENABLE_TRACE -DEXOSIP_EXPORTS -DSYSTEM_LOGGER_ENABLED \
              -I $(PWD)/ThirdParty/exosip -I $(PWD)/ThirdParty/exosip/c-cres \
			  -I $(PWD)/ThirdParty/exosip/osipparser2 -I $(PWD)/ThirdParty/exosip/osipparser2/headers \
			  -I $(PWD)/ThirdParty/exosip/osip2 -I $(PWD)/ThirdParty/exosip/eXosip

all:chkdir exosip$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)exosip/)

exosip_static:$(CARES_OBJS) $(OSIPPARSER2_OBJS) $(OSIP2_OBJS) $(EXOSIP2_OBJS)
	$(AR) $(OUT_DIR)exosip.a $(CARES_OBJSD) $(OSIPPARSER2_OBJSD) $(OSIP2_OBJSD) $(EXOSIP2_OBJSD)

exosip_shared:$(CARES_OBJS) $(OSIPPARSER2_OBJS) $(OSIP2_OBJS) $(EXOSIP2_OBJS)
	$(CC) -shared -fPIC $(CARES_OBJSD) $(OSIPPARSER2_OBJSD) $(OSIP2_OBJSD) $(EXOSIP2_OBJSD) -o $(OUT_DIR)exosip.so

$(CARES_OBJS):%.o:$(CARES_SRC_DIR)%.c
	$(CC) $(CARES_FLAG) $(CFLAGS) -c $< -o $(TMP_DIR)exosip/$@
$(OSIPPARSER2_OBJS):%.o:$(OSIPPARSER2_SRC_DIR)%.c
	$(CC) $(OSIPPARSER2_FLAG) $(CFLAGS) -c $< -o $(TMP_DIR)exosip/$@
$(OSIP2_OBJS):%.o:$(OSIP2_SRC_DIR)%.c
	$(CC) $(OSIP2_FLAG) $(CFLAGS) -c $< -o $(TMP_DIR)exosip/$@
$(EXOSIP2_OBJS):%.o:$(EXOSIP2_SRC_DIR)%.c
	$(CC) $(EXOSIP2_FLAG) $(CFLAGS) -c $< -o $(TMP_DIR)exosip/$@


clean:
	rm -rf $(TMP_DIR)exosip/*.o
