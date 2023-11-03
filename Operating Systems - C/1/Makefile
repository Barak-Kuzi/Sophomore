CC = gcc
EXEC = loopcmp digcmp lencmp lexcmp

all: $(EXEC)

loopcmp: loopcmp.c
	$(CC) loopcmp.c -o loopcmp
	
digcmp: digcmp.c
	$(CC) digcmp.c -o digcmp
	
lencmp: lencmp.c
	$(CC) lencmp.c -o lencmp
			
lexcmp: lexcmp.c
	$(CC) lexcmp.c -o lexcmp
	
clean:
	rm -rf $(EXEC)		
	
