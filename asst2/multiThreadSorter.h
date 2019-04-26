#ifndef multiThreadSorter_h
#define multiThreadSorter_h
	typedef struct args{
		char *name;
		char *colname;
		char *outputdir;
		int dirLevel;
	}args;
	typedef struct Node{
		pthread_t TID;
		char *line;
		char binArr[28];
		struct Node *next;
	}Node;

	void *dirHandler(void *Args);
	void *fileHandler(void *Args);
	void getToken(Node **arr, char *line, int index, char binArr[28]);
	void writeToFile(Node **arr, char *colname, char *outputdir);
	Node *insert(Node *front, pthread_t tid, char *line, char binArr[28]);
	void freeNode(Node *front);

	void mergeSort(Node **arr, int left, int right, int colNum, int numCols);
	void merge(Node **arr, int left, int mid, int right, int colNum, int numCols);
	char *trim(char *str);
	Node *copy(Node *dest, Node *src);
#endif
