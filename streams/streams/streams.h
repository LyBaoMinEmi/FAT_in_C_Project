#pragma once
#define DEFAULT_BLOCK_SIZE	4096
#define MAX_DIR_ENTRIES 64
#define MAX_BLOCK_COUNT 1024
struct bootsector {
	char label[32];
	int blocksize;
	int blockcount;
};
typedef struct bootsector bootsector;
typedef struct direntry { // declaration of a struct and typedef in once
	char filename[32];
	int size;
	int firstblock;
} direntry;
typedef struct dir {
	struct direntry entries[MAX_DIR_ENTRIES];
}dir;

typedef struct fat {
	int f[MAX_BLOCK_COUNT];
}fat;

typedef struct admin {
	bootsector b;
	dir rootDir;
	fat fat;

} admin;

typedef struct freeBlocks {
	int* arr;
	int cnt;
} freeBlocks;


void printStats(char*);
int fileSize(char*);
int getNumberOfBlocks(int filesize, int blocksize);
int getNumberOfFreeBlocks(admin* a);
int getArrayWithFreeBlocks(admin* a, int filesize, freeBlocks* fb);
// returns pointer to first element in array
void printBlockArray(int* arr, int size);

int writeFileToPartition(admin* a, char* fileName, char* partName, freeBlocks* fb);
