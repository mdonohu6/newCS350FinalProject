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

struct iNode2{
	char fName[32];
	int blockAddressTable[12];
	int indBlockPointer;
	int doubleIndBlockPointer;
	int fSize;

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
	cout<<"numBlocks: "<<numBlocks<<" blockSize: "<<blockSize<<endl;

	
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
		outFile.open(diskName, ios::binary | ios::in | ios::out);

		int bytesWritten = 0;
		int blocksWrittenTo = startByte / blockSize;
		startByte = startByte % blockSize;

		int currentIndBlock = 0;
		// char to write = ch

		int bytesToWrite;

		while(bytesWritten < numBytes) {
			bytesToWrite = min(blockSize - startByte, numBytes);

			bytesWritten += bytesToWrite;

			if(blocksWrittenTo < 12) {
				if(iNodeList[iNodeIndex].blockAddressTable[blocksWrittenTo] == -1) {
					for(int i = 0; i < numBlocks; i++) {
						if(freeBlockList[i] == 0) {
							freeBlockList[i] = 1;
							iNodeList[iNodeIndex].blockAddressTable[blocksWrittenTo] = i;

							break;
						}

						if(i == numBlocks-1) {
							cout << "There's no memory left! The file system wrote as much as it could before running out of memory." << endl;
						}
					}
				}

				outFile.seekp(startByte + (OFFSET + iNodeList[iNodeIndex].blockAddressTable[blocksWrittenTo]) * blockSize);
			} else if(blocksWrittenTo < indBlockSize) {
				int cur = blocksWrittenTo - 12;

				outFile.seekp(startByte + (OFFSET + iNodeList[iNodeIndex].ib.blockTable[cur]) * blockSize);
			} else {
				int cur = blocksWrittenTo - indBlockSize;

				if(cur != 0 && cur % indBlockSize == 0)  {
					currentIndBlock += 1;
				}

				outFile.seekp(startByte + (OFFSET + iNodeList[iNodeIndex].doubleIndBlockTable[currentIndBlock].blockTable[cur%indBlockSize]) * blockSize);
			}

			for(int i = 0; i < bytesToWrite; i++) outFile << ch;
			
			startByte = 0;
			blocksWrittenTo++;
		}
		
		outFile.close();
	} else { // file was not found in the list
		cerr << ssfsFName << " was not found in the disk" << endl;
	}
}

