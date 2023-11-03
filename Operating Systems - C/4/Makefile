CC = gcc
EXEC = primecount
DEBUG = -g
CFLAGS = -Wall -Werror $(DEBUG) $(THREAD)
THREAD = -pthread -lpthread

all: $(EXEC)

primecount: primecount.c
	$(CC) $(CFLAGS) primecount.c -o primecount -lm
			
clean:
	rm -rf $(EXEC)
