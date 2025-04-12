CC = gcc
CFLAGS = -g -std=c89 -pedantic -O0 -Werror -MMD -MP -L $(BUILD_DIR)

EXEC_NAME = zcc
BUILD_DIR = $(PWD)/build
EXEC = $(BUILD_DIR)/$(EXEC_NAME)

ZCC_SOURCES = main.c
ZCC_SOURCES_DIR = $(PWD)/src
ZCC_SOURCES_PATHS = $(addprefix $(ZCC_SOURCES_DIR)/,$(ZCC_SOURCES))

FRONTEND_SOURCES_DIR = $(PWD)/src/frontend
FRONTEND_SOURCES_PATHS = $(wildcard $(FRONTEND_SOURCES_DIR)/*.c)
FRONTEND_SOURCES_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(patsubst %.c,%.o,$(FRONTEND_SOURCES_PATHS))))
FRONTEND = $(BUILD_DIR)/libfrontend.a

COMMON_SOURCES_DIR = $(PWD)/src/common
COMMON_SOURCES_PATHS = $(wildcard $(COMMON_SOURCES_DIR)/*.c)
COMMON_SOURCES_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(patsubst %.c,%.o,$(COMMON_SOURCES_PATHS))))

COMMON = $(BUILD_DIR)/libcommon.a

TESTS_SOURCES_DIR = $(PWD)/tests
TESTS_SOURCES_PATHS = $(wildcard $(TESTS_SOURCES_DIR)/*.c)
TESTS_OBJECTS = $(addprefix $(BUILD_DIR)/tests/,$(notdir $(patsubst %.c,%.o,$(TESTS_SOURCES_PATHS))))
TEST_EXECUTABLES = $(addprefix $(BUILD_DIR)/tests/,$(notdir $(patsubst %.c,%,$(TESTS_SOURCES_PATHS))))
TESTS_FLAGS = -g -std=c89 -pedantic -O0 -Werror -MMD -MP -L $(BUILD_DIR)

.PHONY: all directories clean test

all: $(EXEC) $(TEST_EXECUTABLES)

$(EXEC): directories $(ZCC_SOURCES_PATHS) $(FRONTEND)
	$(CC) $(ZCC_SOURCES_PATHS) $(CFLAGS) -I $(COMMON_SOURCES_DIR) -I $(FRONTEND_SOURCES_DIR) -o $(EXEC) -lcommon -lfrontend

directories: $(BUILD_DIR) $(BUILD_DIR)/tests

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/tests:
	mkdir -p $(BUILD_DIR)/tests

$(FRONTEND): $(FRONTEND_SOURCES_OBJECTS)
	ar r $@ $(addprefix $(BUILD_DIR)/,$(notdir $(FRONTEND_SOURCES_OBJECTS)))

$(BUILD_DIR)/%.o: $(FRONTEND_SOURCES_DIR)/%.c $(COMMON)
	$(CC) -c $< $(CFLAGS) -I $(COMMON_SOURCES_DIR) -o $(BUILD_DIR)/$(notdir $@) -lcommon

$(COMMON): $(COMMON_SOURCES_OBJECTS)
	ar r $@ $(addprefix $(BUILD_DIR)/,$(notdir $^))

$(BUILD_DIR)/%.o: $(COMMON_SOURCES_DIR)/%.c
	$(CC) -c $< $(CFLAGS) -o $(BUILD_DIR)/$(notdir $@)

# Rule to run all test executables
test: directories $(TEST_EXECUTABLES) 
	@for executable in $(TEST_EXECUTABLES); do \
		valgrind --leak-check=full --errors-for-leak-kinds=all --error-exitcode=1 $$executable; \
	done

$(TEST_EXECUTABLES): $(BUILD_DIR)/tests/%: $(TESTS_SOURCES_PATHS) $(COMMON) $(FRONTEND)
	$(CC) $(TESTS_SOURCES_PATHS) $(PWD)/ext/Unity/src/unity.c $(TESTS_FLAGS) -I $(PWD)/ext/Unity/src -I $(COMMON_SOURCES_DIR) -I $(FRONTEND_SOURCES_DIR) -o $@ -lcommon -lfrontend
	
-include $(wildcard $(BUILD_DIR)/*.d)

clean:
	rm -rf $(BUILD_DIR)