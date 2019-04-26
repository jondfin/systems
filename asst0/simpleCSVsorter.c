#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simpleCSVsorter.h"

int main(int argc, char **argv){
	/*Check for correct parameters*/
	if(argc < 2){
		fprintf(stderr, "Too few arguments. Exiting\n");
		exit(EXIT_FAILURE);
	}   
	if(argc > 3){
		fprintf(stderr, "Too many arguments. Exiting\n");
		exit(EXIT_FAILURE);
	}
	if(strcmp(argv[1], "-c") != 0){
		fprintf(stderr, "Second argument must be '-c'. Exiting.\n");
		exit(EXIT_FAILURE);
	}
	/*Allocate space for a line, max of 1024 characters*/
	char *line = (char *)malloc(1024*sizeof(char));
	/*Number of columns in file*/
	int numCols = 0;
	/*Column to be sorted*/
	int sortCol = 0;	
	int found = 0;
	/*Number of lines in file*/
	int lineCount = 0;
	
	/*Read the first line to get column headers*/
	fgets(line, 1024, stdin);
	/*Store headers in a buffer node*/
	BN *headers = NULL;
	headers = BNinsert(headers, line);
	/*If the line is empty, do nothing*/
	if(strcmp(line, "") == 0) return 0;
	char *token = strtok(line, ",\n\r");
	while(token != NULL){
		/*Find the index of the column to be sorted*/
		if(strcmp(token, argv[2]) == 0){
			sortCol = numCols;
			found = 1;
		}
		numCols++;
		token = strtok(NULL, ",\n\r");
	}
	if(!found){
		fprintf(stderr, "Column not found\n");
		exit(-1);
	}
	printf("%s", headers->data);
	/*Create linked list to store each line (unsorted, not parsed)*/
	BN *buffer = NULL;
	/*Read in the data and store it*/
	while(fgets(line, 1024, stdin)){
		/*Each index is a line*/
		buffer = BNinsert(buffer, line);
		lineCount++;
		line = realloc(line, 1024);
	}

	/*Allocate space for a node array to hold sorted data*/
	Node **arr = (Node **)malloc(lineCount*sizeof(Node *));

	/*Parse through the lines and separate the values. Store each value in arr*/
	int index = 0;
	BN *ptr = buffer;
	while(ptr->next != NULL){
		getToken(arr, ptr->data, index);
		index++;
		ptr = ptr->next;
	}
	getToken(arr, ptr->data, index);

	/*Sort the data*/
	mergeSort(arr, 0, lineCount - 1, sortCol, numCols - 1);
	/*Print out data*/
	printList(arr, lineCount);

	/*Free allocated memory and pointers*/
	free(line);
	freeBNode(headers);
	freeBNode(buffer);
	int i;
	for(i = 0 ; i < lineCount - 1; i++){
		freeNode(arr[i]);
	}
	free(arr);
	return 0;
}
void printList(Node **arr, int lineCount){
	/*Print out the data*/
	int i;
	for(i = 0; i < lineCount; i++){
		Node *ptr = arr[i];
		while(ptr->next != NULL){
			printf("%s,", ptr->data);
			ptr = ptr->next;
		}
		printf("%s", ptr->data);
	}
	return;
}
void freeNode(Node *head){
	/*Recursively free nodes*/
	if(head->next == NULL) free(head);
	else freeNode(head->next);
}
void freeBNode(BN *head){
	/*Recursively free buffer nodes*/
	if(head->next == NULL) free(head);
	else freeBNode(head->next);
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
	}
	else{
		while(ptr->next != NULL){
			ptr = ptr->next;
		}
		ptr->next = node;
	}
	return front;
}

BN *BNinsert(BN *front, char line[1024]){
	/*Create a new buffer node, fill it with data, then insert it*/
	BN *node = (BN *)malloc(sizeof(BN));
	strcpy(node->data, line);
	node->next = NULL;
	BN *ptr = front;
	if(front == NULL){
		node->next = NULL;
		front = node;
	}
	else{
		while(ptr->next != NULL){
			ptr = ptr->next;
		}
		ptr->next = node;
	}
	return front;
}
void getToken(Node **arr, char *line, int index){
	/*Move through each character in a line to separate words*/
	int i;
	int k = 0;
	char prev = '\0';
	/*Buffer to hold entries*/
	char token[256];
	memset(token, 0, 256);
	for(i = 0; i < strlen(line); i++){
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
