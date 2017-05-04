//
//  fileSystem.cpp
//  
//
//  Created by oliver on 4/24/17.
//
//

#include "fileSystem.hpp"
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream

using namespace std;

#define OFFSET 259
#define FREE_INODE_LIST_OFFSET 258
#define FREE_BLOCK_LIST_OFFSET 257

struct Superblock{
	int numBlocks;
	int blockSize;
	int offset; // in bytes
	int hasFiles;
	
};



int getFileSize(string fName){
	std::streampos fsize = 0;
	std::ifstream file( fName, std::ios::binary );
	
	fsize = file.tellg();
	file.seekg( 0, std::ios::end );
	fsize = file.tellg() - fsize;
	file.close();
	cout<<"file size is "<<fsize<<" bytes"<<endl;
	return fsize;
}




fileSystem::fileSystem(string fileName){

	diskName = fileName;
	
	// open up the disk file for reading
	ifstream ifs;	
	ifs.open(fileName, ios::in | ios::binary);

	Superblock blank;
	ifs.read(reinterpret_cast<char*>(&blank),sizeof(Superblock));

	// set numBlocks and blockSize by reading this info from the 
	// Superblock that is currently written in the disk file
	// at this point the Superblock is the only thing of importance that is written in the file
	numBlocks = blank.numBlocks;
	blockSize = blank.blockSize;
	
		
	// the number of block pointers you can fit in an indirect block
	// is the size of a block divided by the size of an int
	// because a block pointer is just an integer which is 4 bytes
	indBlockSize = blockSize / 4;
	
	// the number of indirect block pointers you can fit in a double indirect block
	// again, for us, block pointers are ints which are 4 bytes big
	doubleIndSize = blockSize / 4;

	// free block is list is as big as the number of total blocks in the disk file
	// true means this block is in use
	// false means this block is not being used
	freeBlockList = new bool[numBlocks];
	
	// max file size is
	// the TOTAL number of block pointers that we have times the blockSize
	max_file_size = (12 + indBlockSize + (indBlockSize * indBlockSize)) * blockSize;

	if(blank.hasFiles){

		//seek to FREE_INODE_LIST_OFFSET
		//read into freeINodeList

		//seek to FREE_BLOCK_LIST_OFFSET
		//read into freeBlockList
		
		for (int i = 0; i < 256; i++) {
		
			if(freeiNodeList[i] == 0) continue;
			else{
			// so the idea here is this:
			// see which iNodes are active by reading from the freeInodeList
			// if freeInodeList[i] == 1 then read that i'th block Inode in the DISK FILE's
			// information into the corresponding INodeArrayData
			}
			
		}
	}

	ifs.close();
	
}

