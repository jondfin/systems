#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>	
void fileHandler(char *filename, char *colname){
	 if(strstr(filename, ".csv")){
  		 FILE *fp;
        	 if( (fp = fopen(filename, "r")) == NULL){
        	 	fprintf(stderr, "Cannot open file, %s\n", strerror(errno));
        	 	return;
        	 }
        	 

        	 char *line = (char*)malloc(1024*sizeof(char));
        	 int numCols = 0;
        	 int sortCol = 0;
        	 int found = 0;
        	 int lineCount = 0;

        	 /*Get line containing headers*/
        	 fgets(line, 1024, fp);
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
		sortCol = sortCol;
        	 if(!found){
        	         fprintf(stderr, "Column not found\n");
        	         exit(0);
        	 }

        	 /*Get the number of lines in the file*/
        	 while(fgets(line, 1024, fp)){
        	         lineCount++;
        	 }
        	 /*Rewind the file pointer and advance one line*/
        	 rewind(fp);
        	 fgets(line, 1024, fp);
		printf("%d\n", lineCount);
        	 /*Create an array of nodes where each index is a line and each node is a column*/
        	// Node **arr = (Node **)malloc(linecount*sizeof(Node*));
        	// int index = 0;
        	 /*Read rest of the file and tokenize it*/
        	 while(fgets(line, 1024, fp)){
        	         //TODO Merge Jeff's parser
        	         printf("%s", line);
        	 }
        	// mergesort(arr, 0, lineCount-1, sortCol, numCols-1);
        	// if(writeFile(arr, lineCount, filename) < 0){
        	// 	return
        	// }
        	//free(line);
		//int i;
		//for(i = 0; i < linecount; i++){
		//	freeNode(arr[i]);
		//}
		//free(arr);
		return;
	}else return;
}

int main(int argc, char **argv){

	char *filename = "extra.csv";
	char *colname = "movie_title";

	fileHandler(filename, colname);
	return 0;
}

