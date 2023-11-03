CC = gcc
EXEC = primecount
DEBUG = -g
CFLAGS = -std=c99 -Wall -Werror $(DEBUG)

all: $(EXEC)

primecount: primecount.c
	$(CC) primecount.c -o primecount -lm
			
clean:
	rm -rf $(EXEC)