void fileSystem::create(string ssfsFName){
	// store file name in tab2
    char tab2[1024];
    strncpy(tab2, ssfsFName.c_str(), sizeof(tab2));
    tab2[sizeof(tab2) - 1] = 0;
    bool unique = true;
    for (int i = 0; i < 256; i++) {
        if (iNodeList[i].getFileName() == ssfsFName)
            unique = false;
    }
    
    iNode input(tab2);
    if (!unique) {
        cout << "This file already exists.";
        return;
    }
    
    bool freeFlag = false;
    for (int i = 0; i < 256; i++) {
        if (freeiNodeList[i] == false) {
            freeiNodeList[i] = true;
            iNodeList[i] = input;
            freeFlag = true;
            break;
        }
    }
    
    if (!freeFlag)
        cout << "There is not enough free memory to create a new file.";
	
}
void fileSystem::import(string ssfsFName, string unixFName){

	
	//find index in iNodeList where ssfs file is
	int iNodeIndex;
	for(iNodeIndex = 0; iNodeIndex<256; iNodeIndex++)
		if(iNodeList[iNodeIndex].getFileName() == ssfsFName)
			break;
	
	//set file size
	iNodeList[iNodeIndex].fSize = getFileSize(unixFName);
	if (iNodeList[iNodeIndex].fSize > max_file_size) {
		cerr << "user file size exceeds max file size, exiting " << endl;
		return;
	}

	
	
	
	//open the disk file
	FILE* diskFile = fopen(diskName.c_str(), "w");

	
	
	
	//open the unix file
	FILE* unixFile = fopen(unixFName.c_str(), "r");
	
	
	
	
	
									
	char toBeWritten[blockSize];
	
	//used to keep track of file size
	int blocksRead = 0;
	int bytesRead = 0;
	
	
	
	int bytesToWrite = blockSize;
	
	
	//MARK: Need to use fileSize instead of .eof
	while(bytesRead != iNodeList[iNodeIndex].fSize){
		
		
	
												 								
		if(iNodeList[iNodeIndex].fSize - bytesRead < blockSize) 
			bytesToWrite = iNodeList[iNodeIndex].fSize - bytesRead;
			
		
		fread(toBeWritten,sizeof(char), bytesToWrite, unixFile);
		
		bytesRead += bytesToWrite;

		for(int blockNum = 0; blockNum<numBlocks; blockNum++){
			//find free block in freeBlockList
			if(freeBlockList[blockNum] == 0){
				freeBlockList[blockNum] = 1;
				
				
				
				
				
				if(blocksRead < 12){ //direct blocks
					
					iNodeList[iNodeIndex].blockAddressTable[blocksRead] = blockNum;
		
					//Write toBeWritten to ( (offset + blockNum)* blockSize)
					fseek(diskFile,(OFFSET+blockNum)*blockSize, 0);
					fwrite(toBeWritten,sizeof(char),bytesToWrite,diskFile);
				}
				
				
				
				
				
				
				else if(blocksRead < indBlockSize){ //single indirect block pointer

					if(blocksRead == 12){ // on 13th block need to make indirect block
						
						for (int i = 0; i<numBlocks; i++) {
							if(freeBlockList[i] ==0){
								freeBlockList[i] = 1;
								iNodeList[iNodeIndex].ib.pointer = i;
								
								break;
							}
						}
					}

					
					
				
					iNodeList[iNodeIndex].ib.blockTable.push_back(blockNum);

					
					fseek(diskFile,(OFFSET+blockNum)*blockSize, 0);
					fwrite(toBeWritten,sizeof(char),bytesToWrite,diskFile);

					
					
				
				}
				else{	//double indirect block pointer


					if(blocksRead == indBlockSize){// need to make double indirect block
						
						for (int i = 0; i<numBlocks; i++){
							if(freeBlockList[i] ==0){
								freeBlockList[i] = 1;
								iNodeList[iNodeIndex].doubleIndBlock = i;
								break;
							}
						}
					}

					
					int currentBlock = blocksRead - indBlockSize;
					
					if(currentBlock % indBlockSize == 0){ //when an indirect block is filled we create another
						
						for (int i = 0; i<numBlocks; i++) {
							if(freeBlockList[i] ==0){

								freeBlockList[i] = 1;
							
								//MARK: might have  to be dynamically allocated?
								indBlock newIB;
								newIB.pointer = i;

								iNodeList[iNodeIndex].doubleIndBlockTable.push_back(newIB);

								break;
							}
							
						}
					}
					
					(iNodeList[iNodeIndex].doubleIndBlockTable.front()).blockTable.push_back(blockNum);
	
					
					fseek(diskFile,(OFFSET+blockNum)*blockSize, 0);
					fwrite(toBeWritten,sizeof(char),bytesToWrite,diskFile);

					
					
				}
				blocksRead++;

				
				break;
			}
		}
		
	}
	fclose(unixFile);
	fclose(diskFile);
}



