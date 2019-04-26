#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
//#include "scannerCSVsorter.h"

int pidCount = 0;
void fileHandler(char *filename, char *colname);
int dirHandler(char *dirname, char *outputdir, char *colname, int ppid);
//TODO
//void getToken(Node **arr, char *line, int index);
//Node *insert(Node *front, char token[256]);
//void freeNode(Node *front);
//void writeFile(Node **arr, int lineCount, char *filename);

int main(int argc, char **argv){
	int ppid = getpid();
	printf("Initial PID: %d\n", ppid);
//	use ulimit -S -u <proc limit> to prevent fork bombs
	dirHandler(".", "memes", "names", ppid);
	printf("pidCount:%d\n", pidCount);
	//printf("Total number of processes: %d\n", pidCount);
	return 0;
}
int dirHandler(char *dirname, char *outputdir, char *colname, int ppid){
	int status;
	/*Append '/' to dirname*/
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
//			printf("Name: %s\n", d->d_name);
			/*Call the handlers if the process is a child*/
			if(d->d_type == DT_DIR){
				int pid = fork();
				/*Ignore outputdir*/
				if(strcmp(d->d_name, outputdir) == 0) continue;
//				printf("Found dir %s\n", d->d_name);

				/*Append found dirname to current dirname*/
				name = realloc(name, length+2 + strlen(d->d_name));
				name = strcpy(name, dirname);
				name[length] = '/';
				name[length+1] = '\0';
				name = strcat(name, d->d_name);
//				printf("name %s\n", name);

				if(pid == 0){	
					pidCount++;
					/*In child proc*/
					status = dirHandler(name, outputdir, colname, ppid);
					exit(1);
				}
				else if(pid < 0){
					/*Error creating child proc*/
					fprintf(stderr, "Cannot create child process\n");
					exit(0);
				}else{
					/*In parent proc*/
//					pid_t wpid = wait(&status);
//					pidCount += WEXITSTATUS(status);
//					if(wpid > 0) printf("%d ", (int)wpid);
//					fflush(stdout);
				}
				/*Reset name*/
				name = strcpy(name, dirname);
				
			}
			else if(d->d_type == DT_REG){
				int pid = fork();
//				printf("Found file %s\n", d->d_name);

				/*Append found filename to current dirname*/
				name = realloc(name, length+2 + strlen(d->d_name));
				name = strcpy(name, dirname);
				name[length] = '/';
				name[length+1] = '\0';
				name = strcat(name, d->d_name);
//				printf("name %s\n", name);

				if(pid == 0){
					pidCount++;
					/*In child proc*/
					fileHandler(name, colname);
					exit(1);
				}
				else if(pid < 0){
					/*Error creating child proc*/
					fprintf(stderr, "Cannot create child process\n");
					exit(0);
				}else{
					/*In parent proc*/
//					pid_t wpid = wait(&status);
//					pidCount += WEXITSTATUS(status);
//					if(wpid > 0) printf("%d ", (int)wpid);
//					fflush(stdout);
				}
				/*Reset name*/
				name = strcpy(name, dirname);	
			}
		}
	}
	free(name);
	closedir(dir);
	int w;
	while((w = wait(&status)) > 0){
//		pidCount += 1; //WEXITSTATUS(status);
		printf("%d\n", w);
	}
//	pid_t wpid;
//	while( (wpid = wait(&status)) > 0){
//		pidCount += WEXITSTATUS(status);
//		if(wpid > 0) printf("%d ", (int)wpid);
///		fflush(stdout);
//	}
	return 1;
}

