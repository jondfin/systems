#ifndef scannerCSVsorter_h
#define scannerCSVsorter_h
	typedef struct record{
		char **line;
		int columns;
		int rows;
	}record;

	char *trim(char *);
	int getColumnSortedOn(record *, char *, int);
	int getDataType(char *);
	void sort (char * filename, char * outputdir, char * colname);
	void fileHandler(char *filename, char *outputdir, char *colname);
	void dirHandler(char *dirname, char *outputdir, char *colname, int ppid, int *count);
	void mergeRecords(record *, int, int, int, int, int, int);
	void mergeSortRecords(record *, int, int, int, int);
#endif