void fileSystem::cat(string ssfsFName){
	
	
	cout<<"CAT "<<ssfsFName<<endl;
	
	
	//find iNode in iNodeList
	int iNodeIndex;
	for(iNodeIndex = 0; iNodeIndex<256; iNodeIndex++)
		if(iNodeList[iNodeIndex].getFileName() == ssfsFName)
			break;
	
	
	if(iNodeIndex < 256) {
		ifstream diskFile;
		diskFile.open(diskName, ios::binary | ios::in | ios::out);
		
		int bytesRead = 0;
		int blocksRead = 0;
		
		int currentIndBlock = 0;
		
		
		char buf[blockSize+1];
		
		int bytesToRead = blockSize;
		
		while(bytesRead < iNodeList[iNodeIndex].fSize) {
			
			if((iNodeList[iNodeIndex].fSize - bytesRead) < blockSize){
				bytesToRead = iNodeList[iNodeIndex].fSize - bytesRead;
			}
			
			bytesRead += bytesToRead;
			
			if(blocksRead < 12) { // seek to next entry in direct block table
				if(iNodeList[iNodeIndex].blockAddressTable[blocksRead] == -1){
					cout<<"this shouldn't happen"<<endl;
					break;
				}
				
				diskFile.seekg((OFFSET + iNodeList[iNodeIndex].blockAddressTable[blocksRead])* blockSize);
				diskFile.read(buf, bytesToRead);

			}
			else if(blocksRead < indBlockSize){
				int cur = blocksRead - 12;
				diskFile.seekg((OFFSET + iNodeList[iNodeIndex].ib.blockTable[cur])* blockSize);
				diskFile.read(buf, bytesToRead);
			
			}
			else{
				int cur = blocksRead - indBlockSize;
				
				if(cur != 0 && cur % indBlockSize == 0)
					currentIndBlock += 1;
					
				diskFile.seekg((OFFSET + iNodeList[iNodeIndex].doubleIndBlockTable[currentIndBlock].blockTable[cur%indBlockSize]) * blockSize);
				diskFile.read(buf, bytesToRead);
	
			}
			buf[bytesToRead] = '\0';
			
			cout<<buf;

			blocksRead++;

		}
		
		cout << endl; // spacing after all of the data has been put out with no endls between
		
		diskFile.close();
	}
	
	else { // file was not found in the list
		cerr << ssfsFName << " was not found in the disk" << endl;
	}
 
}

void fileSystem::del(string ssfsFName){
	
	
	//find iNode in iNodeList
	int iNodeIndex;
	for(iNodeIndex = 0; iNodeIndex<256; iNodeIndex++)
		if(iNodeList[iNodeIndex].getFileName() == ssfsFName)
			break;


	
	int curBlock;
	
	//free all direct associated with file
	for(int i = 0; i < 12; i++){
		curBlock = iNodeList[iNodeIndex].blockAddressTable[i];
		if(curBlock != -1)
			freeBlockList[curBlock] = 0;
	}
	
	//free all indirect blocks associated with file
	while(iNodeList[iNodeIndex].ib.blockTable.size() != 0){
		curBlock = iNodeList[iNodeIndex].ib.blockTable.back();
		freeBlockList[curBlock] = 0;
		
		iNodeList[iNodeIndex].ib.blockTable.pop_back();
	}
	if(iNodeList[iNodeIndex].ib.pointer != -1){
		freeBlockList[iNodeList[iNodeIndex].ib.pointer] = 0;
	}
	
	//free double indirect block
	if(iNodeList[iNodeIndex].doubleIndBlock != -1){
		freeBlockList[iNodeList[iNodeIndex].doubleIndBlock] = 0;
		
		while(iNodeList[iNodeIndex].doubleIndBlockTable.size() != 0){
			
			vector<int> currentIndBlock = iNodeList[iNodeIndex].doubleIndBlockTable.back().blockTable;
			freeBlockList[iNodeList[iNodeIndex].doubleIndBlockTable.back().pointer] = 0;
	
			while(currentIndBlock.size() != 0){
				curBlock = currentIndBlock.back();
				freeBlockList[curBlock] = 0;
				currentIndBlock.pop_back();
			}
			iNodeList[iNodeIndex].doubleIndBlockTable.pop_back();

		}
	}
	
	
	//add iNodeIndex to free list
	freeiNodeList[iNodeIndex] = 0;
	
	
	//set to default values
	iNodeList[iNodeIndex] = iNode();
	
}

