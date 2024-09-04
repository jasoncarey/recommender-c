# Makefile for the recommender system project

CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = recommender

# Source files
SRCS = main.c sparse_matrix.c model.c utils.c

# Object files (each .c file will be compiled to a .o file)
OBJS = $(SRCS:.c=.o)

# Default rule: compile and link the program
all: $(TARGET)

# Rule to link the object files into the final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile each .c file into a .o file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJS) $(TARGET)