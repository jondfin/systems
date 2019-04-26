#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
#include "scannerCSVsorter.h"

int main(int argc, char **argv){

//	char buf[256];
	char *startdir = "temp";
	char *outputdir = "temp";
	char *colname = "temp";
	/*Grab the parent process id*/
	int ppid = getpid();
	printf("Initial PID: %d\nPIDS of all child processes: ", ppid);
	fflush(stdout);

	/*Set up a counter in memory to keep track of the number of recursive calls*/
	int *count;
	count = (int*)malloc(sizeof(int));
	*count = 0;

	/*Remember to set ulimit -S -u 256 to avoid fork bombing*/
	dirHandler(startdir, outputdir, colname, ppid, count);

	free(count);
	return 0;
}
void dirHandler(char *dirname, char *outputdir, char *colname, int ppid, int *count){
	int status;
	/*Append '/' to dirname*/
  	static int procNum = 0;
	int length = strlen(dirname);
	char *name = (char *)malloc(length + 2);
	name = strcpy(name, dirname);
	name[length] = '/';
	name[length+1] = '\0';

	/*Open dir and read*/
	DIR *dir;
	if(!(dir = opendir(dirname))){
		fprintf(stderr, "Cannot open directory, %s\n", strerror(errno));
		exit(0);
	}
	struct dirent *d;
	while((d = readdir(dir)) != NULL){
		if(strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0){
			/*Do nothing*/
		}else{
			/*Call the handlers if the process is a child*/
			if(d->d_type == DT_DIR){
				int pid = fork();
				
				/*Append found dirname to current dirname*/
				name = realloc(name, length+2 + strlen(d->d_name));
				name = strcpy(name, dirname);
				name[length] = '/';
				name[length+1] = '\0';
				name = strcat(name, d->d_name);

				if(pid == 0){
					/*In child proc*/
					(*count)++;
					procNum++;
					/*Ignore outputdir*/
					if(strcmp(d->d_name, outputdir) == 0){
						exit(procNum);
					}
					dirHandler(name, outputdir, colname, ppid, count);
					exit(*count + procNum);
				}
				else if(pid < 0){
					/*Error creating child proc*/
					fprintf(stderr, "Cannot create child process\n");
					exit(0);
				}else{
					/*In parent proc*/
					*count = 0;
				}
				/*Reset name*/
				name = strcpy(name, dirname);
			}
			else if(d->d_type == DT_REG){
				int pid = fork();

				/*Append found filename to current dirname*/
				name = realloc(name, length+2 + strlen(d->d_name));
				name = strcpy(name, dirname);
				name[length] = '/';
				name[length+1] = '\0';
				name = strcat(name, d->d_name);

				if(pid == 0){
					/*In child proc*/
					procNum++;
					fileHandler(name, colname);
					exit(procNum);
				}
				else if(pid < 0){
					/*Error creating child proc*/
					fprintf(stderr, "Cannot create child process\n");
					exit(0);
				}else{
					/*In parent proc*/
				}
				/*Reset name*/
				name = strcpy(name, dirname);	
			}
		}
	}
	free(name);
	closedir(dir);

	/*Print out the pids of all the children*/
	pid_t wpid;
	int c = 0;

	/*Wait on the forked children*/
	while((wpid = wait(&status)) > 0){
		procNum++;
		c += WEXITSTATUS(status);
		printf("%d, ", (int)wpid);
	}
	fflush(stdout);
	if(getpid() == ppid){
		printf("\nTotal number of processes: %d\n", c);
		fflush(stdout);
	}
}
	
