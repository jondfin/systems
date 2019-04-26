#include "bankingServer.h"

/*Globals*/
Node *table[20];
sock *socketList = NULL;
sem_t s1;
pthread_mutex_t m1, m2;
int run = 1;

/*Handler for client*/
void *server_client_interaction(void *arg){
	printf("Server has accepted a connection from a client\n");

	/*Client socket*/
	int c_socket = *(int*)arg;
	
	/*Buffers*/
	char command[256];
	char param[256];
	char inputBuffer[512];
	char outputBuffer[256];

	int inServiceSession = 0;
	int key = -1;
	int bytesRead = 0;
	char name[256];

	/*Read input commands from client*/
	while(1){
		if((bytesRead = recv(c_socket, inputBuffer, sizeof(inputBuffer), 0)) == 0){
			printf("Client has ended their process\n");
			break;
		}else if(bytesRead == -1){
			perror("Error receiving message\n");
			break;
		}
		sscanf(inputBuffer, "%s %[^\n]", command, param);

		memset(outputBuffer, 0, sizeof(outputBuffer));		
                if(strcmp(command,"create") == 0){
			/*Currently in a service session, unable to create new account*/
			if(inServiceSession == 1){
				write(c_socket, outputBuffer, sprintf(outputBuffer, "Already servicing an account, cannot create new account\n"));
			}else{
				/*Account name taken*/
                        	if(create(param) == 0){
					write(c_socket, outputBuffer, sprintf(outputBuffer, "Account name already taken, please pick another name\n"));
				}else{
					/*Creation successful*/
					write(c_socket, outputBuffer, sprintf(outputBuffer, "Account creation successful\n"));
				}
			}
                }
                else if(strcmp(command,"serve") == 0){
			/*Currently in a service session, unable to serve account*/
			if(inServiceSession == 1){
				write(c_socket, outputBuffer, sprintf(outputBuffer, "Already servicing an account, cannot serve another account\n"));
			}else{
				key = hash(param);
				int check = 0;
				/*Account exists, define check to see if it is being used by a different client process*/
				pthread_mutex_lock(&m1);
				Node *ptr = table[key];
				while(ptr != NULL){
					if(strcmp(ptr->acc->name, param) == 0){
						check = ptr->acc->inSession;
						break;
					}
					ptr = ptr->next;
				}
				/*Account does not exist*/
				if(ptr == NULL){
					write(c_socket, outputBuffer, sprintf(outputBuffer, "Account does not exist\n"));
				}
				pthread_mutex_unlock(&m1);

				/*If account being served by a different client process*/
				if(check == 1){
					write(c_socket, outputBuffer, sprintf(outputBuffer, "Account is being serviced by a different client\n"));		
				}
				else if(check == 0){
					pthread_mutex_lock(&m1);
					ptr = table[key];
					while(ptr != NULL){
						if(strcmp(ptr->acc->name, param) == 0){
							ptr->acc->inSession = 1;
							inServiceSession = 1;
							strcpy(name, param);
							printf("%s\n", name);
							write(c_socket, outputBuffer, sprintf(outputBuffer, "Now serving %s\n", param));
							break;
						}
						ptr = ptr->next;
					}
					pthread_mutex_unlock(&m1);
				}
			}
                }
                else if(strcmp(command,"deposit") == 0){
			/*Not in service session*/
			if(inServiceSession == 0){
				write(c_socket, outputBuffer, sprintf(outputBuffer, "Must be in service to deposit\n"));
			}else{
				/*Deposit money*/
				pthread_mutex_lock(&m1);
				key = hash(name);
				Node *ptr = table[key];
				while(ptr != NULL){
					if(strcmp(ptr->acc->name, name) == 0){
						ptr->acc->balance = ptr->acc->balance + atof(param);
						write(c_socket, outputBuffer, sprintf(outputBuffer, "Successfully deposited\n"));
						break;
					}
					ptr = ptr->next;
				}
				pthread_mutex_unlock(&m1);
			}
                }
                else if(strcmp(command,"withdraw") == 0){
			/*Not in service session*/
			if(inServiceSession == 0){
				write(c_socket, outputBuffer, sprintf(outputBuffer, "Must be in service to withdraw\n"));
			}else{
				pthread_mutex_lock(&m1);
				key = hash(name);
				Node *ptr = table[key];
				while(ptr != NULL){
					if(strcmp(ptr->acc->name, name) == 0){
						/*Overdraw*/
						if((ptr->acc->balance - atof(param)) < 0){
							write(c_socket, outputBuffer, sprintf(outputBuffer, "Withdrawal amount is greater than your current balance\n"));
						}else{
							/*Withdraw money*/
							ptr->acc->balance = ptr->acc->balance -  atof(param);
							write(c_socket, outputBuffer, sprintf(outputBuffer, "Successfully withdrawn\n"));
						}
						break;
					}
					ptr = ptr->next;
				}
				pthread_mutex_unlock(&m1);
			}
		}
                else if(strcmp(command,"query") == 0){
			/*Not in service session*/
			if(inServiceSession == 0){
				write(c_socket, outputBuffer, sprintf(outputBuffer, "Must be in service to query bank account\n"));
			}else{
				pthread_mutex_lock(&m1);
				key = hash(name);
				Node *ptr = table[key];
				while(ptr != NULL){
					if(strcmp(ptr->acc->name, name) == 0){
						write(c_socket, outputBuffer, sprintf(outputBuffer, "Current balance is: %f\n", ptr->acc->balance));
						break;
					}
					ptr = ptr->next;
				}
				pthread_mutex_unlock(&m1);
			}
                }
                else if(strcmp(command,"end") == 0){
			/*Not in service session*/
			if(inServiceSession == 0){
				write(c_socket, outputBuffer, sprintf(outputBuffer, "Must be in service to end current session\n"));
			}else{
				write(c_socket, outputBuffer, sprintf(outputBuffer, "Ending current service session. You may now create a new account or enter a service session again\n"));
				pthread_mutex_lock(&m1);
				key = hash(name);
				Node *ptr = table[key];
				while(ptr != NULL){
					if(strcmp(ptr->acc->name, name) == 0){
						ptr->acc->inSession = 0;
						break;
					}
					ptr = ptr->next;
				}
				pthread_mutex_unlock(&m1);
				inServiceSession = 0;
				memset(name, 0, sizeof(name));
			}
                }
                else if(strcmp(command,"quit") == 0){
			/*Do this stuff first then terminate client-side*/
			printf("Client %s is terminating their session\n", name);
			/*If client attempts to quit before ending their service session, end their session then quit*/
			if(inServiceSession == 1){
				key = hash(name);
				Node *ptr = table[key];
				while(ptr != NULL){
					if(strcmp(ptr->acc->name, name) == 0){
						ptr->acc->inSession = 0;
						break;
					}
					ptr = ptr->next;
				}
			}
			inServiceSession = 0;
			memset(name, 0, sizeof(name));
			close(c_socket);
			pthread_exit(NULL);
                }
		memset(inputBuffer, 0, sizeof(inputBuffer));
		memset(command, 0, sizeof(command));
		memset(param, 0, sizeof(param));
	}
	pthread_exit(NULL);
}
/*Hash function based off of the first letter of the username*/
int hash(char *name){
	char letter = name[0];
	return letter%20;
}
/*Insert a new node*/
Node *insert(int key, account *a){
	pthread_mutex_lock(&m1);

	Node *front = table[key];
	Node *newAcc = (Node*)malloc(sizeof(Node));
	newAcc->acc = a;
	newAcc->next = NULL;

	if(table[key] == NULL){
		table[key] = newAcc;
	}
	else{
		Node *ptr = table[key];
		while(ptr->next != NULL){
			ptr = ptr->next;
		}
		ptr->next = newAcc;
	}

	pthread_mutex_unlock(&m1);
	return front;
}
/*Searches for account name in our table of accounts*/
int search(int key, char *name){
	pthread_mutex_lock(&m1);

	int ret = 0;
	Node *ptr = table[key];
	while(ptr != NULL){
		/*Found the account*/
		if(strcmp(ptr->acc->name, name) == 0){
			ret =  1;
		}
		ptr = ptr->next;
	}

	pthread_mutex_unlock(&m1);
	return ret;
}
/*Create a new account*/
int create(char accountName[256]){
	/*Check if account was already created*/
	int key = hash(accountName);
	if(search(key, accountName) == 1){
		return 0;
	}
	/*Create new account*/
	account *a = (account*)malloc(sizeof(account));
	strcpy(a->name, accountName);
	a->balance = 0;
	a->inSession = 0;
	insert(key, a);
	return 1;
}
/*Free nodes*/
void freeNode(Node *front){
	if(front == NULL) return;
	if(front->next == NULL){
		free(front->acc);
		free(front);
	}
	else freeNode(front->next);
}
/*Close all sockets and free memory*/
void freeSockList(sock *front){
	if(front == NULL) return;
	if(front->next == NULL){
		/*Tell client that server is shutting down*/
		char buf[256];
		if(write(front->sockID, buf, sprintf(buf, "Server is shutting down. Disconnecting...\n")) > 0){
			/*If we can successfully write to the client, then the socket was not closed by the 'quit' command*/
			memset(buf, 0, sizeof(buf));
			/*Message that disconnects client from server*/
			write(front->sockID, buf, sprintf(buf, "DIE"));
			close(front->sockID);
		}
		free(front);
	}
	else freeSockList(front->next);
}
/*Aggregate all socket ids into one list for closing later*/
sock *addSock(sock *front, int sid){
	pthread_mutex_lock(&m2);

	sock *s = (sock*)malloc(sizeof(sock));
	s->sockID = sid;
	if(front == NULL){
		front = s;
	}else{
		sock *ptr = front;
		while(ptr->next != NULL){
			ptr = ptr->next;
		}
		ptr->next = s;
	}
	
	pthread_mutex_unlock(&m2);
	return front;
}
void killServer(int sig){
	printf("\nTerminating server\n");
	run = 0;

	/*Close sockets and free memory*/
	freeSockList(socketList);	

	/*Destroy mutexes and semaphores*/
	pthread_mutex_destroy(&m1);
	pthread_mutex_destroy(&m2);
	sem_destroy(&s1);

	/*Free memory*/
	int i;
	for(i = 0; i < 20; i++){
		freeNode(table[i]);
	}
	exit(EXIT_SUCCESS);
}
/*Print out table data*/
void print15(int sig){
	sem_wait(&s1);
	printf("\n");
	int i;
	for(i = 0; i < 20; i++){
		Node *ptr = table[i];
		while(ptr != NULL){
			printf("%s\t%f\t", ptr->acc->name, ptr->acc->balance);
			if(ptr->acc->inSession == 1){
				printf("IN SERVICE\n");
			}else{
				printf("\n");
			}
			ptr = ptr->next;
		}	
	}
	sem_post(&s1);
	return;
}
int main(int argc, char **argv){
	/*Check if valid port was input*/
	if(atoi(argv[1]) < 8192){
		printf("ERROR: Please input a port number >8192\n");
		exit(1);
	}

	/*Initialize addr info*/
	struct addrinfo addr;
	addr.ai_flags = AI_PASSIVE;
	addr.ai_family = AF_INET;
	addr.ai_socktype = SOCK_STREAM;
	addr.ai_protocol = 0;
	addr.ai_addrlen = 0;
	addr.ai_canonname = NULL;
	addr.ai_next = NULL;
	
	struct addrinfo *addr2;
	getaddrinfo(0, argv[1], &addr, &addr2);

	/*Initialize server socket*/
	int s_socket = socket(addr2->ai_family, addr2->ai_socktype, addr2->ai_protocol);
	bind(s_socket, addr2->ai_addr, addr2->ai_addrlen);
	listen(s_socket,10);

	/*Set socket options*/
	int opt = 1;
	setsockopt(s_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
	
	/*Accept incoming client connections*/
	int c_socket; //File descriptor for client socket
	void *client_socket_arg; 
	/*Thread to handle incoming clients*/
	pthread_t client;
	/*Initialize mutexes and semaphores*/
	pthread_mutex_init(&m1, NULL);
	pthread_mutex_init(&m2, NULL);
	sem_init(&s1, 0, 1);
	
	/*Timer that prints out table every 15 seconds*/
	struct itimerval timer;
	timer.it_value.tv_sec = 15;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = 15;
	timer.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);

	/*Signal handler to catch SIGALRM caused by the itimer*/
	signal(SIGALRM, print15);
	/*Signal handler to catch CTRL+C and terminate server*/
	signal(SIGINT, killServer);

	/*Loop that spawns threads to handle incoming client connections*/
	printf("Server initiated, please use CTRL+C to terminate server\n");
	while(1){
		if(run == 0) break;
		c_socket = accept(s_socket, NULL, NULL);
		
		/*List of created sockets*/
		addSock(socketList, c_socket);	

		client_socket_arg = malloc(sizeof(int));
		memcpy(client_socket_arg, &c_socket, sizeof(int));
		
		/*Spawn a new thread to handle new client*/
		pthread_create(&client, NULL, &server_client_interaction, client_socket_arg);
	}
	return 1;
}	
