#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

void traverse(char *newdir){
	char cwd[256];
//	printf("Dir is %s\n", getcwd(cwd, sizeof(cwd))); 
	DIR *dir;
	if(!(dir = opendir("."))){
		perror("Error: ");
		exit(EXIT_FAILURE);
	}

	struct dirent *d;
	while( (d = readdir(dir)) != NULL){
		if(strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0){
		}else{
			if(d->d_type == DT_DIR){
				printf("Directory: %s\n", getcwd(cwd, sizeof(cwd)));
				chdir(d->d_name);
				int pid = fork();
				if(pid > 0){
					//parent proc
					printf("Parent PID: %d\n", pid);
					wait(NULL);
				}else{
					printf("Child's directory: %s\n", getcwd(cwd, sizeof(cwd)));
					printf("Child PID: %d\n", pid);
					printf("Child's PPID: %d\n", getppid());
//					traverse(d->d_name);
				}
				chdir("..");
			}else{
				/*Check if file is a .csv file*/
				int length = strlen(d->d_name);
				if(d->d_name[length-4] == '.' && d->d_name[length-3] == 'c' && d->d_name[length-2] == 's' && d->d_name[length-1] == 'v'){
					printf("Directory: %s File: %s\n", getcwd(cwd,sizeof(cwd)), d->d_name);
				}
			}
		}
	}
	closedir(dir);
}

int main(int argc, char **argv){

	printf("main pid: %d\n", getpid());
	traverse(".");
	return 0;
}
