# MEX Makefile for MATLAB on OSX

# First, define various system paths
## Define the output path for the mex module
MODULES_PATH = /Users/melchi/Documents/MATLAB
## Define MATLAB library and include paths
MLROOT = /Applications/MATLAB_R2017a.app
MLINCLUDE = $(MLROOT)/extern/include
MLBIN = $(MLROOT)/extern/bin/maci64
MLLIB = $(MLROOT)/bin/maci64
## Define Eyelink library and include paths
ELROOT = /Library/Frameworks/eyelink_core.framework
ELLIB = $(ELROOT)
ELINCLUDE = $(ELROOT)/Headers

# compiler command
CC := g++
# compilation flags
CCFLAGS = -O3 -fpic -std=c++11
# local paths
SRCPATH=src
OBJPATH=obj
LIBPATH=lib

SRC:=$(wildcard $(SRCPATH)/*.cpp)
OBJ:=$(patsubst $(SRCPATH)/%.cpp,$(OBJPATH)/%.o, $(SRC))

# define mex module name
MODULE = eyelink_hrt
# ... extension
EXT = mexmaci64
# ... and name of output file
OUT = $(LIBPATH)/$(MODULE).$(EXT)

DEFINES = -DMATLAB_MEX_FILE
# compiler flags for include paths
INCLUDES = -I$(SRCPATH) -I$(MLINCLUDE) -I$(ELINCLUDE)
# linker flags for mex libraries
LIBRARIES = -L$(MLLIB) -lmx -lmex -lmat
# ... and for Eyelink library
LIBRARIES += $(ELLIB)/eyelink_core
# ... and for standard libraries
LIBRARIES += -lstdc++ -lpthread

################################################################################
# The actual compilation/link commands appear below

default: makedirs $(OUT)
	cp $(OUT) $(MODULES_PATH)

makedirs: 
	mkdir -p $(OBJPATH) $(LIBPATH)

$(OUT): $(OBJ)
	$(CC) $(LDFLAGS) -shared -o $@ $(OBJ) $(LIBRARIES)

$(OBJPATH)/%.o: $(SRCPATH)/%.cpp
	$(CC) $(CCFLAGS) $(DEFINES) $(INCLUDES) -c -fpic $< -o $@

.PHONY: clean cleanall

clean:
	rm -f $(OBJPATH)/*.o
	rmdir $(OBJPATH)

cleanall: clean
	rm -f $(OUT)
	rm -f $(MODULES_PATH)/$(OUT)
	rmdir $(LIBPATH)
