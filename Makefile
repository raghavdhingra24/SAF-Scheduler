CC ?= gcc
CFLAGS ?= -std=c11 -D_GNU_SOURCE -Wall -Wextra -Werror -g -pthread -Iinclude

TARGET = scheduler
SRCS = main.c src/tcb.c src/runqueue.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	./$(TARGET)

test-mem: $(TARGET)
	@chmod +x scripts/test_mem.sh
	./scripts/test_mem.sh

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all run test test-mem clean
