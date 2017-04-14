
PROG=acio_test

DEBUG=-g
OPTIMIZATION=-O2
CFLAGS=-Wall $(DEBUG) $(OPTIMIZATION)
CPPFLAGS=
CXXFLAGS=$(CFLAGS)
CXX=g++ -std=c++14
LDFLAGS=$(DEBUG)
LD=g++ -std=c++14
LIBS=-lrt
RM=rm -f

THISFILE=$(MAKEFILE_LIST)
DEPDIR=.d
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MF $(DEPDIR)/$*.Td
COMPILE.cpp=$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
POSTCOMPILE=mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

ALLSRCS=$(wildcard *.cpp *.c)
SRCS=$(sort $(ALLSRCS))
OBJS=$(SRCS:.cpp=.o)

all: $(PROG)

$(PROG): $(OBJS) $(THISFILE)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROG)

%.o : %.cpp $(THISFILE)
%.o : %.cpp $(DEPDIR)/%.d $(THISFILE)
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

DEPFILES=$(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS)))
-include $(DEPFILES)

clean:
	$(RM) $(OBJS) $(PROG) $(DEPFILES)

