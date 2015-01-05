CC=gccx

all: shell

shell: shell.c
	$(CC) -o shell shell.c
clean:
	rm -f shell *.o