void fileHandler(char *filename, char *colname){
	/*Check if the file is a .csv file*/
	if(strstr(filename, ".csv")){
		/*Check if the file is an already sorted .csv file*/
		if(strstr(filename, "-sorted-") != NULL){
			fprintf(stderr, "Already sorted\n");	
			return;
		}
		FILE *fp;
		if((fp = fopen(filename, "r")) == NULL){
			fprintf(stderr, "Cannot open file, %s\n", strerror(errno));
			return;
		}
		
		/*Allocate space for a line, max of 1024 chars*/
		char *line = (char *)malloc(1024*sizeof(char));
		int numCols = 0;
		int sortCol = 0;
		int found = 0;
		int lineCount = 0;
	
		/*Get line containing headers*/
		fgets(line, 1024, fp);
	
		/*Empty file, return*/
		if(strcmp(line, "") == 0) return;
	
		/*Get the number of headers and look for sorting column*/
		char *token = strtok(line, ",\n\r");
		while(token != NULL){
			if(strcmp(token, colname) == 0){
				sortCol = numCols;
				found = 1;
			}
			numCols++;
			token = strtok(NULL, ",\n\r");
		}
		
		if(!found){
			fprintf(stderr, "Column not found\n");
			return;
		}
		
		/*Get the number of lines in the file*/
		while(fgets(line, 1024, fp)){
			lineCount++;
		}
		/*Rewind the file pointer and advance one line*/
		rewind(fp);
		fgets(line, 1024, fp);
		
		/*Create an array of nodes where each index is a line and each node is a column*/
		Node **arr = (Node **)malloc(lineCount*sizeof(Node*));
		int index = 0;
		/*Read rest of the file and tokenize it*/
		while(fgets(line, 1024, fp)){
			getToken(arr, line, index);
			index++;				
		}
		mergesort(arr, 0, lineCount-1, sortCol, numCols-1);
		if(writeFile(arr, lineCount, filename) < 0) return;
	
		int i;
		for(i = 0; i < lineCount-1; i++){
			freeNode(arr[i]);
		}
		free(arr);
		free(line);
	}else{
		/*Not a .csv file, return*/
		return;
	}
}

void getToken(Node **arr, char *line, int index){
	/*Move through each character in a line to separate words*/
	int i;
	int k = 0;
	char prev = '\0';
	/*Buffer to hold entries*/
	char token[256];
	memset(token, 0, 256);
	for(i  = 0; i < strlen(line); i++){
                /*If there is a comma in a title (disinguished by quotes)*/
                if(line[i] == '\"'){
                        token[0] = '\"';
                        i++;
                        k++;
                        while(line[i] != '\"'){
                                token[k] = line[i];
                                i++;
                                k++;
                        }
                        token[k] = '\"';
                        arr[index] = insert(arr[index], token);
                        i++;
                        k = 0;
                        memset(token, 0, 256);
                }else{
                        /*If a comma is found after a word*/
                        if(line[i] == ',' || line[i] == '\0'){
                                arr[index] = insert(arr[index], token);
                                k = 0;
                                memset(token, 0, 256);
                        }
                        /*If a comma is the last character*/
                        if(line[i] == '\0' && prev == ','){
                                arr[index] = insert(arr[index], token);
                                k = 0;
                                memset(token, 0, 256);
                        }
                        /*Append chars until a comma or null byte is read*/
                        if(line[i] != ',' && line[i] != '\0'){
                                token[k] = line[i];
                                k++;
                        }
                }
        }
        arr[index] = insert(arr[index], token);
	return;
}

int writeFile(Node **arr, int lineCount, char *filename){
	/*Create the sorted .csv file*/
	FILE *fp;
	if(! (fp = fopen(filename, "w+"))){
		fprintf(stderr, "File could not be opened, %s\n", strerror(errno));
		return -1;
	}
	/*Loop that iterates through the array and writes to the file line by line*/
	int i;
	for(i = 0; i < lineCount; i++){
		Node *ptr = arr[i];
		while(ptr->next != NULL){
			fprintf(fp, "%s, ", ptr->data);
			ptr = ptr->next;
		}
		fprintf(fp, "%s\n", ptr->data);
	}
	fclose(fp);
	return 0;
}

void freeNode(Node *front){
	/*Recursivel free nodes*/
	if(front->next == NULL) free(front);
	else freeNode(front->next);
}
Node *insert(Node *front, char token[256]){
	/*Create a new node, fill it with data, then insert it*/
	Node *node = (Node *)malloc(sizeof(Node));
	strcpy(node->data, token);
	node->next = NULL;
	Node *ptr = front;
	if(front == NULL){
		node->next = NULL;
		front = node;
	}else{
		while(ptr->next != NULL){
			ptr = ptr->next;
		}
		ptr->next = node;
	}
	return front;
}
	
