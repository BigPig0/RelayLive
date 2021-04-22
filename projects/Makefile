#./projects/Makefile
SUBDIR = sip_server ipc_server livectrl_server live_server hik_sdk

all:$(SUBDIR)
	
$(SUBDIR):ECHO
	make -C $@

ECHO:  
	@echo $@

.PHONY : clean

clean :CLEANDIR

CLEANDIR:ECHO
	make -C $(SUBDIR) clean