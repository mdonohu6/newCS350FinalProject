//
//  fileSystem.cpp
//  
//
//  Created by oliver on 4/24/17.
//
//

#include "fileSystem.hpp"
using namespace std;



struct Superblock{
	int numBlocks;
	int blockSize;
	int offset; // in bytes
	int hasFiles;
	
};



fileSystem::fileSystem(string diskName){
	ifstream ifs;	
	ifs.open(diskName, ios::in | ios::binary);

	Superblock blank;

	ifs.read(reinterpret_cast<char*>(&blank),sizeof(Superblock));

	cout << "reading from file, Superblock has numBlocks = " << blank.numBlocks << endl;
	cout << "reading from file, Superblock has blockSize = " << blank.blockSize << endl;

	offset = blank.offset;
	numBlocks = blank.numBlocks;
	blockSize = blank.numBlocks;
	
	
	
	indBlockSize = blockSize / log2(numBlocks);
	
	//This is wrong***********************************
	doubleIndSize = blockSize / log2(numBlocks);

	
	freeBlockList = new bool[numBlocks];
	
	if(blank.hasFiles){

	}
	
}

void fileSystem::create(string ssfsFName){
    char tab2[1024];
    strncpy(tab2, ssfsFName.c_str(), sizeof(tab2));
    tab2[sizeof(tab2) - 1] = 0;
    bool unique = true;
    for (int i = 0; i < 256; i++) {
        if (iNodeList[i].getFileName() == ssfsFName)
            unique = false;
    }
    
    iNode input(tab2, indBlockSize);
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
	
	
	
	
	ifstream unixFile;
	unixFile.open(unixFName, ios::binary | ios::in);
	
	char toBeWritten[blockSize];
	
	//used to keep track of file size
	int blocksInFile = 0;
	
	
	
	
	
	while(!unixFile.eof()){
		
		blocksInFile++;
		
		//read 1 block of data from unixFile
		unixFile.read(toBeWritten,blockSize);
		
		
		
		for(int blockNum = 0; blockNum<numBlocks; blockNum++){
		
			
			//find free block in freeBlockList
			if(freeBlockList[blockNum] == 0){
				
				freeBlockList[blockNum] = 1;
				
				if(blocksInFile < 12){ //direct blocks
					
					iNodeList[iNodeIndex].blockAddressTable[blocksInFile] = blockNum;
					
					
					//TODO: Write toBeWritten to ( offset + (blockNum * blockSize))
					
					
					
					
				}
				else if(blocksInFile < indBlockSize){ //single indirect block pointer
				
					
					
					
					if(blocksInFile == 12){ // on 13th block need to make indirect block
						
						for (int i = 0; i<numBlocks; i++) {
							if(freeBlockList[i] ==0){
								freeBlockList[i] = 1;
								iNodeList[iNodeIndex].ib.pointer = blockNum;
								
								break;
							}
						}
					}

					
					
				
					iNodeList[iNodeIndex].ib.blockTable.push_back(blockNum);
					
					//TODO Write toBeWritten of data to ( offset + (blockNum * blockSize))

					
					
				
				}
				else{	//double indirect block pointer


					if(blocksInFile == doubleIndSize){// need to make double indirect block
						
						for (int i = 0; i<numBlocks; i++){
							if(freeBlockList[i] ==0){
								freeBlockList[i] = 1;
								iNodeList[iNodeIndex].doubleIndBlock = blockNum;
								break;
							}
						}
					}
					
					
					
					
					
					
					
					int currentBlock = blocksInFile - doubleIndSize;
					
					if(currentBlock % indBlockSize == 0){ //when an indirect block is filled we create another
						
						for (int i = 0; i<numBlocks; i++) {
							if(freeBlockList[i] ==0){

								freeBlockList[i] = 1;
							
								
								indBlock newIB;
								newIB.pointer = i;

								iNodeList[iNodeIndex].doubleIndBlockTable.push_back(newIB);

								break;
							}
							
						}
					}
					
					(iNodeList[iNodeIndex].doubleIndBlockTable.front()).blockTable.push_back(blockNum);
					

					
				}
				
				break;
			}
		}
		
	}
}



void fileSystem::cat(string ssfsFName){
	
	
}

void fileSystem::del(string ssfsFName){
	

	
}

void fileSystem::write(string ssfsFName, char ch, int startByte, int numBytes){

	
	
	
}

void fileSystem::read(string ssfsFName, int startByte, int numBytes){
	
}


string fileSystem::list(){
	

	return "place holder";
}

void fileSystem::shutdown(){
	
}
