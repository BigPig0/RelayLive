#./thirdparty/Makefile
SUBDIR = sqlite3 #cjson exosip hiredis libcstl libmariadb librabbitmq libuv lua ocilib pugixml zlib

all:$(SUBDIR)
	
$(SUBDIR):ECHO
	make -C $@

ECHO:  
	@echo $@

.PHONY : clean

clean :CLEANDIR

CLEANDIR:ECHO
	make -C $(SUBDIR) clean