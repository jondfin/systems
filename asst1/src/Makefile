FLAGS = -Wall -Werror -fsanitize=address

all: scannerCSVsorter

scannerCSVsorter: scannerCSVsorter.o mergesort.o
	gcc $(FLAGS) scannerCSVsorter.o mergesort.o -o scannerCSVsorter 

scannerCSVsorter.o: scannerCSVsorter.c
	gcc $(FLAGS) -c -g scannerCSVsorter.c -o scannerCSVsorter.o

mergesort.o: mergesort.c
	gcc $(FLAGS) -c -g mergesort.c -o mergesort.o

clean:
	rm *.o scannerCSVsorter