void fileHandler(char *filename, char *colname){
	/*Check if the file is a .csv file*/
	if(strstr(filename, ".csv")){
//		printf("%s is a .csv\n", filename);
		/*Check if the file is an already sorted .csv file*/
		if(strstr(filename, "-sorted-") != NULL) return;
//		FILE *fp;
//		if((fp = fopen(filename, "r")) == NULL){
//			fprintf(stderr, "Cannot open file, %s\n", strerror(errno));
//			exit(0);
//		}
//		
//		/*Allocate space for a line, max of 1024 chars*/
//		char *line = (char *)malloc(1024*sizeof(char));
//		int numCols = 0;
//		int sortCol = 0;
//		int found = 0;
//		int lineCount = 0;
//		int headerCount = 0;
//
//		/*Get line containing headers*/
//		fgets(line, 1024, fp);
//		if(strcmp(line, "") == 0) return 0;
//		/*Get the number of headers and look for sorting column*/
//		char *token = strtok(line, ",\n\r");
//		while(token != NULL){
//			if(strcmp(token, argv[2]) == 0){
//				sortCol = numCols;
//				found = 1;
//			}
//			numCols++;
//			token = strtok(NULL, ",\n\r");
//		}
//		if(!found){
//			fprintf(stderr, "Column not found\n");
//			exit(0);
//		}
//		
//		/*Get the number of lines in the file*/
//		while(fgets(line, 1024, fp)){
//			lineCount++;
//		}
//		/*Rewind the file pointer and advance one line*/
//		rewind(fp);
//		fgets(line, 1024, fp);
//		
//		/*Create an array of nodes where each index is a line and each node is a column*/
//		Node **arr = (Node **)malloc(linecount*sizeof(Node*));
//		int index = 0;
//		/*Read rest of the file and tokenize it*/
//		while(fgets(line, 1024, fp)){
//			getToken(arr, line, index);
//		}
//		mergesort(arr, 0, lineCount-1, sortCol, numCols-1);
//		writeFile(arr, lineCount, filename);
//
//		free(line);
//		int i;
//		for(i = 0; i < lineCount-1; i++){
//			freeNode(arr[i]);
//		}
//		free(arr);
//
//
//		//don't forget to check if csv file is correctly formatted
	}else return;
	
}

//void getToken(Node **arr, char *line, int index){
//	/*Move through each character in a line to separate words*/
//	int i;
//	int k = 0;
//	char *prev = '\0';
//	/*Buffer to hold entries*/
//	char token[256];
//	memset(token, 0, 256);
//	for(i = 0; i < strlen(line); i++){
//		/*If there is a comma in a title (distinguished by quotes)*/
//		if(line[i] == '\"'){
//			i++;
//			k++;
//			while(line[i] != '\"'){
//				token[k] = line[i];
//				i++;
//				k++;
//			}
//			token[k] = '\"';
//			arr[index] = insert(arr[index], token);
//			i++;
//			k = 0;
//			memset(token, 0, 256);
//		}else{
//			/*If a comma is found after a word*/
//			if(line[i] == ',' || line[i] == '\0'){
//				arr[index] = insert(arr[index], token);
//				k = 0;
//				memset(token, 0, 256);
//			}
//			/*If a comma is the last*/
//			else if(line[i] == '\0' && prev == ','){
//				arr[index] = insert(arr[index], token);
//				k = 0;
//				memset(token, 0, 256);
//			}
//			/*Append chars untila comma or null byte is read*/
//			else if(line[i] != ',' && line[i] != '\0'){
//				token[k] = line[i];
//				k++;
//			}
//		}
//	}
//	arr[index] = insert(arr[index], token);
//	return;
//}
//int writeFile(Node **arr, int lineCount, char *filename){
//	FILE *fp;
//
//	if(! (fp = fopen(filename, "w+"))){
//		fprintf(stderr, "File could not be opened, %s\n", strerror(errno));
//		return -1;
//	}
//	int i;
//	for(i = 0; i < lineCount; i++){
//		Node *ptr = arr[i];
//		while(ptr->next != NULL){
//			fprintf(fp, "%s,", ptr->data);
//			ptr = ptr->next;
//		}
//		fprintf(fp, "%s\n", ptr->data);
//	}
//	fclose(fp);
//	return 0;
//}
//void freeNode(Node *front){
//	if(front->next == NULL) free(head);
//	else freeNode(head->next);
//}
//Node *insert(Node *front, char token[256]){
//	/*Create a new node, fill it with data, then insert it*/
//	Node *node = (Node *)malloc(sizeof(Node));
//	strcpy(node->data, token);
//	node->next = NULL;
//	Node *ptr = front;
//	if(front == NULL){
//		node->next = NULL;
//		front = node;
//	}else{
//		while(ptr->next != NULL){
//			ptr = ptr->next;
//		}
//		ptr->next = node;
//	}
//	return front;
//}
	
