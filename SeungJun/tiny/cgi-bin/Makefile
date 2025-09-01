CC = gcc
CFLAGS = -O2 -Wall -I ..

all: adder

adder: adder.c ../csapp.o
	$(CC) $(CFLAGS) -o adder adder.c ../csapp.o

clean:
	rm -f adder *~
