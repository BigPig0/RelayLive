#./ThirdParty/Makefile
SUBDIR = exosip # cjson libuv pugixml lua #  hiredis   ocilib 

all:$(SUBDIR)
	
$(SUBDIR):ECHO
	make -C $@

ECHO:  
	@echo $@

.PHONY : clean

clean :CLEANDIR

CLEANDIR:ECHO
	make -C $(SUBDIR) clean