CXX = $(shell root-config --cxx)
LD = $(shell root-config --ld)
INC = $(shell pwd)

CPPFLAGS := $(shell root-config --cflags) -I$(INC)/include
LDFLAGS := $(shell root-config --glibs)
CPPFLAGS += -g -std=c++11

TARGETS = VMEDat2Root DRSDat2Root
#ScopeDat2Root
SRC = src/Configuration.cc src/DatAnalyzer.cc 

TARGETS2 = BTL_Analysis
#ScopeDat2Root
SRC2 = src/pulse.cc

all : $(TARGETS) $(TARGETS2)

$(TARGETS) : %Dat2Root : $(SRC:.cc=.o) src/%Analyzer.o app/%Dat2Root.cc 
	@echo Building $@
	$(LD) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

$(TARGETS2) : $(SRC2:.cc=.o) app/BTL_Analysis.cc 
	@echo Building $@
	$(LD) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

%.o : %.cc
	@echo $@
	$(CXX) $(CPPFLAGS) -o $@ -c $<
clean :
	rm -rf *.o app/*.o src/*.o $(TARGETS) $(TARGETS2) *~ *.dSYM
