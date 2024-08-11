# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Libraries
LIBS = -lSDL2 -lm

# Output executable
TARGET = simulador

# Source files
SRCS = simulador.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(TARGET) $(OBJS)

# Phony targets
.PHONY: all clean
