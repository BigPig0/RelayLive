#./ThirdParty/cjson/Makefile
SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(notdir $(SOURCES)))
OBJSD = $(addprefix $(TMP_DIR)pugixml/,$(OBJS))

all:chkdir pugixml$(TAGTYPE)

chkdir:
	$(shell mkdir -p $(TMP_DIR)pugixml/)

pugixml_static:$(OBJS)
	$(AR) $(OUT_DIR)pugixml.a $(OBJSD)

pugixml_shared:$(OBJS)
	$(GG) -shared -fPIC $(OBJSD) -o $(OUT_DIR)pugixml.so

$(OBJS):%.o:%.cpp
	$(GG) $(GFLAGS) -c $< -o $(TMP_DIR)pugixml/$@

clean:
	rm -rf $(TMP_DIR)pugixml/*.o