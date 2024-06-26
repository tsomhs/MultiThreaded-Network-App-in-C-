# Compiler
CC = gcc

# Compiler flags
CFLAGS = -g -Wall -I.

# Directories
BIN_DIR = bin
BUILD_DIR = build
SRC_DIR = src
TEST_DIR = tests

# Source files
SRC_FILES = jobExecutorServer.c jobCommander.c client_send.c
TEST_FILES = jobExecutorServer_test.c jobCommander_test.c

# Object files
OBJ_FILES = $(SRC_FILES:%.c=$(BUILD_DIR)/%.o)
TEST_OBJ_FILES = $(TEST_FILES:%.c=$(BUILD_DIR)/%.o)

# Executables
EXECUTABLES = $(BIN_DIR)/jobExecutorServer.exe $(BIN_DIR)/jobCommander.exe $(BIN_DIR)/jobExecutorServer_test.exe $(BIN_DIR)/jobCommander_test.exe

# Default target
all: clean src tests

src: $(BIN_DIR)/jobExecutorServer.exe $(BIN_DIR)/jobCommander.exe

tests: $(BIN_DIR)/jobExecutorServer_test.exe $(BIN_DIR)/jobCommander_test.exe

# Rules to build the executables
$(BIN_DIR)/jobExecutorServer.exe: $(BUILD_DIR)/jobExecutorServer.o
	$(CC) $(CFLAGS) -o $@ $< -lpthread

$(BIN_DIR)/jobCommander.exe: $(BUILD_DIR)/jobCommander.o $(BUILD_DIR)/client_send.o
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

$(BIN_DIR)/jobExecutorServer_test.exe: $(BUILD_DIR)/jobExecutorServer_test.o
	$(CC) $(CFLAGS) -o $@ $< -lpthread

$(BIN_DIR)/jobCommander_test.exe: $(BUILD_DIR)/jobCommander_test.o $(BUILD_DIR)/client_send.o
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

# Pattern rules to build object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(EXECUTABLES) $(OBJ_FILES) $(TEST_OBJ_FILES)
