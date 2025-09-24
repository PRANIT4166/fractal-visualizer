# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g # -g adds debug symbols, useful for Valgrind
LDFLAGS = -lmpfr -lgmp   # Link against MPFR and GMP libraries

# Source files
SOURCES = mandelbrot.c MPFR_Complex_Math.c tester.c

# Executable name
TARGET = test_runner

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)~