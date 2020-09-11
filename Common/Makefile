#./Common/Makefile
SUBDIR = ludb#utilc util ssl ludb

all:$(SUBDIR)
	
$(SUBDIR):ECHO
	make -C $@

ECHO:  
	@echo $@

.PHONY : clean

clean :CLEANDIR

CLEANDIR:ECHO
	make -C $(SUBDIR) clean