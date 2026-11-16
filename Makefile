CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
SRCS = src/main.c src/student.c src/storage.c
OBJS = $(SRCS:.c=.o)

# Windows toolchains (MinGW/MSYS2) append .exe automatically
ifeq ($(OS),Windows_NT)
TARGET = sms.exe
else
TARGET = sms
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c src/student.h src/storage.h
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all run clean
