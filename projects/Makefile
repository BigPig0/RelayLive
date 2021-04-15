#./projects/Makefile
SUBDIR = hik_sdk #sip_server ipc_server livectrl_server live_server

all:$(SUBDIR)
	
$(SUBDIR):ECHO
	make -C $@

ECHO:  
	@echo $@

.PHONY : clean

clean :CLEANDIR

CLEANDIR:ECHO
	make -C $(SUBDIR) clean