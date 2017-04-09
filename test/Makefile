# $@ name of the target
# $^ name of all prerequisites with duplicates removed
# $< name of the first prerequisite

# Paths
UT_PATH=test
UT_OBJ_PATH=$(BUILD_PATH)/$(UT)
UT_BIN_PATH=$(BIN_PATH)/$(UT)

# Libraries
UT_LIBS = -lboost_unit_test_framework

# Dependences

# Objects
_UT_OBJ = test.o

############################################################
# UNIT TEST
############################################################

UNIT_TEST=$(UT_BIN_PATH)/unit_test
RUN_UT_TEST=run_unit_test

$(UT): $(UT_OBJ_PATH) $(UT_BIN_PATH) $(RUN_UT_TEST)

# Path creation
$(UT_OBJ_PATH):
	$(MK) $@

$(UT_BIN_PATH):
	$(MK) $@

UT_OBJ= $(patsubst %,$(UT_OBJ_PATH)/%,$(_UT_OBJ))

UT_CXXFLAGS=-g -O0 $(CXXFLAGS)
UT_LIB=$(LIB) $(UT_LIBS)

$(UT_OBJ_PATH)/%.o: $(UT_PATH)/%.cc $(DEPS)
	$(CC) $(UT_CXXFLAGS) -I./$(INCLUDE_PATH) -c $< -o $@

$(UNIT_TEST): $(UT_OBJ)
	$(CC) $(UT_CXXFLAGS) $(UT_LIB) -o $@ $^

$(RUN_UT_TEST): $(UNIT_TEST)
	$(UNIT_TEST)
