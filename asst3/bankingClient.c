#include "bankingServer.h"

pthread_t fromServer, toServer;
int server_socket;

void* server_To_User(){

	char response[256];
	memset(response, 0, sizeof(response));

	while(read(server_socket, response, 255) > 0){
		if(strcmp(response, "DIE") == 0){
			printf("Server is shutting down. Disconnecting...\n");	
			break;
		}
		else printf("%s\n", response);
		memset(response, 0, sizeof(response));
	}
        printf("\n\tThank you for using the banking system\n\tThere is no longer a connection to the server\n\t\n");
	pthread_detach(fromServer);
	exit(EXIT_SUCCESS);
}

void* user_To_Server(){
	
	/*Buffer is large enough to hold sizeof(cmd) + sizeof(param)[<=256]*/	
	char inputBuffer[512];
	memset(inputBuffer, 0, sizeof(inputBuffer));
	
	printf("COMMANDS:\n\tcreate\n\tserve\n\tdeposit\n\twithdraw\n\tquery\n\tend\n\tquit\n");
	char validCommands[] = "create,serve,deposit,withdraw,query,end,quit";
	
	while(1){
		char cmd[20];
		char param[256];
		int noWrite = 0;

		/*Read user input. Parse and store into separate variables*/
		fgets(inputBuffer, 512, stdin);
		sscanf(inputBuffer, "%s %s", cmd, param);

		if(strstr(validCommands,cmd) == NULL){
			printf("Error: Not a valid command\n\n");
		}
		else if(strcmp(cmd, "deposit") == 0 || strcmp(cmd,"withdraw") == 0){
			if(!isValidFloat(param)){
				printf("Error: Input is not a number\n\n");
				noWrite = 1;
			}
			else if(atof(param) < 0){
				printf("Error: You must input a non-negative number\n\n");
				noWrite = 1;
			}
		}

		if(noWrite == 0) write(server_socket, inputBuffer, sizeof(inputBuffer));
		sleep(2);
		memset(inputBuffer, 0, sizeof(inputBuffer));
	}

	pthread_exit(0);
}

//Determines whether the input is a valid float number. Returns 1 if true, 0 if false.
int isValidFloat(char *num){
	int decimal = 0;
        int i;
        for(i = 0 ; i < strlen(num) ; i++){
    		if(isdigit(num[i]) == 0 || num[0] == '-'){
			if(num[i] == '.'){
    				decimal++;
                		if(decimal > 1){
    					return 0;
		                }
        	        }
            	}
        }
        return 1;   
}

int main(int argc, char* argv[]){
	
	//error checking
        if(argc < 3){
            printf("ERROR: Must  specify the name of the machine and port you are trying to connect to\n");
            exit(1);
        }
    
        // check validity of host computer server
        if (gethostbyname(argv[1]) == NULL){
            printf("ERROR: This is not a valid machine\n");
            exit(1);
        }
        
        if(atoi(argv[2]) < 8192){
    		printf("ERROR: Please input a port number greater than 8k(8192).\n");
    		exit(1);
    	}
        
        //addrinfo needed for socket communication
        struct addrinfo addr;
        addr.ai_flags = 0;
        addr.ai_family = AF_INET;
        addr.ai_socktype = SOCK_STREAM;
        addr.ai_protocol = 0;
        addr.ai_addrlen = 0;
        addr.ai_canonname = NULL;
        addr.ai_next = NULL;    
        struct addrinfo *addr2;
    
        getaddrinfo(argv[1], argv[2], &addr, &addr2 );
    
        //server socket
    	int s_socket = socket(addr2->ai_family, addr2->ai_socktype, addr2->ai_protocol);
    	server_socket = s_socket;
    	void *c_socket = malloc(sizeof(s_socket));
    	memcpy(c_socket, &s_socket, sizeof(int));
        //connecting to server socket
        int connected = connect(s_socket, addr2->ai_addr, addr2->ai_addrlen); 
        while(connected < 0){
            printf("Unable to find server. Please check that the machine name and port number are correct. Will attempt to reconnect in 3 seconds.\n");
            sleep(3);
            connected = connect(s_socket, addr2->ai_addr, addr2->ai_addrlen);
        }
    
        printf("\n\tYou are now connected to the banking server.\n");

	//thread to receive messages from a the server
	if(pthread_create(&fromServer, NULL, server_To_User, NULL) < 0){
		printf("Error: output thread was not created\n");
		exit(0);
	}
	

	//thread to send user input to server	
	if(pthread_create(&toServer, NULL, user_To_Server, NULL) < 0){
		printf("Error: input thread was not created\n");
		exit(0);
	}
	
	pthread_join(fromServer, NULL);
	pthread_join(toServer, NULL);

	return 0;
}
