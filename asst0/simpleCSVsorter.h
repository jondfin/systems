#ifndef simpleCSVsorter_h
#define simpleCSVsorter_h
	typedef struct Node{
	char data[256];
	struct Node *next;
	}Node;
	typedef struct BN{
		char data[1024];
		struct BN *next;
	}BN;	
	void printList(Node **arr, int lineCount);
	void freeBNode(BN *head);
	void freeNode(Node *head);
	BN *BNinsert(BN *front, char line[1024]);
	Node *insert(Node *front, char token[256]);
	void getToken(Node **arr, char *line, int index);
#endif
#ifndef mergesort_h
#define mergesort_h
	void mergeSort(Node **arr, int left, int right, int colNum, int numCols);
#endif
