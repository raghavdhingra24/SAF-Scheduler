CC ?= gcc
CXX ?= g++
CFLAGS ?= -Wall -Wextra -Werror -g -pthread
CXXFLAGS ?= -Wall -Wextra -Werror -g -pthread

TARGET = scheduler
SRCS_C = main.c
SRCS_CXX = tcb.cpp
OBJS = $(SRCS_C:.c=.o) $(SRCS_CXX:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.c tcb.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp tcb.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	./$(TARGET)

test-mem: $(TARGET)
	@chmod +x test_mem.sh
	./test_mem.sh

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all run test test-mem clean
