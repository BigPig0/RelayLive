#./common/crypto/Makefile
SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)crypto/,$(OBJS))

all:chkdir crypto$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)crypto/)

crypto_static:$(OBJS)
	$(AR) $(OUT_DIR)crypto.a $(OBJSD)

crypto_shared:$(OBJS)
	$(GG) -shared -fPIC $(OBJSD) -o $(OUT_DIR)crypto.so

$(OBJS):%.o:%.cpp
	$(GG) $(GFLAGS) -c $< -o $(TMP_DIR)crypto/$@

clean:
	rm -rf $(TMP_DIR)crypto/*.o