void fileSystem::write(string ssfsFName, char ch, int startByte, int numBytes){
	
	//find iNode in iNodeList
	int iNodeIndex;
	for(iNodeIndex = 0; iNodeIndex<256; iNodeIndex++)
		if(iNodeList[iNodeIndex].getFileName() == ssfsFName)
			break;

	if(iNodeIndex < 256) {
		ofstream outFile;
		outFile.open(diskName, ios::binary | ios::out);

		int bytesWritten = 0;
		int blocksWrittenTo = 0;

		int currentIndBlock = 0;
		// char to write = ch

		int bytesToWrite = numBytes;

		while(bytesWritten < bytesToWrite) {
			if(startByte >= blockSize) {
				startByte -= blockSize;
				blocksWrittenTo++;

				continue;
			}

			if((iNodeList[iNodeIndex].fSize - bytesWritten) < blockSize) {
				bytesToWrite = iNodeList[iNodeIndex].fSize - bytesWritten;
			}

			bytesWritten += bytesToWrite;

			if(blocksWrittenTo < 12) {
				if(iNodeList[iNodeIndex].blockAddressTable[blocksWrittenTo] == -1) {
					cout << "this shouldn't happen" << endl;

					break;
				}

				outFile.seekp((startByte + OFFSET + iNodeList[iNodeIndex].blockAddressTable[blocksWrittenTo]) * blockSize);
			} else if(blocksWrittenTo < indBlockSize) {
				int cur = blocksWrittenTo - 12;

				outFile.seekp((startByte + OFFSET + iNodeList[iNodeIndex].ib.blockTable[cur]) * blockSize);
			} else {
				int cur = blocksWrittenTo - indBlockSize;

				if(cur != 0 && cur % indBlockSize == 0)  {
					currentIndBlock += 1;
				}

				outFile.seekp((startByte + OFFSET + iNodeList[iNodeIndex].doubleIndBlockTable[currentIndBlock].blockTable[cur%indBlockSize]) * blockSize);
			}

			for(int i = 0; i < bytesToWrite; i++) {
				outFile.write(&ch, sizeof(char));
				outFile.seekp(outFile.cur + 1);
			}

			startByte = 0;
			blocksWrittenTo++;
		}
		
		outFile.close();
	} else { // file was not found in the list
		cerr << ssfsFName << " was not found in the disk" << endl;
	}
}

void fileSystem::read(string ssfsFName, int startByte, int numBytes){
	//find iNode in iNodeList
	int iNodeIndex;
	for(iNodeIndex = 0; iNodeIndex<256; iNodeIndex++)
		if(iNodeList[iNodeIndex].getFileName() == ssfsFName)
			break;

	if(iNodeIndex < 256) {
		ifstream diskFile;
		diskFile.open(diskName, ios::binary | ios::in | ios::out);

		int bytesRemaining = numBytes; // keeps track of how many bytes we still need to read out
		int directBlocksRead = 0; // keep track of the 12 blocks we must iterate through first
		int indirectBlocksRead = 0; // keep track of how many blocks we've read from the current indirect block being pointed to (changes once we enter double indirect blocks)
		int doubleIndirectBlocksRead = 0; // keep track of what # indirect block we should be looking at from the doubleIndBlock vector in currentNode
		int bytesToRead; // will determine how many bytes we read below after seeking to the correct position in the file
		iNode* currentNode = &iNodeList[iNodeIndex]; // store this indexed value just because it gets referenced so often below
		indBlock* currentIndBlock = &currentNode->ib; // start off with the single indirect block assigned to this inode
		// after reading this first indBlock, this value will iterate through the list of indBlocks in the doubleIndBlockPtr vector of indBlocks

		while(bytesRemaining > 0 && doubleIndirectBlocksRead < currentNode->doubleIndBlockTable.size()) {
			if(startByte >= blockSize) { // skip over the whole read process if the startByte is further than the upcoming block's end point
				startByte -= blockSize;

				if(directBlocksRead < 12) directBlocksRead++;
				else if(indirectBlocksRead < currentIndBlock->blockTable.size()) indirectBlocksRead++;
				else doubleIndirectBlocksRead++;

				continue;
			} else {
				if(directBlocksRead < 12) { // seek to next entry in direct block table
					diskFile.seekg(OFFSET*blockSize + startByte + (currentNode->blockAddressTable[directBlocksRead] * blockSize));

					directBlocksRead++;
				} else if(indirectBlocksRead < currentIndBlock->blockTable.size()){ // seek to the next entry in indirect block table
					diskFile.seekg(OFFSET*blockSize + startByte + (currentIndBlock->blockTable[indirectBlocksRead] * blockSize));

					indirectBlocksRead++;
				} else {
					// following three lines allow us to iterate through this vector of indBlocks in the same manner we did above to the initial indBlock
					currentIndBlock = &currentNode->doubleIndBlockTable[doubleIndirectBlocksRead];
					indirectBlocksRead = 0;
					doubleIndirectBlocksRead++;

					continue;
				}

				if(startByte) startByte = 0; // we've already used this by this point

				// go for the whole block if bytesRemaining is >= blockSize
				bytesToRead = min(blockSize, bytesRemaining);
				bytesRemaining -= bytesToRead;

				// read min(remaining bytes, blockSize) data
				char* data = new char[bytesToRead];
				diskFile.read(data, bytesToRead);

				// print the data we read in from file
				cout << data;
			}
		}

		cout << endl; // spacing after all of the data has been put out with no endls between

		diskFile.close();
	} else { // file was not found in the list
		cerr << ssfsFName << " was not found in the disk" << endl;
	}
}


