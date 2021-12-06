#./projects/Makefile
SUBDIR = ipc_server control_server sip_server live_server #worker sip_server ipc_server control_server live_server hik_sdk relay_server capture_server

all:$(SUBDIR)
	
$(SUBDIR):ECHO
	make -C $@

ECHO:  
	@echo $@

.PHONY : clean

clean :CLEANDIR

CLEANDIR:ECHO
	make -C $(SUBDIR) clean