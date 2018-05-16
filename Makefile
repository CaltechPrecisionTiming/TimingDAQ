CXX = $(shell root-config --cxx)
LD = $(shell root-config --ld)
INC = $(shell pwd)

CPPFLAGS := $(shell root-config --cflags) -I$(INC)/include
LDFLAGS := $(shell root-config --glibs)
CPPFLAGS += -g -std=c++11

TARGETS = VMEDat2Root DRSDat2Root DRSclDat2Root NetScopeDat2Root
SRC = src/Configuration.cc src/DatAnalyzer.cc

ALL_TARGETS = $(TARGETS) BTL_PulseShapeSimulation

all : $(ALL_TARGETS)


BTL_PulseShapeSimulation: $(SRC:.cc=.o) src/SiPM_SimAnalyzer.o src/PulseShape.o app/BTL_PulseShapeSimulation.cc
	@echo Building $@
	$(LD) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

$(TARGETS) : %Dat2Root : $(SRC:.cc=.o) src/%Analyzer.o app/%Dat2Root.cc
	@echo Building $@
	$(LD) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

%.o : %.cc
	@echo $@
	$(CXX) $(CPPFLAGS) -o $@ -c $<
clean :
	rm -rf *.o app/*.o src/*.o $(ALL_TARGETS) $(all) *~ *.dSYM
