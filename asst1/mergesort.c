//Authors: Jeffrey Pham
//Systems Programming Fall 2018

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#include "scannerCSVsorter.h"

//A recursive mergesort that is traditional in the sense that it takes in an array and a low and high int
//Added parameters are columnIndex to be able to compare the values of 2 array indexes and type of variable to use the correct comparison function 
void mergeSortRecords(record *recordArray, int low, int high, int columnIndex, int type){
	int mid = (low+high)/2;
	
	if(low < high) {
		mergeSortRecords(recordArray, low, mid, columnIndex, type);
		mergeSortRecords(recordArray, mid+1, high, columnIndex, type);
		mergeRecords(recordArray, low, mid, mid+1, high, columnIndex, type);
	}
}

//Function to merge the smaller arrays that are created from recursively calling mergeSortRecords
void mergeRecords(record *recordArray,int lowOne,int lowTwo,int highOne,int highTwo,int columnIndex, int type) {
	
	record *tempArray = malloc(recordArray[0].rows * sizeof(record));
	int i = lowOne;
	int j = highOne;
	int k = 0;
	
	//if the type is of string, use strcmp to compare the two values(which are trimmed) at the array index and sort accordingly
	if (type == 0){
		while (i<=lowTwo && j<=highTwo) {
			if((strcmp(trim(recordArray[i].line[columnIndex]), trim(recordArray[j].line[columnIndex])) < 0) || strcmp(trim(recordArray[i].line[columnIndex]), trim(recordArray[j].line[columnIndex])) == 0 ) {
				tempArray[k++] = recordArray[i++];
			} else {
				tempArray[k++] = recordArray[j++];
			}
		}
	}
	//if the type is of int, use atoi to cast the values at the array indexes to ints to be able to compare and sort accordingly
	else if(type == 1){
		while (i<=lowTwo && j<=highTwo) {
			int numOne = atoi(trim(recordArray[i].line[columnIndex]));
			int numTwo = atoi(trim(recordArray[j].line[columnIndex]));
			if(numOne < numTwo || numOne == numTwo) {
				tempArray[k++] = recordArray[i++];
			} else {
				tempArray[k++] = recordArray[j++];
			}		
		}
	}
	//if the type is of double, use atof to cast values at the array indexes to doubles to be able to compare and sort accordingly
	else if(type == 2){
		while (i<=lowTwo && j<=highTwo) {
			int numOne = atof(trim(recordArray[i].line[columnIndex]));
			int numTwo = atof(trim(recordArray[j].line[columnIndex]));
			if( numOne < numTwo || numOne == numTwo) {
				tempArray[k++] = recordArray[i++];
			} else {
				tempArray[k++] = recordArray[j++];
			}
		}		
	}
	
	//when there is only either the left or right subarrays remaining, you can't compare anything anymore, so you just copy the rest of the remaining subarray
	while(i<=lowTwo) {
		tempArray[k++] = recordArray[i++];
	}
	
	while(j<=highTwo) {
		tempArray[k++] = recordArray[j++];
	}
	
	//copy the sorted array to our recordsArray
	for(i=lowOne, j=0; i<=highTwo; i++, j++) {
		recordArray[i] = tempArray[j];
	}  	
	
}	 
