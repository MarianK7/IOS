CC=gcc
CFLAGS= -std=gnu99 -Wall -Wextra -Werror -pedantic -pthread
BIN=proj2
SOURCE=proj2.c
PARAMS=5 5 10 10

all:
	$(CC) $(CFLAGS) $(SOURCE) -o $(BIN)

clean:
	rm $(BIN) *.out	

zip:
	zip proj2.zip $(SOURCE) Makefile

run:
	./$(BIN) 5 5 10 10

out:
	cat proj2.out
