#ifndef BANKINGSERVER_H
#define BANKINGSERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>
#include <semaphore.h>
	typedef struct account {
		char name[256];
		double balance;
		int inSession;     
	}account;
	typedef struct Node{
		account *acc;
		struct Node *next;
	}Node;
	typedef struct sock{
		int sockID;
		struct sock *next;
	}sock;	

	int hash(char *name);
	Node *insert(int key, account *a);
	int search(int key, char *name);
	int create(char accountName[256]);

	void print15(int sig);
	sock *addSock(sock *front, int sid);
	void freeNode(Node *front);
	void freeSockList(sock *front);
	void killServer(int sig);

	void *server_client_interaction(void *arg);
	void *server_To_User();
	void *user_To_Server();
	int isValidFloat(char *num);
#endif
