CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
SRCS = src/main.c src/student.c src/storage.c
OBJS = $(SRCS:.c=.o)
TEST_SRC = tests/test_student.c
TEST_BIN = test_runner

# Detect Windows shells: Git Bash / MSYS2 / MinGW report MINGW*_NT or MSYS*_NT
UNAME_S := $(shell uname -s 2>/dev/null)
IS_WINDOWS := $(or $(filter Windows_%,$(OS)),$(findstring MINGW,$(UNAME_S)),$(findstring MSYS,$(UNAME_S)))

ifeq ($(IS_WINDOWS),)
TARGET = sms
TEST_BIN = test_runner
else
TARGET = sms.exe
TEST_BIN = test_runner.exe
# MSYS make may strip TMP/TEMP; MinGW tools need a writable temp dir
export TMP := /tmp
export TEMP := /tmp
export TMPDIR := /tmp
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c src/student.h src/storage.h
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(TARGET)
	./$(TARGET)

# Build and run the assert-based unit tests (no external framework)
test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_SRC) src/student.c src/storage.c
	$(CC) $(CFLAGS) -Isrc -o $@ $(TEST_SRC) src/student.c src/storage.c

# Seed the database with 10 sample students (appends; duplicates skipped)
seed: $(TARGET)
	./$(TARGET) < scripts/seed_input.txt

clean:
	rm -f $(TARGET) $(OBJS) $(TEST_BIN)

.PHONY: all run test seed clean
