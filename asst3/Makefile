FLAGS = -Wall -Werror -fsanitize=address

all: bankSystem

bankSystem: bankingServer.o bankingClient.o
	gcc $(FLAGS) -lpthread bankingServer.o -o bankingServer -g
	gcc $(FLAGS) -lpthread bankingClient.o -o bankingClient -g

bankingServer.o: bankingServer.c
	gcc $(FLAGS) -c -g bankingServer.c

bankingClient.o: bankingClient.c
	gcc $(FLAGS) -c -g bankingClient.c

clean:
	rm *.o bankingServer bankingClient
