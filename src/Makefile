DEPDIR := .d
OBJDIR := ./obj

CC = clang
CXX = clang++

CPPSRCS = $(shell find . -name "*.cpp" | grep -v "test/" | grep -v "blob/")
CSRCS = $(shell find . -name "*.c" | grep -v "test/" | grep -v "blob/")

TEST_CPPSRCS = $(shell find ./test/ -name "*.cpp")
SRCS = $(CPPSRCS) $(CSRCS)
OBJS = $(patsubst %.o, $(OBJDIR)/%.o, $(CPPSRCS:.cpp=.o) $(CSRCS:.c=.o))
TEST_OBJS = $(patsubst %.o, $(OBJDIR)/%.o, $(TEST_CPPSRCS:.cpp=.o))

$(shell mkdir -p $(DEPDIR) >/dev/null)
$(shell mkdir -p $(OBJDIR) >/dev/null)

DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
H_INCLUDE = -I./ -I../binaryninja-api/ -I../3rd_party/argparse/include -I../3rd_party/plog/include
LIB_INCLUDE = -L../binaryninja-api/bin/
LIBS = -lbinaryninjaapi -ldl -pthread
TARGET=../bin/main


CPPFLAGS = $(H_INCLUDE) -Wall -g -std=c++20
CPPFLAGS += ${TEST_CPPFLAGS}

#Link binaryninja, set rpath
LDFLAGS = -g /opt/binaryninja/libbinaryninjacore.so.1 -Wl,-rpath,/opt/binaryninja/
LINKER = clang++

COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
COMPILE.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@
POSTCOMPILE = touch $@

.PHONY : clean test

$(TARGET): $(OBJS)
	$(LINKER) $(LIB_INCLUDE) $(LDFLAGS) -o $(@) $(^) $(LIBS)

$(OBJDIR)/%.o : %.c
$(OBJDIR)/%.o : %.c $(DEPDIR)/%.d
	mkdir -p $(shell dirname $^ $@)
	$(COMPILE.c) $(OUTPUT_OPTION) $< -o $@
	$(POSTCOMPILE)

$(OBJDIR)/%.o : %.cc
$(OBJDIR)/%.o : %.cc $(DEPDIR)/%.d
	mkdir -p $(shell dirname $^ $@)
	$(COMPILE.cc) $(OUTPUT_OPTION) $< -o $@
	$(POSTCOMPILE)

$(OBJDIR)/%.o : %.cxx
$(OBJDIR)/%.o : %.cxx $(DEPDIR)/%.d
	mkdir -p $(shell dirname $^ $@)
	$(COMPILE.cc) $(OUTPUT_OPTION) $< -o $@
	$(POSTCOMPILE)

$(OBJDIR)/%.o : %.cpp
$(OBJDIR)/%.o : %.cpp $(DEPDIR)/%.d
	mkdir -p $(shell dirname $^ $@)
	$(COMPILE.cc) $(OUTPUT_OPTION) $< -o $@
	$(POSTCOMPILE)

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

../bin/run_all_test: $(TEST_OBJS) $(OBJS)
	$(LINKER) $(LIB_INCLUDE) $(LDFLAGS) -o $(@) $(^) $(LIBS) -lgtest

test: ../bin/run_all_test
	-../bin/run_all_test --gtest_filter='-*SLOW*'
full_test: ../bin/run_all_test
	-../bin/run_all_test

clean:
	-rm -rf $(OBJS)
	-rm -rf $(TEST_OBJS)
	-rm -rf $(TARGET)
	-rm -rf $(DEPDIR)
	-rm -rf ../bin/*
include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS))))
