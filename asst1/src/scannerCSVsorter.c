#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include "scannerCSVsorter.h"

int main(int argc, char **argv){
	//Error checking to make sure all inputs are correct in order to continue on with code
	if(argc < 3){
                fprintf(stderr, "%s\n", "Error: Not enough parameters inputted. Needs at minimum -c and column name to sort on.");
                exit(EXIT_FAILURE);
        }
        if(strcmp(argv[1], "-c") != 0){
                fprintf(stderr, "%s\n", "Error: First parameter must be -c");
                exit(EXIT_FAILURE);
        }
        if(argc > 3 && strcmp(argv[3], "-d") != 0){
                if(strcmp(argv[3], "-o") != 0){
                        fprintf(stderr, "%s\n", "Error: If there is a third parameter must either be -d or -o");
                        exit(EXIT_FAILURE);
                }
        }
        if(argc == 4){
                fprintf(stderr, "%s\n", "Error: Must either specify a starting directory or output directory depending on the third parameter.");
                exit(EXIT_FAILURE);
        }
        if(argc > 5 && strcmp(argv[5], "-o") != 0){
                if (strcmp(argv[5], "-d") != 0){
                        fprintf(stderr, "%s\n", "Error: The fifth parameter must be -d or -o");
                        exit(EXIT_FAILURE);
                }
        }
        if(argc == 6){
                fprintf(stderr, "%s\n", "Error: Must specify an output or starting directory depending on the last parameter inputted.");
                exit(EXIT_FAILURE);
        }
        if (argc == 7){
                if ((strcmp(argv[3],"-d") == 0 && strcmp(argv[5],"-d") == 0) || (strcmp(argv[3],"-o") == 0 && strcmp(argv[5],"-o") == 0)){
                        fprintf(stderr, "%s\n", "Error: Third and fifth parameter cannot be the same.");
                        exit(EXIT_FAILURE);
                }
        }
        if(argc > 7){
                fprintf(stderr, "%s\n", "Error: Too many parameters. At max, inputs allowed are: -c <columnNameToSortOn> -d <directoryToBeginSearchingForCSVFiles> -o <directoryToOutputSortedCSVFile>.");
                exit(EXIT_FAILURE);
        }

	char cwd[256];
        //establishing the start directory if one is not specified by user
	char *directory = getcwd(cwd, sizeof(cwd));

	/*Grab the parent process id*/
	int ppid = getpid();
	printf("Initial PID: %d\nPIDS of all child processes: ", ppid);
	fflush(stdout);

	/*Set up a counter in memory to keep track of the number of recursive calls*/
	int *count;
	count = (int*)malloc(sizeof(int));
	*count = 0;

        //if just -c <columnNameToSortBy> is the only input, start from cwd and leave output directory blank for now
        if(argc == 3){
                dirHandler(directory, directory, argv[2], ppid, count);
        }
        //if there are 5 arguments, can either be -d or -o as the 3rd argument
        else if(argc == 5){
                if(strcmp(argv[3], "-d") == 0){
                        dirHandler(argv[4], directory, argv[2], ppid, count);
                } else {
                        dirHandler(directory, argv[4], argv[2], ppid, count);
                }
        }
        //if all parameters entered, then the 3rd and 5th argument can either be -o or -d
        else if(argc == 7){
                if(strcmp(argv[3],"-d") == 0){
                        dirHandler(argv[4], argv[6], argv[2], ppid, count);
                } else {
                        dirHandler(argv[6], argv[4], argv[2], ppid, count);
                }
        }

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
	DIR *dir2;
	if(!(dir = opendir(dirname))){
		fprintf(stderr, "Cannot open directory, %s\n", strerror(errno));
		exit(0);
	}
	/*Check if output dir is present*/
	if(!(dir2 = opendir(outputdir))){
		fprintf(stderr, "Output directory does not exist, %s\n", strerror(errno));
		exit(0);
	}else closedir(dir2);

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
					dirHandler(name, outputdir, colname, ppid, count);
					exit(*count + procNum);
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
					fileHandler(name, outputdir, colname);
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
		printf("\nTotal number of processes: %d\n", c+*count);
		fflush(stdout);
	}
}
	
