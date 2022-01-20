#./thirdparty/Makefile
SUBDIR = mbedtls#cjson exosip hiredis libcstl libmariadb librabbitmq sqlite3 libuv lua ocilib pugixml zlib

all:$(SUBDIR)
	
$(SUBDIR):ECHO
	make -C $@

ECHO:  
	@echo $@

.PHONY : clean

clean :CLEANDIR

CLEANDIR:ECHO
	make -C $(SUBDIR) clean