void fileSystem::read(string ssfsFName, int startByte, int numBytes){

	
	
	
	int startBlock = startByte / blockSize;
	int ofst = startByte % blockSize;
	
	cout<<"READ "<<ssfsFName<<endl;




	//find iNode in iNodeList
	int iNodeIndex;
	for(iNodeIndex = 0; iNodeIndex<256; iNodeIndex++)
		if(iNodeList[iNodeIndex].getFileName() == ssfsFName)
			break;
	
	
	if(iNodeIndex < 256) {
		ifstream diskFile;
		diskFile.open(diskName, ios::binary | ios::in | ios::out);
		
		int bytesRead = 0;
		int blocksRead = startBlock;
		
		int currentIndBlock = 0;
		
		
		char buf[blockSize+1];
		int bytesToRead = blockSize;
		
		
		
		while(bytesRead < numBytes) {
			
			if((numBytes - bytesRead) < blockSize){
				bytesToRead = numBytes - bytesRead;
			}
			else if(blocksRead == startBlock){
				bytesToRead = blockSize - ofst;
			}
			
			bytesRead += bytesToRead;
			
			if(blocksRead < 12) { // seek to next entry in direct block table
				if(iNodeList[iNodeIndex].blockAddressTable[blocksRead] == -1){
					cout<<"Reached end of file while trying to read!"<<endl;
					break;
				}

				diskFile.seekg((OFFSET + iNodeList[iNodeIndex].blockAddressTable[blocksRead])* blockSize + ofst);
				diskFile.read(buf, bytesToRead);
			}
			else if(blocksRead < indBlockSize){
				int cur = blocksRead - 12;


				diskFile.seekg((OFFSET + iNodeList[iNodeIndex].ib.blockTable[cur])* blockSize + ofst);
				diskFile.read(buf, bytesToRead);				
			}
			else{
				int cur = blocksRead - indBlockSize;
				
				if(cur != 0 && cur % indBlockSize == 0)
					currentIndBlock += 1;

								
				diskFile.seekg((OFFSET + iNodeList[iNodeIndex].doubleIndBlockTable[currentIndBlock].blockTable[cur%indBlockSize]) * blockSize + ofst);
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


string fileSystem::list(){
	string fileList = "";
	
	for (int i = 0; i<256; i++) {
		if(iNodeList[i].getFileName() != (string) "~")
			fileList +=( iNodeList[i].getFileName() + (string)"\n");
		   
	}
	return fileList;
}

void fileSystem::importiNode(int iNodeIndex) {
	FILE * in = fopen(diskName.c_str(), "rb+wb");

	char iNodeBuf[sizeof(iNode2)];
	iNode2* node = new iNode2;

	fseek(in, (FREE_INODE_LIST_OFFSET + iNodeIndex)*blockSize,0);
	fread(iNodeBuf, sizeof(iNode2), 1, in);
	memcpy(node, &iNodeBuf, sizeof(iNode2));
	cout << "import reads: " << iNodeBuf << endl;

	for(int i = 0; i < 32; i++) iNodeList[iNodeIndex].fileName[i] = node->fName[i];
	for(int i = 0; i < 12; i++) iNodeList[iNodeIndex].blockAddressTable[i] = node->blockAddressTable[i];
	iNodeList[iNodeIndex].ib.pointer = node->indBlockPointer;
	iNodeList[iNodeIndex].doubleIndBlock = node->doubleIndBlockPointer;
	iNodeList[iNodeIndex].fSize = node->fSize;

	char blockBuf[blockSize];
	if(node->indBlockPointer != -1) { // done
		int curIndBlock[indBlockSize];

		int curPtr = iNodeList[iNodeIndex].ib.pointer;
		fseek(in, (curPtr*blockSize), 0);
		fread(blockBuf, blockSize, 1, in);
		memcpy(&curIndBlock, blockBuf, blockSize);

		for(int i = 0; i < indBlockSize; i++) {
			if(i > curIndBlock[i]) iNodeList[iNodeIndex].ib.blockTable = vector<int>();
			else iNodeList[iNodeIndex].ib.blockTable[i] = curIndBlock[i];
		}
	}

	/*
	char doubleBlockBuf[blockSize];
	if(node->doubleIndBlockPointer != -1) { // not done
		int doubleIndBlock[indBlockSize];
		int j = 0;
		in.seekg(curPtr*blockSize);
		in.read(doubleBlockBuf, blockSize);
		memcpy(doubleIndBlock, doubleBlockBuf, blockSize);
		// unfinished, need to reorder this stuff
	}*/

	fclose(in);
}


void fileSystem::convertiNode(int iNodeIndex){
	FILE * out = fopen(diskName.c_str(), "rb+wb");
/*
struct iNode2{
	char fName[32];
	int blockAddressTable[12];
	int indBlockPointer;
	int doubleIndBlockPointer;
	int fSize;

};
*/
	char* iNodeBuf[sizeof(iNode2)];


	iNode2 * node = new iNode2;

	for(int i = 0; i<32; i++) {
		node ->fName[i] = iNodeList[iNodeIndex].fileName[i];

		cout << "copying into node struct fName: " << node->fName[i] << endl;
	}

	for(int i = 0; i<12; i++) {
		node ->blockAddressTable[i] = iNodeList[iNodeIndex].blockAddressTable[i];
	
		cout << "copying into node struct block address table: " << node->blockAddressTable[i] << endl;
	}

	node -> indBlockPointer = iNodeList[iNodeIndex].ib.pointer;
	node -> doubleIndBlockPointer = iNodeList[iNodeIndex].doubleIndBlock;
	node -> fSize = iNodeList[iNodeIndex].fSize;

	memcpy(iNodeBuf, &node, sizeof(iNode2));
	cout << "writing contents: " << iNodeBuf <<endl;
	fseek(out, (FREE_INODE_LIST_OFFSET + iNodeIndex)*blockSize,0);
	fwrite(iNodeBuf, blockSize, 1, out);
	
	char* blockBuf = new char[blockSize];
	if(node -> indBlockPointer != -1){
		int curIndBlk[indBlockSize];

		int curPtr = iNodeList[iNodeIndex].ib.pointer;
		for(int i = 0; i<indBlockSize; i++){
			if(i > iNodeList[iNodeIndex].ib.blockTable.size()) curIndBlk[i] = -1;
			else curIndBlk[i] = iNodeList[iNodeIndex].ib.blockTable[i];
		}

		memcpy(blockBuf, curIndBlk, blockSize);
		fseek(out, curPtr*blockSize,0);
		fwrite(blockBuf, blockSize, 1, out);
	}


	char* doubleBlockBuf = new char[blockSize];
	if(node -> doubleIndBlockPointer != -1){
		int doubleIndBlock[indBlockSize];
		int j = 0;
		while(iNodeList[iNodeIndex].doubleIndBlockTable.size() != 0){
			int curIndBlk[indBlockSize];
			int curPtr = iNodeList[iNodeIndex].doubleIndBlockTable[j].pointer;
			for(int i = 0; i<indBlockSize; i++){
				if(i > iNodeList[iNodeIndex].doubleIndBlockTable.front().blockTable.size()) curIndBlk[i] = -1;
				else curIndBlk[i] = iNodeList[iNodeIndex].doubleIndBlockTable.front().blockTable[i];
			}
			doubleIndBlock[j] = curPtr;
			j++;
			
			memcpy(blockBuf, curIndBlk, blockSize);
			fseek(out, curPtr*blockSize,0);
			fwrite(blockBuf, blockSize, 1, out);
		}
		while(j < indBlockSize){
			doubleIndBlock[j] = -1;
			j++;
		}
		memcpy(doubleBlockBuf, doubleIndBlock, blockSize);
		fseek(out, (node -> doubleIndBlockPointer)*blockSize,0);
		fwrite(doubleBlockBuf, blockSize, 1, out);

	}

	fclose(out);

	cout << "out is closed, now opening in" << endl;

	char* testBuf[sizeof(iNode2)];
	FILE * in = fopen(diskName.c_str(), "r");
	fseek(in, (FREE_INODE_LIST_OFFSET + iNodeIndex)*blockSize,0);
	fread(testBuf, sizeof(iNode2), 1, in);
	cout << "reading contents: " << testBuf << endl;
	fclose(in);
}


void fileSystem::shutdown(){


	FILE * out = fopen(diskName.c_str(), "w");



	Superblock s;

	char sBuf[sizeof(Superblock)];


	s.offset = offset;
	s.blockSize = blockSize;
	s.numBlocks = numBlocks;
	s.hasFiles = 1;
	fseek(out, 0, 0);
	memcpy(sBuf, &s, sizeof(s));

	fwrite(sBuf, blockSize, 1, out);

	char freeiNodeBuf[256];

	for(int i=0; i<256; i++){
		if(freeiNodeList[i]) freeiNodeBuf[i] = '1';
		else freeiNodeBuf[i] = '0';
	}
	fseek(out, FREE_INODE_LIST_OFFSET*blockSize, 0);
	fwrite(freeiNodeBuf, blockSize, 1, out);

	char freeBlockBuf[256];

	for(int i=0; i<256; i++){
		if(freeBlockList[i]) freeBlockBuf[i] = '1';
		else freeBlockBuf[i] = '0';
		
	}
//	fseek(out, FREE_BLOCK_LIST_OFFSET*blockSize, 0);
//	fwrite(freeBlockBuf, blockSize, 1, out);



	for(int i = 0; i<256; i++){
		if(freeiNodeList[i] == 1) {
			convertiNode(i);
			importiNode(i);
		}
	}	




	
}
