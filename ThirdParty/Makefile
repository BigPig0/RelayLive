#./ThirdParty/Makefile
SUBDIR = hiredis_linux # cjson libuv pugixml lua exosip #     ocilib 

all:$(SUBDIR)
	
$(SUBDIR):ECHO
	make -C $@

ECHO:  
	@echo $@

.PHONY : clean

clean :CLEANDIR

CLEANDIR:ECHO
	make -C $(SUBDIR) clean