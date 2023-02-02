#define _CRT_SECURE_NO_WARNINGS // tell compiler that we aren't NOOBS but PROS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "streams.h"



int main(int argc, char* argv[])
{
	bootsector b = { "4AKIFT",DEFAULT_BLOCK_SIZE,MAX_BLOCK_COUNT };
	admin a = { 0 };


	char* fileName = NULL; //fileName = test.bin

	// create a file ...
	// utilization: streams -create test.bin
	for (int i = 0; i < argc; i++)
	{


		//create a partition
		if (strcmp("-create", argv[i]) == 0)
		{
			fileName = argv[i + 1];

			FILE* fd = fopen(fileName, "wb"); // create a new binary file, erases old one if exists ... 
			int totalSize = sizeof(admin) + b.blocksize * b.blockcount;


			//write all info of admin at these adresses (&b,..) in fd
			fwrite(&b, 1, sizeof(bootsector), fd);
			fwrite(&a.fat, 1, sizeof(fat), fd); // attention stream! continues from current cursor position in stream
			fwrite(&a.rootDir, 1, sizeof(dir), fd);
			//fwrite(pt, 1, count, filepointer): write 'count' elements with size 1 to pt, pt is written in filepointer from the current position
			void* pt = malloc(b.blocksize * b.blockcount);
			if (!pt)
				return;
			memset((void*)pt, 0, b.blocksize * b.blockcount);
			fwrite((void*)pt, 1, b.blocksize * b.blockcount, fd);

			int cursorPos = ftell(fd);
			printf("\ncreating [%s] with [%d] bytes ... cursor=[%d]",
				fileName, totalSize, cursorPos);
			fclose(fd);
		}

		if ((strcmp("-partition", argv[i]) == 0))
		{
			fileName = argv[i + 1];
			FILE* fd = fopen(fileName, "rb");
			fread(&a, 1, sizeof(admin), fd);
			fclose(fd);

		}
		if ((strcmp("-stats", argv[i]) == 0))
		{
			printStats(fileName);
		}
		// streams -partition {partname} -add {userfile}
		if ((strcmp("-add", argv[i]) == 0))
		{
			if (fileName == NULL)
			{
				printf("\n###Error: no partition specified!");
				exit(-1);
			}
			for (int k = 0; (argv[i + 1 + k] != NULL) && (argv[i + 1 + k][0] !='-'); k++) {


				char* userFile = argv[i + 1 + k];

				// search empty entry in root-dir ...
				dir* d = &(a.rootDir);
				// does file exist already ???
				for (int i = 0; i < MAX_DIR_ENTRIES; i++)
				{
					if (strcmp(d->entries[i].filename, userFile) == 0)
					{
						printf("\n###error: [%s] exists already ... exit now", userFile);
						exit(-1);
					}
				}
				// look for free entry
				for (int i = 0; i < MAX_DIR_ENTRIES; i++)
				{
					if (d->entries[i].filename[0] == '\0') // yippie, found one ..
					{

						/*int _fileSize = d->entries[i].size = fileSize(userFile);
						int bCnt = getNumberOfBlocks(d->entries[i].size, b.blocksize);
						int _freeBlocks = getNumberOfFreeBlocks(&a);*/

						int _fileSize = fileSize(userFile);
						int bCnt = getNumberOfBlocks(_fileSize, b.blocksize);
						int _freeBlocks = getNumberOfFreeBlocks(&a);

						if (bCnt <= _freeBlocks) // cool, geht sich aus ...
						{
							strcpy(&(d->entries[i].filename[0]), userFile);
							printf("\nadded [%s] to partition [%s] size=[%d], nummber of blocks of this file =[%d] free=[%d]",
								userFile, fileName, _fileSize, bCnt, _freeBlocks);
							freeBlocks fb;
							getArrayWithFreeBlocks(&a, _fileSize, &fb);
							writeFileToPartition(&a, userFile, fileName, &fb);
							// write used blocks to FAT
							printf("\nblocks used: ");
							for (int j = 0; j < fb.cnt; j++)
							{
								//a.fat.f[j]
								printf("%d, ", fb.arr[j]);
								if (j == 0) // to root dir
								{
									strcpy(a.rootDir.entries[i].filename, userFile);
									a.rootDir.entries[i].size = _fileSize;
									a.rootDir.entries[i].firstblock = fb.arr[j];
								}
								if (j < fb.cnt - 1) // to FAT and root dir
								{
									a.fat.f[fb.arr[j]] = fb.arr[j + 1];
								}
								else // last buddy 
								{
									a.fat.f[fb.arr[j]] = EOF;
								}
							}
							// write entire admin struct back to file
							FILE* fd = fopen(fileName, "rb+");
							fwrite(&a, 1, sizeof(admin), fd);
							fclose(fd);
						}
						/*else if (bCnt > _freeBlocks) {
							printf("\nthis file is greater than memory\n");
							exit(-1);
						}*/
						break; // everthing ok and good bye
					}

				}
			}
		}
		//show all fat
		if ((strcmp("-fat", argv[i]) == 0)) {
			for (int i = 0; i < b.blockcount; i++) {
				printf("[%d]_th block, [%d] address\n", i, a.fat.f[i]);
			}

		}
		if ((strcmp("-dir", argv[i]) == 0)) {
			if (fileName == NULL)
			{
				printf("\n###Error: no partition specified!");
				exit(-1);
			}
			printf("\nfile name, size, first block\n");
			//search unempty entry in root dir
			dir* d = &(a.rootDir);
			for (int i = 0; i < MAX_DIR_ENTRIES; i++) {

				if (d->entries[i].filename[0] != '\0') {
					printf("[%s],[%d],[%d] \n",
						d->entries[i].filename, d->entries[i].size, d->entries[i].firstblock);
				}
			}
		}
		if ((strcmp("-show", argv[i]) == 0)) {
			if (fileName == NULL)
			{
				printf("\n###Error: no partition specified!");
				exit(-1);
			}
			char* userFile = argv[i + 1];

			for (int i = 0; i < MAX_DIR_ENTRIES; i++)
			{
				direntry* e = &a.rootDir.entries[i];
				if (strcmp(e->filename, userFile) == 0) {
					printf("\nFile [%s], size= [%d] \n", e->filename, e->size);
					printf("\nthe position of blocks in FAT = [");
					int blnr = e->firstblock;
					while (a.fat.f[blnr] != EOF)
					{
						printf("%d,", blnr);
						blnr = a.fat.f[blnr];
					}
					printf("%d]", blnr);
					printf("\n_______________________________\n");
				}
			}

		}
		if ((strcmp("-delete", argv[i]) == 0)) {
			if (fileName == NULL)
			{
				printf("\n###Error: no partition specified!");
				exit(-1);
			}
			for (int k = 0; (argv[i + 1 + k] != NULL) && (argv[i + 1 + k][0] != '-'); k++) {
				char* userFile = argv[i + 1 +k];
				int hasFile = 0;//to check if userFile exists or not
				for (int i = 0; i < MAX_DIR_ENTRIES; i++)
				{
					direntry* e = &a.rootDir.entries[i];
					if (strcmp(e->filename, userFile) == 0) {
						hasFile = 1;
						//find block array of this userfile in partition
						//sucessful
						int filesize = fileSize(userFile);//int filesize = e->size;

						int numberOfBlocks = getNumberOfBlocks(filesize, b.blocksize);
						int* arrBlock = malloc(sizeof(int) * numberOfBlocks);
						int blnr = e->firstblock;
						printf("the file %s has arrBlock = [", userFile);
						for (int i = 0; i < numberOfBlocks; i++) {
							if (a.fat.f[blnr] != EOF)
							{
								arrBlock[i] = blnr;
								printf("%d,", arrBlock[i]);
								blnr = a.fat.f[blnr];
							}
						}
						arrBlock[numberOfBlocks - 1] = blnr; //the last block
						printf("%d]", arrBlock[numberOfBlocks - 1]);
						//write 0 in partition at these position

						FILE* ptUserfile = fopen(userFile, "rb");
						FILE* ptPartition = fopen(fileName, "rb+");

						void* buf = malloc(b.blocksize);
						for (int i = 0; i < numberOfBlocks; i++) {
							//find  cursor postion of userFile in Partition
							memset(buf, 0, b.blocksize);
							int offset = sizeof(admin) + b.blocksize * arrBlock[i];
							fseek(ptPartition, offset, SEEK_SET);//SEEK_SET: beginning of the file

							fwrite(buf, 1, b.blocksize, ptPartition);
						}

						free(buf);
						fclose(ptUserfile);
						fclose(ptPartition);

						//update rootdir
						strcpy(e->filename, "");
						e->size = 0;
						e->firstblock = 0;


						//update FAT (write 0)
						//sucessful
						for (int i = 0; i < numberOfBlocks; i++) {
							a.fat.f[arrBlock[i]] = 0;

						}
						//update partition
						FILE* fd = fopen(fileName, "rb+");
						fwrite(&a, 1, sizeof(admin), fd);
						fclose(fd);

						printf("\nFile [%s] is deleted\n", userFile);

						//output
						printf("The file [%s] is deleted", userFile);

					}

				}
				if (hasFile == 0) {
					printf("Error: there is no file [%s] to delete...exit now\n", userFile);
					exit(-1);
				}
			}
		}
		if ((strcmp("-put", argv[i]) == 0)) {
			if (fileName == NULL)
			{
				printf("\n###Error: no partition specified!");
				exit(-1);
			}
			char* userFile = argv[i + 1];
			char* newName = argv[i + 2];
			
			int hasFile = 0;//to check if userFile exists or not


			for (int i = 0; i < MAX_DIR_ENTRIES; i++)
			{
				direntry* e = &a.rootDir.entries[i];
				if (strcmp(e->filename, userFile) == 0) {
					hasFile = 1;
					//find array Blocks of this userFile
					int filesize = fileSize(userFile);//int filesize = e->size;

					int numberOfBlocks = getNumberOfBlocks(filesize, b.blocksize);
					int* arrBlock = malloc(sizeof(int) * numberOfBlocks);
					int blnr = e->firstblock;
					printf("the file %s has arrBlock = [", userFile);
					for (int i = 0; i < numberOfBlocks; i++) {

						if (a.fat.f[blnr] != EOF)
						{
							arrBlock[i] = blnr;
							printf("%d,", arrBlock[i]);
							blnr = a.fat.f[blnr];
						}

					}
					arrBlock[numberOfBlocks - 1] = blnr; //the last block
					printf("%d]", arrBlock[numberOfBlocks - 1]);

					//open fileName Partition to read, open fileCopy to write
					char* fileCopy = userFile;			
					
				
					if (newName == NULL || newName[0] == '-') {
						fileCopy = userFile;
						printf("\nuserFile [%s] is overwritten in host computer\n", userFile);
					}
					else if (newName[0] != '-')
					{ fileCopy = newName;
						printf("\nThe file [%s] has the new name = [%s] ",userFile,fileCopy);
					}					
								
					
					FILE* ptCopy = fopen(fileCopy, "wb");
					FILE* ptPartition = fopen(fileName, "rb");

					unsigned char* buffer = malloc(b.blocksize);
					for (int i = 0; i < numberOfBlocks; i++) {
						//find  cursor postion of userFile in Partition
						int offset = sizeof(admin) + b.blocksize * arrBlock[i];
						fseek(ptPartition, offset, SEEK_SET);//SEEK_SET: beginning of the file
						fread(buffer, 1, b.blocksize, ptPartition);


						fwrite(buffer, 1, b.blocksize, ptCopy);
						
					}

					free(buffer);
					fclose(ptCopy);
					fclose(ptPartition);			
					
				}

			}

			//if no userFile, exit!
			if (hasFile == 0) {
				printf("Error: there is no file [%s] to put...exit now\n", userFile);
				exit(-1);
			}
		}

	}
}
//open file, read file from 0 until SEEK_END, to find the size of file
int fileSize(char* fileName)
{
	FILE* fd = fopen(fileName, "rb");
	fseek(fd, 0, SEEK_END); // seek to end of file
	int size = ftell(fd);
	fclose(fd);
	return size;
}

