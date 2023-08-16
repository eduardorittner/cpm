FLAGS = -Wall -Wextra -Werror -pedantic -std=c17

core.o: core.c core.h
	$(CC) $(FLAGS) -c core.c

cpm.o: main.c core.h
	$(CC) $(FLAGS) -c main.c
	

cpm: main.o core.o
	$(CC) main.o core.o -lm -o cpm 

all: cpm

clean:
	rm *.o
