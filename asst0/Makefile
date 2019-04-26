FLAGS = -Wall -Werror -fsanitize=address

all: simpleCSVsorter

simpleCSVsorter: simpleCSVsorter.o mergesort.o
	gcc $(FLAGS) simpleCSVsorter.o mergesort.o -o simpleCSVsorter 

simpleCSVsorter.o: simpleCSVsorter.c
	gcc $(FLAGS) -c -g simpleCSVsorter.c -o simpleCSVsorter.o

mergesort.o: mergesort.c
	gcc $(FLAGS) -c -g mergesort.c -o mergesort.o

clean:
	rm *.o simpleCSVsorter
