#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "simpleCSVsorter.h"
Node *copy(Node *dest, Node *src){
	/*Copy data from one index to another*/
	Node *p1 = dest;
	Node *p2 = src;
	/*Avoid strcpy param overlap*/
	if(dest == src) return dest;
	while(p2 != NULL){
		strcpy(p1->data, p2->data);
		p1 = p1->next;
		p2 = p2->next;
	}
	return dest;
}
char *trim(char *str){
	while(isspace((unsigned char)*str)){
		str++;
	}
	return str;
}
void merge(Node **arr, int left, int mid, int right, int colNum, int numCols){
	int i, j;
	int count = 0;
	Node *p1, *p2;
	int sizeA1 = mid - left + 1;
	int sizeA2 = right - mid;
	
	/*Separate data between A1 and A2. Left side of array goes into A1, right goes to A2*/
	Node **A1, **A2;
	A1 = (Node **)malloc(sizeA1*sizeof(Node*));
	A2 = (Node **)malloc(sizeA2*sizeof(Node*));
	for(i = 0; i < sizeA1; i++){
		A1[i] = (Node*)malloc(sizeof(Node));
		p1 = A1[i];
		while(count < numCols){
			if(p1->next == NULL){
				p1->next = (Node*)malloc(sizeof(Node));
				count++;
			}
			p1 = p1->next;
		}
		count = 0;
		A1[i] = copy(A1[i], arr[left + i]);
	}
	for(j = 0; j < sizeA2; j++){
		A2[j] = (Node*)malloc(sizeof(Node));
		p2 = A2[j];
		while(count < numCols){
			if(p2->next == NULL){
				p2->next = (Node*)malloc(sizeof(Node));
				count++;
			}
			p2 = p2->next;
		}
		count = 0;
		A2[j] = copy(A2[j], arr[mid + 1 + j]);
	}
	
	/*Compare data*/
	i = 0;
	j = 0;
	int k = left;
	while(i < sizeA1 && j < sizeA2){
		p1 = A1[i];
		p2 = A2[j];
		count = 0;
		/*Get the value of the column*/
		while(count != colNum){
			p1 = p1->next;
			p2 = p2->next;
			count++;
		}
		/*Compare the values of the columns*/
		/*Check if the entry is a digit*/
		if(isdigit(trim(p1->data)[0]) && isdigit(trim(p2->data)[0])){
			/*Convert to double to hold data*/
			double num1 = atof(p1->data);
			double num2 = atof(p2->data);
			if(num1 <= num2){
				arr[k] = copy(arr[k], A1[i]);
				i++;
			}else{
				arr[k] = copy(arr[k], A2[j]);
				j++;
			}
		}else{
			/*Compare values of strings*/
			switch(strcmp(trim(p1->data), trim(p2->data))){
				case -1:/*Less than*/
					arr[k] = copy(arr[k], A1[i]);
					i++;
					break;
				case 0:/*Equal to*/
					arr[k] = copy(arr[k], A1[i]);
					i++;
					break;
				case 1:/*Greater than*/
					arr[k] = copy(arr[k], A2[j]);
					j++;
					break;
			}
		}
		k++;
	}
	/*If an array has leftover data, copy to end of arr*/
	while(i < sizeA1){
		arr[k] = copy(arr[k], A1[i]);
		i++;
		k++;
	}
	while(j < sizeA2){
		arr[k] = copy(arr[k], A2[j]);
		j++;
		k++;
	}

	/*Free allocated memory*/
	for(i = 0; i < sizeA1; i++){
		freeNode(A1[i]);
	}
	for(i = 0; i < sizeA2; i++){
		freeNode(A2[i]);
	}
	free(A1);
	free(A2);
	return;		
}

void mergeSort(Node **arr, int left, int right, int colNum, int numCols){
	int mid = (left + (right-1))/2;
	if(left < right){
		/*Split the data*/
		mergeSort(arr, left, mid, colNum, numCols);
		mergeSort(arr, mid+1, right, colNum, numCols);
		/*Merge split data*/
		merge(arr, left, mid, right, colNum, numCols);
	}
	return;
}