string fileSystem::list(){
	string fileList = "";
	
	for (int i = 0; i<256; i++) {
		if(iNodeList[i].getFileName() != (string) "~")
			fileList +=( iNodeList[i].getFileName() + (string)"\n");
		   
	}
	return fileList;
}


void fileSystem::shutdown(){

    cout << " made it to shut down" << endl;
/*	ofstream diskFile;
	diskFile.open(diskName, ios::out | ios::binary | ios::ate);
*/
	//open the disk file
	// try doing shutdown using FILE object instead of ofstream object
	FILE* diskFile = fopen(diskName.c_str(), "w");
	
	//open the unix file
	FILE* diskFileRead = fopen(diskName.c_str(), "r");	

	Superblock s;

	s.offset = offset;
	s.blockSize = blockSize;
	s.numBlocks = numBlocks;
	s.hasFiles = 1;
	fseek(diskFile, 0, 0);
	fwrite(&s, sizeof(Superblock), 1, diskFile);


/*	ifstream diskFileREAD;
	diskFileREAD.open(diskName, ios::in | ios::binary);
*/
	//MARK: Maybe write superblock out to file, just to be safe
		
	int iNodeListSeekOffset;

	for(int i=0; i<256; i++){
		if(freeiNodeList[i]){
			// seek to the beginning of the next available iNodeList BLOCK in the file
			iNodeListSeekOffset = (1+i) * blockSize; 

/*			diskFile.seekp(iNodeListSeekOffset);
			diskFile.write(iNodeList[i].fileName, sizeof(iNodeList[i].fileName));
*/

			// seek in the FILE way
			// don't need to seek after writing because
			// fwrite automatically moves the file pointer by num of bytes written
			fseek(diskFile,iNodeListSeekOffset, 0);
			fwrite(&iNodeList[i].fileName, sizeof(char), 32, diskFile);

			// increment iNodeListSeekOffset by the 32 bytes it took to write the filename
/*			iNodeListSeekOffset += sizeof(iNodeList[i].fileName);
			diskFile.seekp(iNodeListSeekOffset);
*/
			// now write the file's size
/*			diskFile.write(reinterpret_cast<const char*>(&iNodeList[i].fSize), sizeof(iNodeList[i].fSize));
*/			
			// write the 4 byte integer fSize to the disk immediately 
			// following the filename			
			fwrite(&iNodeList[i].fSize, sizeof(int), 1, diskFile);
	
/*			iNodeListSeekOffset += sizeof(int);
			diskFile.seekp(iNodeListSeekOffset);
*/
/*			for (int k = 0; k < 12; k++) {
				diskFile.write(reinterpret_cast<char*>(&iNodeList[i].blockAddressTable[k]), sizeof(int));
*/
			// don't need to use the loop with fwrite because we can write the whole
			// block address array by writing 12 ints at once
			fwrite(&iNodeList[i].blockAddressTable, sizeof(int), 12, diskFile);
				// increment seek offset by the size of one int to get ready to write the next one 
				// in the array
/*				iNodeListSeekOffset += sizeof(int);
				diskFile.seekp(iNodeListSeekOffset);
			}
*/

			// now we can write the indirect block pointer number
			fwrite(&iNodeList[i].ib.pointer, sizeof(int), 1, diskFile);

/*			diskFile.write(reinterpret_cast<char*>(&iNodeList[i].ib.pointer), sizeof(int));
			iNodeListSeekOffset += sizeof(int);
*/
			// now we can write the double indirect block pointer
			fwrite(&iNodeList[i].doubleIndBlock, sizeof(int), 1, diskFile);

/*			diskFile.write(reinterpret_cast<char*>(&iNodeList[i].doubleIndBlock), sizeof(int));
			iNodeListSeekOffset += sizeof(int);
*/
		}
	}

	// now we want to write teh free block list to the disk file
	// so seek to the write block 
	int freeBlockListOffset = (FREE_BLOCK_LIST_OFFSET)*blockSize;
	fseek(diskFile,freeBlockListOffset, 0);

/*	diskFile.seekp(freeBlockListOffset);
*/
	//Write freeBlockList out to disk
	char one[] = {'1'}; 
	int zero[] = {'0'};
	for(int i=0; i<numBlocks; i++){
		if(freeBlockList[i]) {
			fwrite(one, sizeof(char), 1, diskFile);

/*			diskFile.write("1",sizeof(char));
*/
	//		freeBlockListOffset += sizeof(char);
	//		diskFile.seekp(freeBlockListOffset);
		} else {
			fwrite(zero, sizeof(char), 1, diskFile);

/*			diskFile.write("0",sizeof(char));
*/	//		freeBlockListOffset += sizeof(char);
	//	diskFile.seekp(freeBlockListOffset);
		}
	}
	
	// seek to spot to begin writing freeInodeListOffset
	int freeInodeListOffset = (FREE_INODE_LIST_OFFSET)*blockSize;
	fseek(diskFile, freeInodeListOffset,0);

/*	diskFile.seekp(freeInodeListOffset);
*/
	//Write freeInodeList out to disk
	for(int i=0; i<256; i++){
		if(freeiNodeList[i]) {
			fwrite(one, sizeof(char), 1, diskFile);

/*			diskFile.write("1",sizeof(char));
*/	//		freeInodeListOffset += sizeof(char);
	//		diskFile.seekp(freeInodeListOffset);
		}
		else {
			fwrite(zero, sizeof(char), 1, diskFile);

/*			diskFile.write("0",sizeof(char));
*/	//		freeInodeListOffset += sizeof(char);
	//		diskFile.seekp(freeInodeListOffset);
		}
		
	}

	// now that we've written all the meta data we need to signify to future runs of our
	// program that data already resides on the disk, so write a 1 to the 

	// superblock's 'hasFiles' variable




/*	
	int read_has_files = -1;
	fseek(diskFileRead, 4*sizeof(int), 0);
	fread(&read_has_files, sizeof(int), 1, diskFileRead); 
	cout << "BEFORE has files is currently set to " << read_has_files << endl;

	int has_files_true = 1;
	fseek(diskFile, 4*sizeof(int), 0);
	fwrite(&has_files_true, sizeof(int), 1, diskFile);
	*/

	Superblock test;

	int new_read_has_files = -1;
	fseek(diskFileRead, 0, 0);
	fread(&test, sizeof(Superblock), 1, diskFileRead); 
	cout << "AFTER has files is currently set to " << test.hasFiles << endl;


	int read_fSize = 0;
	char read_filename[32];
	int db_array[12];	

	fseek(diskFileRead, (blockSize), 0);
	fread(read_filename, sizeof(char), 32, diskFileRead);
	fread(&read_fSize, sizeof(int), 1, diskFileRead);
	fread(db_array, sizeof(int), 12, diskFileRead);
	cout << "now recovering iNode info from the file by reading it, here are the results: " << endl;
	cout << "the name of the first file created is: " << read_filename << endl;
	cout << "the size of this file is: " << read_fSize << endl;
	for (int i = 0; i < 12; i++) {
		cout << "db pointer number " << i+1 << " is " << db_array[i] << endl;
	}

//	cout << "THE ACTUAL SIZE IS " << iNodeList[0].fSize << endl;
//	cout << "the size of the file is " << read_fSize << endl;
	
	fclose(diskFile);
	fclose(diskFileRead);
	
//	diskFile.close();
	
}



/*
 _______________________________
 |								|
 |			Superblock			|
 |  							|
 |	int:numBlocks				|	<-----  size: 1 block
 |	int:blockSize				|
 |	int:offset					|
 |	int:hasFiles				|
 |______________________________|
 |								|
 |			iNodeList			|   <----- size: 256 blocks
 |								|
 |______________________________|
 |								|
 |			freeBlockList		|   <----- size: 1 block
 |______________________________|
 |								|
 |			freeiNodeList		|   <----- size: 1 block
 |								|
 |------------------------------|	<-------
 |								|			|
 |								|			|_______ offset points here
 |								|
 |								|
 |								|
 |			mainMemory			|	<----- size: numBlocks*blockSize bytes
 |								|
 |								|
 |								|
 |								|
 |								|
 |								|
 |______________________________|

 
 _______________________________________
|			iNode						|
|										|
|	string: fName						|
|										|
|	int: fSize							|
|										|
|	int[12]: blockAddressTable			|
|										|
|	int indBlockPointer					|
|										|
|	int doubleIndBlockPointer			|
|										|
|										|
|_______________________________________|
 
 */
