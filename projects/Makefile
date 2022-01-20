#./projects/Makefile
SUBDIR = test#worker sip_server ipc_server control_server live_server hik_sdk relay_server capture_server hikisup_sdk

all:$(SUBDIR)
	
$(SUBDIR):ECHO
	make -C $@

ECHO:  
	@echo $@

.PHONY : clean

clean :CLEANDIR

CLEANDIR:ECHO
	make -C $(SUBDIR) clean