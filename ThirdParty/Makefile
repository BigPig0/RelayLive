#./ThirdParty/Makefile
SUBDIR = lua # cjson libuv pugixml exosip hiredis   ocilib 

all:$(SUBDIR)
	
$(SUBDIR):ECHO
	make -C $@

ECHO:  
	@echo $@

.PHONY : clean

clean :CLEANDIR

CLEANDIR:ECHO
	make -C $(SUBDIR) clean