void fileHandler(char *filename, char *outputdir, char *colname){
	/*Check if the file is a .csv file*/
	if(strstr(filename, ".csv")){
		/*Check if the file is an already sorted .csv file*/
		if(strstr(filename, "-sorted-") != NULL){
			return;
		}
		sort(filename, outputdir, colname);
	}else{
		/*Not a .csv file, return*/
		return;
	}
}
//function to trim any leading and trailing whitespaces and will return the trimmed version
char *trim(char *str)
{
	char *endOfStr;
	
	while(isspace((unsigned char)*str)){
		str++;
	}
	
	endOfStr = str + strlen(str) - 1;
	
	while(endOfStr > str && isspace((unsigned char)*endOfStr)){
		endOfStr--;
	}
	endOfStr[1] = '\0';
	
	return str;
}
	

//compares the user input to all the trimmed column names (first row of array), and returns the number of the column(array index)
int getColumnSortedOn(record *recordArr, char *columnName, int numberOfColumns) 
{
	int i = 0;
	for (; i < numberOfColumns; i++){
		if (strcmp(trim(recordArr[0].line[i]), columnName) == 0){
			return i;
		}
	}
	return -1;
}

//This function returns an int that identifies what type the values of the column we are sorting on will be
//2 indicates a double
//1 indicates an int
//0 indicates a string
int getDataType (char *str) {
	int i = 0;
	int isItANumber = 0;
	char *isItADouble = strstr(str, ".");
	
	//parse through the char array to make sure it is all digits, otherwise it will set isItANumber to false
	for (; i < strlen(str); i++) {
		if (isdigit(str[i]) != 0 || isItADouble != NULL) {
			continue;
		} else {
			isItANumber = 1;
			break;
		}
	}
	//if isItANumber is true, we will check if it contains a '.', which indecates a double, otherwise it's an int
	if (isItANumber == 0) {
                if (isItADouble != NULL){
                        return 2;
                } else {
                        return 1;
                }
        }
        //return the type as a string if it is not a double or an int
        return 0;
}
void sort(char *csvFileName, char *outputdir, char *colName)
{
	//open the csv file to be sorted
	FILE *filename = fopen(csvFileName, "r");
	//grab just the file name
	char *tempcsvName = strchr(csvFileName, '/');

	char *csvName = malloc(sizeof(char)*strlen(tempcsvName));
        strcpy(csvName,&tempcsvName[1]);

        char validMovieHeadings[] = "color,director_name,num_critic_for_reviews,duration,director_facebook_likes,actor_3_facebook_likes,actor_2_name,actor_1_facebook_likes,gross,genres,actor_1_name,movie_title,num_voted_users,cast_total_facebook_likes,actor_3_name,facenumber_in_poster,plot_keywords,movie_imdb_link,num_user_for_reviews,language,country,content_rating,budget,title_year,actor_2_facebook_likes,imdb_score,aspect_ratio,movie_facebook_likes";

        //initializing all the variables
        int numberOfColumns = 0;
        int numberOfRecords = 0;
        int numberOfTokenInLine;
        int columnIndex = 0;
        char *line = NULL;
        record *recordArray = malloc(sizeof(record));
        size_t len = 0;
        ssize_t read;

        //while loop that continuously runs as long as getLine is reading something from the CSV file, goes 1 line at a time
        while((read = getline(&line, &len, filename) != -1)) {

                //make enough room in the record array, reallocs to add another index for the new line read
                recordArray = realloc(recordArray, (numberOfRecords + 1) * sizeof(record));

                if (numberOfRecords == 1) {
                       numberOfColumns = numberOfTokenInLine;
                       recordArray[0].columns = numberOfColumns;
                       //go through all the column headings and ensure it's a proper movie csv file
                       int k = 0;
                       for (; k < numberOfColumns; k++){
                               char * temp = strstr(validMovieHeadings, trim(recordArray[0].line[k]));
                               if(temp == NULL){
                                        fprintf(stderr, "Error: The following file does not contain proper movie csv headings: %s\n", csvName);
                                        exit(EXIT_FAILURE);
                                }
                        }
                }
                //variables in order to parse and store each category(token) in a line
                char *token;
                char *end = line;
                numberOfTokenInLine = 0;
                recordArray[numberOfRecords].line = malloc(sizeof(char **));
                //if there are quotes, tokenize whatever is in betweeen the quotes, otherwise just tokenize everything between commas
                while (end != NULL) {
                        if (end[0] != '\"') {
                                token = strsep(&end, ",");
                        } else {
                                end++;
                                token = strsep(&end, "\"");
                                end ++;
                        }
                        //allocate enough space in the array to store the tokens
                        if (numberOfTokenInLine > 0) {
                                recordArray[numberOfRecords].line = realloc(recordArray[numberOfRecords].line, (numberOfTokenInLine + 1) * sizeof(char **));
                        }
                        //set the tokens into their respective array index and increase the number of tokens to go back into the while loop
                        recordArray[numberOfRecords].line[numberOfTokenInLine] = malloc(strlen(token) * sizeof(char) + 1);
                        strcpy(recordArray[numberOfRecords].line[numberOfTokenInLine], token);
                        numberOfTokenInLine++;
                }
                recordArray[numberOfRecords].columns = numberOfTokenInLine;
                numberOfRecords++;
        }

        if(recordArray[0].columns != recordArray[1].columns){
                fprintf(stderr, "Error: More headings than number of columns in file: %s\n", csvName);
                exit(EXIT_FAILURE);
        }


        //set number of rows in the record struct so we know how many lines total there are
        recordArray[0].rows = numberOfRecords;

        //get the column we need to sort on
        columnIndex = getColumnSortedOn(recordArray, colName, numberOfColumns);
        if (columnIndex < 0) {
                fprintf(stderr, "Error: Unable to find the column specified for sorting in file: %s\n", csvName);
                exit(EXIT_FAILURE);
	}else {
                int type = getDataType(recordArray[1].line[columnIndex]);
                mergeSortRecords(recordArray, 1, numberOfRecords-1, columnIndex, type);
        }

        //adds quotation marks back to any token that has a comma in it
        int x, y;
        for (x = 0; x < numberOfRecords; x++) {
                for (y = 0; y < numberOfColumns; y++) {
                        if (strstr(recordArray[x].line[y], ",") != NULL) {
                                size_t len = sizeof(recordArray[x].line[y]);
                                char *temp = malloc(len * sizeof(char) + 1);
                                strcpy(temp, recordArray[x].line[y]);
                                recordArray[x].line[y] = realloc(recordArray[x].line[y], len * sizeof(char) + 3);
                                snprintf(recordArray[x].line[y], len + 3, "\"%s\"", temp);
                                free(temp);
                        }
                }
        }

        //make new file name into the correct format
        //this creates the ".csv" part of the filename
        char *csvPartOfFileName = strrchr(csvFileName, '.');
        //this creates the directory path that includes the csv file minus the ".csv" part
        char *fileName = malloc(sizeof(char) * strlen(csvFileName) - 3);
        strncpy(fileName, csvFileName, strlen(csvFileName) - 4);
        //if an output directory is specified, then make the filename just the csv filename itself and not the directory path with the filename
        if(strcmp(outputdir, "") != 0){
                fileName = strrchr(fileName, '/');
        }
        //create the path to the new output file
        int newFileNameLength = strlen(outputdir) + strlen(fileName) + strlen(colName) + strlen("-sorted-") + strlen(csvPartOfFileName) + 1;
        char *newFileName = malloc(sizeof(char) * newFileNameLength + 1);
        strcat(newFileName, outputdir);
        strcat(newFileName, fileName);
        strcat(newFileName, "-sorted-");
        strcat(newFileName, colName);
        strcat(newFileName, csvPartOfFileName);

        //open the new file and write the sorted list in it
        FILE *newSortedFile = fopen(newFileName, "w+");

        int i;
        int j;
        //print out the sorted rows
        for(i = 0; i < numberOfRecords; i++) {
                for(j = 0; j < numberOfColumns; j++) {
                        if(j != (numberOfColumns - 1)) {
                                fprintf(newSortedFile,"%s,", recordArray[i].line[j]);
                        } else {
                                fprintf(newSortedFile,"%s", recordArray[i].line[j]);
                                if (strstr(recordArray[i].line[j], "\n") == NULL) {
                                        fprintf(newSortedFile, "\n");
                                }
                        }
                }
        }
}	

