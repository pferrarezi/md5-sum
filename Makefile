# Makefile for md5sum (Windows, CNG / bcrypt)
# Works with MinGW-w64 (gcc) out of the box.
# For MSVC use:  nmake /f Makefile.msvc   (or just: cl md5sum.c bcrypt.lib)

CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=c11
LDLIBS  = -lbcrypt
TARGET  = md5sum.exe
SRC     = md5sum.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDLIBS)

clean:
	-del /Q $(TARGET) 2> nul

.PHONY: all clean
