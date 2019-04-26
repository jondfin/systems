#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include <pthread.h>
#include "multiThreadSorter.h"

int threadCount = 0;
int lineCount = 0;
/*idList is the list of pthread_t ids, TID list are the artificial TIDs*/
Node *idList = NULL;
Node *TIDlist = NULL;
Node *buffer = NULL;
pthread_mutex_t m1, m2;

int main(int argc, char **argv){
	//TODO
	//Check for correctly formatted command line arguments
	//-c -d -o
	//-c -o -d
	//-d -c -o
	//-d -o -c
	//-c -d -o
	//-c -o -d
	
	//Dummy values, temporary
	char *dirname = ".";
	char *outputdir = ".";
	char *colname = "director_name";

	/*Initialize mutexes*/
	if(pthread_mutex_init(&m1, NULL) != 0){
		fprintf(stderr, "Error initializing mutex 1: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(pthread_mutex_init(&m2, NULL) != 0){
		fprintf(stderr, "Error initializing mutex 2: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/*Initialize arguments*/
	args *a = (args*)malloc(sizeof(args));
	a->name = dirname;
	a->outputdir = outputdir;
	a->colname = colname;
	a->dirLevel = 0;

	dirHandler(a);

	/*Join the threads*/
	Node *ptr = idList;
	while(ptr != NULL){
		if(pthread_join(ptr->TID, NULL) != 0){
			fprintf(stderr, "Error joining pthread %lu: %s\n", ptr->TID, strerror(errno));
			fprintf(stderr, "Cancelling thread %lu\n", ptr->TID);
			pthread_cancel(ptr->TID);
		}
		ptr = ptr->next;
	}

	/*Nothing was read in*/
	if(buffer == NULL){
//		printf("No files read. Exiting\n");
		free(a);
		freeNode(idList);
		exit(EXIT_SUCCESS);
	}

	/*Input functions should be done by now. Sort the overall data structure*/
	/*Allocate space for a node array to hold sorted data*/
	Node **arr = (Node**)malloc(lineCount*sizeof(Node*));
	
	/*Parse through the lines and separate the values. Store each value in arr*/
	ptr = buffer;
	int index = 0;
	while(ptr->next != NULL){
		getToken(arr, ptr->line, index, ptr->binArr);
		index++;
		ptr = ptr->next;
	}
	getToken(arr, ptr->line, index, ptr->binArr);

	/*Get the column to be sorted on*/
	char validMovieHeadings[] = "color,director_name,num_critic_for_reviews,duration,director_facebook_likes,actor_3_facebook_likes,actor_2_name,actor_1_facebook_likes,gross,genres,actor_1_name,movie_title,num_voted_users,cast_total_facebook_likes,actor_3_name,facenumber_in_poster,plot_keywords,movie_imdb_link,num_user_for_reviews,language,country,content_rating,budget,title_year,actor_2_facebook_likes,imdb_score,aspect_ratio,movie_facebook_likes";
	char *token = strtok(validMovieHeadings, ",\n\r");
	int sortCol = 0;
	while(token != NULL){
		if(strcmp(token, colname) == 0) break;
		sortCol++;
		token = strtok(NULL, ",\n\r");
	}

	mergeSort(arr, 0, lineCount - 1, sortCol, 27);

	/*Write sorted data to file*/
	writeToFile(arr, colname, outputdir);

	/*Destroy mutexes*/
	if(pthread_mutex_destroy(&m1) != 0){
		fprintf(stderr, "Error destroying mutex 1: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(pthread_mutex_destroy(&m2) != 0){
		fprintf(stderr, "Error destroying mutex 2: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/*Print the final set of metadata*/
	printf("Initial PID: %d\n", getpid());
	printf("TIDs of all spawned threads: ");
	ptr = TIDlist;
	int i = 0;
	while(ptr != NULL){
		printf("%s", ptr->line);
		if(i < threadCount-1) printf(",");
		ptr = ptr->next;
		i++;
	}
	printf("\n");
	printf("Total number of threads: %d\n", threadCount);

	/*Free memory*/	
	i = 0;
	for(i = 0; i < lineCount - 1; i++){
		freeNode(arr[i]);
	}
	free(arr);
	freeNode(idList);
	free(a);
	return 0;
}
void printMetaData(char *layer, int threadCount){
	/*Print initial PID*/
	printf("Initial PID: %d\n", getpid());
	
	/*Create TIDs. The number of digits reflect the number of subdirectories*/
	int digits = threadCount / 10;
	if(digits == 0) digits = 1;
	char *TID = (char*)malloc((strlen(layer)+digits+1)*sizeof(char));
	int i = 0;
	printf("TIDs of all spawned threads: ");
	for(i = 0; i < threadCount; i++){
		if(threadCount == 1) break;
		char s[digits];
		memset(s, '0', digits);
		sprintf(s, "%d", i);
		TID = strcpy(TID, layer);
		TID = strcat(TID, s);
		TID = strcat(TID, "\0");
		printf("%s", TID);
		if(i < threadCount - 1) printf(",");
		TIDlist = insert(TIDlist, -1, TID, "");
		TID = memset(TID, '0', strlen(TID));
	}
	printf("\n");

	/*Print the number of threads spawned by this thread*/
	printf("Total number of threads: %d\n", threadCount);
	free(TID);
	return;
}
void writeToFile(Node **arr, char *colname, char *outputdir){
	/*Create file*/
	char *filename = (char*)malloc((strlen(colname)+21)*sizeof(char));
	filename = strcpy(filename, "AllFiles-sorted-");
	filename = strcat(filename, colname);
	filename = strcat(filename, ".csv\0");
//	char *finalFileName = NULL;

	/*If outputdir was specified, send there, else output to cwd*/
//	int newFileNameLength = strlen(outputdir) + strlen(filename) + 1;
//	finalFileName = (char*)malloc(newFileNameLength*sizeof(char)+1);
//	strcat(finalFileName, outputdir);
//	strcat(finalFileName, "/");
//	strcat(finalFileName, filename);

	FILE *fp;
	fp = fopen(filename, "w+");
	if(!fp){
		fprintf(stderr, "Couldn't create %s: %s\n", filename, strerror(errno));
		free(filename);
		exit(EXIT_FAILURE);
	}
	
	/*Write headers to file*/
	char validMovieHeadings[] = "color,director_name,num_critic_for_reviews,duration,director_facebook_likes,actor_3_facebook_likes,actor_2_name,actor_1_facebook_likes,gross,genres,actor_1_name,movie_title,num_voted_users,cast_total_facebook_likes,actor_3_name,facenumber_in_poster,plot_keywords,movie_imdb_link,num_user_for_reviews,language,country,content_rating,budget,title_year,actor_2_facebook_likes,imdb_score,aspect_ratio,movie_facebook_likes";
	fprintf(fp, "%s\n", validMovieHeadings);
	
	/*Write the lines to file*/
	int i;
	for(i = 0; i < lineCount; i++){
		Node *ptr = arr[i];
		while(ptr->next != NULL){
			fprintf(fp, "%s,", ptr->line);
			ptr = ptr->next;
		}
		fprintf(fp, "%s", ptr->line);
	}

	free(filename);
	fclose(fp);
	return;
}

void *dirHandler(void *Args){
	DIR *dir;
	struct dirent *d;
	pthread_t dirThread, fileThread;
	args *a = (args*)Args;
	char *name;
	int digits = a->dirLevel / 10;
	if(digits == 0) digits = 1;
	char *layer = (char*)malloc(digits*sizeof(char)+1);
	sprintf(layer, "%d", a->dirLevel);
	int tCount = 0;

	if( (dir = opendir(a->name)) == NULL){
		fprintf(stderr, "Cannot open directory %s: %s\n", a->name, strerror(errno));
		return NULL;
	}
	

	while((d = readdir(dir)) != NULL){
		if(d->d_type == DT_DIR){
			if(strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0){
				/*Do nothing*/
			}else{
				/*Fix current pathname and append new directory*/
				if(a->name[strlen(a->name)-1] != '/'){
					name = (char*)malloc((strlen(a->name)+strlen(d->d_name))*sizeof(char)+2);
					name = strcpy(name, a->name);
					name = strcat(name, "/");
					name = strcat(name, d->d_name);
					name = strcat(name, "\0");
				}else{
					name = (char*)malloc((strlen(a->name)+strlen(d->d_name))*sizeof(char)+1);
					name = strcpy(name, a->name);
					name = strcat(name, d->d_name);
					name = strcat(name, "\0");
				}
				/*Create new argument list*/
				args *dirArgs = (args*)malloc(sizeof(args));
				dirArgs->name = (char*)malloc(strlen(name)*sizeof(char)+1);
				strncpy(dirArgs->name, name, strlen(name));

				dirArgs->colname = (char*)malloc(strlen(a->colname)*sizeof(char)+1);
				strcpy(dirArgs->colname, a->colname);

				dirArgs->outputdir = (char*)malloc(strlen(a->outputdir)*sizeof(char)+1);
				strncpy(dirArgs->outputdir, a->outputdir, strlen(a->outputdir));

				dirArgs->dirLevel++;

				/*Create a new thread to handle the directory*/
				pthread_create(&dirThread, NULL, dirHandler, (void*)dirArgs);
				tCount++;

				/*Lock to protect globals*/
				pthread_mutex_lock(&m1);
				threadCount++;
				idList = insert(idList, dirThread, "", "");
				pthread_mutex_unlock(&m1);
			}
		}
		else if(d->d_type == DT_REG){
			/*Fix current pathanme and append new filename*/
			if(a->name[strlen(a->name)-1] != '/'){
				name = (char*)malloc((strlen(a->name)+strlen(d->d_name))*sizeof(char)+2);
				name = strcpy(name, a->name);
				name = strcat(name, "/");
				name = strcat(name, d->d_name);
				name = strcat(name, "\0");
			}else{
				name = (char*)malloc((strlen(a->name)+strlen(d->d_name))*sizeof(char)+1);
				name = strcpy(name, a->name);
				name = strcat(name, d->d_name);
				name = strcat(name, "\0");
			}
			/*Create new argument list*/
			args *fileArgs = (args*)malloc(sizeof(args));
			fileArgs->name = (char*)malloc(strlen(name)*sizeof(char)+1);
			strncpy(fileArgs->name, name, strlen(name));
			
			fileArgs->colname = (char*)malloc(strlen(a->colname)*sizeof(char)+1);
			strcpy(fileArgs->colname, a->colname);

			fileArgs->outputdir = (char*)malloc(strlen(a->outputdir)*sizeof(char)+1);
			strncpy(fileArgs->outputdir, a->outputdir, strlen(a->outputdir));

			/*Create a new thread to handle the file*/
			pthread_create(&fileThread, NULL, fileHandler, (void*)fileArgs);
			tCount++;

			printMetaData(layer, 0);

			/*Lock to protect globals*/
			pthread_mutex_lock(&m1);
			threadCount++;
			idList = insert(idList, fileThread, "", "");
			pthread_mutex_unlock(&m1);
		}
	}
	
	printMetaData(layer, tCount);

	closedir(dir);
	free(name);
	free(layer);
	return NULL;
}
	
void *fileHandler(void *Args){
	args *a = (args*)Args;
	/*Check if this is a csv file*/
	if(strstr(a->name, ".csv")){
		/*Check if the AllFiles-sorted- file was already created*/
		char *endFile = (char*)malloc((strlen(a->colname)+21)*sizeof(char));
		endFile = strcpy(endFile, "AllFiles-sorted-");
		endFile = strcat(endFile, a->colname);
		endFile = strcat(endFile, ".csv\0");
		if(strstr(a->name, endFile)){
			fprintf(stderr, "%s was already created, please remove and run again\n", endFile);
			free(endFile);
			exit(EXIT_SUCCESS);
		}
		free(endFile);

		/*Open file*/
		FILE *fp;
		fp = fopen(a->name, "r");
		if(!fp){
			fprintf(stderr, "Cannot open %s: %s\n", a->name, strerror(errno));
			return NULL;
		}
		
		/*Variables to contain file data*/
		char validMovieHeadings[] = "color,director_name,num_critic_for_reviews,duration,director_facebook_likes,actor_3_facebook_likes,actor_2_name,actor_1_facebook_likes,gross,genres,actor_1_name,movie_title,num_voted_users,cast_total_facebook_likes,actor_3_name,facenumber_in_poster,plot_keywords,movie_imdb_link,num_user_for_reviews,language,country,content_rating,budget,title_year,actor_2_facebook_likes,imdb_score,aspect_ratio,movie_facebook_likes";
		char *headers = NULL;
		char *lines = NULL;
		size_t len = 1024;
		int read = 0;

		/*Binary array corresponding to each of the columns in the header. If if a column is present set value to 1 in its corresponding spot in the array*/
		char binArr[28];
		memset(binArr, '1', 28);

		/*Variables for creating overall data structure and sorting*/
		int numCols = 0;
		int found = 0;

		/*Get headers*/
		if((read = getline(&headers, &len, fp) == -1)){
			fprintf(stderr, "Error getting headers from %s: %s\n", a->name, strerror(errno));
			fclose(fp);
			return NULL;
		}

		/*Check the validity of headers*/
		if(strstr(validMovieHeadings, headers) == NULL){
			/*Full headers not listed, check if the headers contained are valid headers*/
			char *saveptr;
			char *token = strtok_r(headers, ",\n\r", &saveptr);
			while(token != NULL){
				if(strstr(validMovieHeadings, token) == NULL){
					/*File contains an invalid header, ignore it*/
					fprintf(stderr, "Invalid header in %s\n", a->name);
					fclose(fp);
					return NULL;
				}
				else if(strcmp(token, a->colname) == 0){
					/*Check to see if the column is present in the file*/
					found = 1;
				}
				numCols++;
				token = strtok_r(NULL, ",\n\r", &saveptr);
			}
			if(numCols > 28){
				/*Too many columns in this file, ignore*/
				fprintf(stderr, "Too many columns in %s\n", a->name);
				fclose(fp);
				return NULL;
			}
			else if(numCols < 28){
				/*Check the partial headers (can be in any order)*/
				char *sptr;
				token = strtok_r(validMovieHeadings, ",\n\r", &sptr);
				int i = 0;
				/*Compare each column from the list of valid movie headings to the read in headers
 				  Set its correspoding entry in the binary array to 1 if matched, 0 otherwise.
				  Use this to account for missing columns in the file*/
				while(token != NULL){
					if(strcmp(token, headers) != 0){
						binArr[i] = '0';
					}else{
						binArr[i] = '1';
					}	
					i++;
					token = strtok_r(NULL, ",\n\r", &sptr);
				}
				/*Fix the file by rearranging columns and inserting nulls*/
				//TODO
				//fixFile(a->name, validMovieHeadings, binArray);
				//return NULL;
			}		
		}
		
		if(found != 1){
			fprintf(stderr, "Column not found in %s\n", a->name);
			fclose(fp);
			return NULL;
		}

		/*Read in the rest of the file*/
		//TODO CHECK FOR VALIDITY OF LINES
		pthread_mutex_lock(&m2);
		while((read = getline(&lines, &len, fp) != -1)){
			buffer = insert(buffer, -1, lines, binArr);
			lineCount++;
		}
		fclose(fp);
		pthread_mutex_unlock(&m2);
	}else{
		/*Not a .csv file*/
		return NULL;
	}
	return NULL;
}

void getToken(Node **arr, char *line, int index, char binArr[28]){
	/*Move through each character ina  line to separate words*/
	int i;
	int j = 0;
	int k = 0;
	char prev = '\0';

	/*Buffer to hold entries*/
	char token[256];
	memset(token, 0, 256);

	/*Move through the line character by character*/
	for(i = 0; i < strlen(line); i++){
		/*Insert null for missing columns*/
		while(binArr[j] == '0'){
			arr[index] = insert(arr[index], -1, "\0", binArr);
			j++;
		}
		/*If there is a comma in a title (distinguished by quotes*/
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
			arr[index] = insert(arr[index], -1, token, binArr);
			j++;
			i++;
			k = 0;
			memset(token, 0, 256);
		}
		else if(line[i] == ',' || line[i] == '\0'){
				/*If a comma is found after a word*/
				arr[index] = insert(arr[index], -1, token, binArr);
				j++;
				k = 0;
				memset(token, 0, 256);
		}
		else if(line[i] == '\0' && prev == ','){
				/*If a comma is the last character*/
				arr[index] = insert(arr[index], -1, token, binArr);
				j++;
				k = 0;
				memset(token, 0, 256);
		}
		else if(line[i] != ',' && line[i] != '\0'){
				/*Append chars until a comma or null byte is read*/
				token[k] = line[i];
				k++;
		}	
	}
	if(strstr(token, "\r")){
		char *cpy = (char*)malloc(strlen(token)*sizeof(char)+1);
		cpy = strncpy(cpy, token, strlen(token)-2);
		cpy = strcat(cpy, "\n\0");
		arr[index] = insert(arr[index], -1, cpy, binArr);
		free(cpy);
	}else{
		arr[index] = insert(arr[index], -1, token, binArr);
	}
	return;
}

/*Insert node into list*/
Node *insert(Node *front, pthread_t tid, char *line, char binArr[28]){
	Node *node = (Node*)malloc(sizeof(Node));
	node->TID = tid;

	node->line = (char*)malloc(strlen(line)*sizeof(char)+1);
	node->line = strcpy(node->line, line);
	node->line = strcat(node->line, "\0");

	strncpy(node->binArr, binArr, 28);

	node->next = NULL;
	if(front == NULL){
		front = node;
		return front;
	}
	Node *ptr = front;
	while(ptr->next != NULL){
		ptr = ptr->next;
	}
	ptr->next = node;
	return front;
}

/*Recursively free nodes*/
void freeNode(Node *front){
	if(front->next != NULL) free(front);
	else freeNode (front->next);
}

