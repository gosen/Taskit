# $@ name of the target
# $^ name of all prerequisites with duplicates removed
# $< name of the first prerequisite

APP=taskit

# Compiler
CC=g++
CXXFLAGS=-Wall -std=gnu++14
DEFINES=

# Includes
INC=-I/usr/include

# Libraries
LIB=-fPIC

# Shell Command
MK=mkdir -p
RM=rm -rf -v

# Targets
REL=release
DBG=debug
UT=test

# Paths
INCLUDE_PATH=include
SRC_PATH=src
UT_PATH=src
BUILD_PATH=build
REL_OBJ_PATH=$(BUILD_PATH)/$(REL)
DBG_OBJ_PATH=$(BUILD_PATH)/$(DBG)
BIN_PATH=bin
REL_BIN_PATH=$(BIN_PATH)/$(REL)
DBG_BIN_PATH=$(BIN_PATH)/$(DBG)

# Dependences
_DEPS = taskit.hpp \
        taskit_tasks.hpp \
        taskit_selector.hpp \
        taskit_sequence.hpp
DEPS= $(patsubst %,$(INCLUDE_PATH)/%,$(_DEPS))

# Objects
_OBJ = main.o
_UT_OBJ = test.o

############################################################
# RELEASE
############################################################

REL_BIN=$(REL_BIN_PATH)/$(APP)

$(REL): $(REL_OBJ_PATH) $(REL_BIN_PATH) $(REL_BIN)

# Path creation
$(REL_OBJ_PATH):
	$(MK) $@

$(REL_BIN_PATH):
	$(MK) $@

REL_OBJ= $(patsubst %,$(REL_OBJ_PATH)/%,$(_OBJ))

REL_CXXFLAGS=-O3 $(CXXFLAGS)

$(REL_OBJ_PATH)/%.o: $(SRC_PATH)/%.cc $(DEPS)
	$(CC) -I./$(INCLUDE_PATH) $(REL_CXXFLAGS) -c $< -o $@

$(REL_BIN): $(REL_OBJ)
	$(CC) $(REL_CXXFLAGS) -o $@ $^

############################################################
# DEBUG
############################################################

DBG_BIN=$(DBG_BIN_PATH)/$(APP)

$(DBG): $(DBG_OBJ_PATH) $(DBG_BIN_PATH) $(DBG_BIN)

# Path creation
$(DBG_OBJ_PATH):
	$(MK) $@

$(DBG_BIN_PATH):
	$(MK) $@

DBG_OBJ= $(patsubst %,$(DBG_OBJ_PATH)/%,$(_OBJ))

DBG_CXXFLAGS=-g -O0 -DDEBUG $(CXXFLAGS)

$(DBG_OBJ_PATH)/%.o: $(SRC_PATH)/%.cc $(DEPS)
	$(CC) -I./$(INCLUDE_PATH) $(DBG_CXXFLAGS) -c $< -o $@

$(DBG_BIN): $(DBG_OBJ)
	$(CC) $(DBG_CXXFLAGS) -o $@ $^

############################################################
# UNIT TEST
############################################################

include $(UT)/Makefile

############################################################
# Clean up
############################################################

.PYTHON: clean

clean:
	$(RM) build bin
