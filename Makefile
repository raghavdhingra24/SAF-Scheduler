CC ?= gcc
CXX ?= g++
CFLAGS ?= -Wall -Wextra -Werror -g -pthread -Iinclude
CXXFLAGS ?= -Wall -Wextra -Werror -g -pthread -Iinclude

TARGET = scheduler
SRCS_C = main.c
SRCS_CXX = src/tcb.cpp
OBJS = $(SRCS_C:.c=.o) $(SRCS_CXX:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.c include/tcb.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp include/tcb.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

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
