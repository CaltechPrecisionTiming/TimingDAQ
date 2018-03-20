CXX = $(shell root-config --cxx)
LD = $(shell root-config --ld)
INC = $(shell pwd)
Repo = $(shell git rev-parse --show-toplevel)
SRC = $(Repo)/src

CPPFLAGS := $(shell root-config --cflags) -I$(INC)/include 
LDFLAGS := $(shell root-config --glibs) 
CPPFLAGS += -g -std=c++14

TARGET = ConvertDat2Root
SRC = app/ConvertDat2Root.cc src/Aux.cc src/Config.cc

TARGET2 = Rereco
SRC2 = app/Rereco.cc src/Aux.cc src/Config.cc

TARGET3 = ConvertDat2RootWithPixels
SRC3 = app/ConvertDat2RootWithPixels.cc src/Aux.cc src/Config.cc

TARGET4 = SkimTree
SRC4 = app/SkimTree.cc

TARGET5 = VMEDat2Root
SRC5 = app/VMEDat2Root.cc src/Aux.cc src/Config.cc src/DatAnalyzer.cc src/VMEAnalyzer.cc

TARGET6 = DRSDat2Root
SRC6 = app/DRSDat2Root.cc src/Aux.cc src/Config.cc src/DatAnalyzer.cc src/DRSAnalyzer.cc

OBJ = $(SRC:.cc=.o)
OBJ2 = $(SRC2:.cc=.o)
OBJ3 = $(SRC3:.cc=.o)
OBJ4 = $(SRC4:.cc=.o)
OBJ5 = $(SRC5:.cc=.o)
OBJ6 = $(SRC6:.cc=.o)

all : $(TARGET) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5) $(TARGET6)

$(TARGET) : $(OBJ)
	@echo $@
	$(LD) $(CPPFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

$(TARGET2) : $(OBJ2)
	@echo $@
	$(LD) $(CPPFLAGS) -o $(TARGET2) $(OBJ2) $(LDFLAGS)

$(TARGET3) : $(OBJ3)
	@echo $@
	$(LD) $(CPPFLAGS) -o $(TARGET3) $(OBJ3) $(LDFLAGS)

$(TARGET4) : $(OBJ4)
	@echo $@
	$(LD) $(CPPFLAGS) -o $(TARGET4) $(OBJ4) $(LDFLAGS)

$(TARGET5) : $(OBJ5)
	@echo $@
	$(LD) $(CPPFLAGS) -o $(TARGET5) $(OBJ5) $(LDFLAGS)

$(TARGET6) : $(OBJ6)
	@echo $@
	$(LD) $(CPPFLAGS) -o $(TARGET6) $(OBJ6) $(LDFLAGS)

%.o : %.cc
	@echo $@
	$(CXX) $(CPPFLAGS) -o $@ -c $<
clean :
	rm -f *.o app/*.o src/*.o $(Aux)/src/*.o $(TARGET) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5) $(TARGET6) *~