//print statistics
//open file, save all info in admin
//print all info of the fileName, e.g: test.bin
void printStats(char* fileName)
{

	//?? -partition test.bin -add name.txt
		//after this command, name.txt is already saved in (a) admin, why do we have to save again in this step?
	admin a;
	FILE* fd = fopen(fileName, "rb");
	fread(&a, 1, sizeof(admin), fd);
	fclose(fd);
	//??fileName and label have the same name? redundant?
	printf("\n\n=========================");
	printf("\npartition:\t[%s]", fileName);
	printf("\nlabel:\t\t[%s]", a.b.label);
	printf("\nnumber of blocks:\t[%d]", a.b.blockcount);
	printf("\nblock size:\t[%d]", a.b.blocksize);
	printf("\n------- files -------");
	//print the info of all direntries: name, size, first block, number of blocks
	for (int i = 0; i < MAX_DIR_ENTRIES; i++)
	{
		direntry* e = &a.rootDir.entries[i];
		//?? not sure that filename == 0, maybe the first value is 0 but other values are not 0
		if (e->filename[0] == '\0')
			continue;

		printf("\nDirentry [%d]_th has name '[%s]', size= [%d], first_block = [%d]", i, e->filename, e->size, e->firstblock);
		printf("\n\tthe position of blocks in FAT = [");
		int blnr = e->firstblock;
		while (a.fat.f[blnr] != EOF)
		{
			printf("%d,", blnr);
			blnr = a.fat.f[blnr];
		}
		printf("%d]", blnr); // last block (EOF) belongs to this file too!!


	}
	printf("\n=========================\n\n");
}
//how many blocks does a file need?
int getNumberOfBlocks(int filesize, int blocksize)
{
	int q = filesize / blocksize;
	if (filesize % blocksize != 0)
		return q + 1;
	else
		return q;
}
//how many free blocks of admin?
int getNumberOfFreeBlocks(admin* a)
{
	int free = 0;
	for (int i = 0; i < a->b.blockcount; i++)
	{
		if (a->fat.f[i] == '\0')
			free++;
	}
	return free;
}
//return an array of the free blocks
int getArrayWithFreeBlocks(admin* a, int _fileSize, freeBlocks* fb)
{
	int _blockCount = getNumberOfBlocks(_fileSize, a->b.blocksize);
	fb->cnt = _blockCount; // return this info to the caller ....
	int* blockArray = malloc(sizeof(int) * _blockCount);
	fb->arr = blockArray;
	// let's seek some free blocks and collect the block numbers in an array
	for (int i = 0, j = 0; i < a->b.blockcount; i++)
	{
		if (a->fat.f[i] == '\0')
		{
			blockArray[j++] = i;
		}
		if (j >= _blockCount) // that's enough ...
			break;
	}
	printBlockArray(blockArray, _blockCount);
	return 1;
}

void printBlockArray(int* arr, int size)
{
	for (int i = 0; i < size; i++)
		printf("\n%d => block=[%d]", i, arr[i]);
}

/*
this function reads the file to be integrated in our partition block by block
and writes those blocks to the previously calculated blocks (FreeBlocks structure)
*/

//write userFile to test.bin at these blocks in fb.arr[]
int writeFileToPartition(admin* a, char* fileName, char* partName, freeBlocks* fb)
{
	unsigned char* buffer = malloc(a->b.blocksize);

	FILE* fdUserFile = fopen(fileName, "rb");
	FILE* fdPart = fopen(partName, "rb+");
	int wCnt;
	for (int i = 0; i < fb->cnt; i++)
	{
		int rCnt = fread(buffer, 1, a->b.blocksize, fdUserFile); // read a single block
		// calculate where to write this block on the partition
		int offset = sizeof(admin) + a->b.blocksize * fb->arr[i];
		// set cursor position in partition file stream accordingly ...
		//SEEK_SET: beginning of the file
		fseek(fdPart, offset, SEEK_SET); // offset bytes vom Ursprung entfernt
		wCnt = fwrite(buffer, 1, a->b.blocksize, fdPart); // write content to partition file
		// don't forget the fat entry !!!
	}
	free(buffer);
	fclose(fdUserFile);
	fclose(fdPart);
	return 1;

	//? don't return anything?
	//?WCnt,rCnt didn't used?

	/*
	2 questions?
	1.fwrite to current position?
	size_t fwrite ( const void * ptr, size_t size, size_t count, FILE * stream );
	Write block of data to stream
	Writes an array of count elements, each one with a size of size bytes, from the block of memory pointed by ptr to the current position in the stream.
	2. why did this loop run until fb->cnt? i think, only until nummber of block of filename?
	*/

}

//
//int findArrBlockOfUserfile(admin* a, char* userFile) {
//	int filesize = fileSize(userFile);//int filesize = e->size;
//
//	int numberOfBlocks = getNumberOfBlocks(filesize, b.blocksize);
//	int* arrBlock = malloc(sizeof(int) * numberOfBlocks);
//	int blnr = e->firstblock;
//	printf("the file %s has arrBlock = [", userFile);
//	for (int i = 0; i < numberOfBlocks; i++) {
//
//		if (a.fat.f[blnr] != EOF)
//		{
//			arrBlock[i] = blnr;
//			printf("%d,", arrBlock[i]);
//			blnr = a.fat.f[blnr];
//		}
//
//	}
//	arrBlock[numberOfBlocks - 1] = blnr; //the last block
//	printf("%d]", arrBlock[numberOfBlocks - 1]);
//	return 1;
//}