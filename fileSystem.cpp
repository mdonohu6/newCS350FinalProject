//
//  fileSystem.cpp
//  
//
//  Created by oliver on 4/24/17.
//
//

#include "fileSystem.hpp"
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
	
	
	ifstream ifs;	
	ifs.open(fileName, ios::in | ios::binary);

	Superblock blank;

	ifs.read(reinterpret_cast<char*>(&blank),sizeof(Superblock));

	numBlocks = blank.numBlocks;
	blockSize = blank.blockSize;
	
	
	
	
	
	//the number of block pointers you can fit in an indirect block
	indBlockSize = blockSize / 4;
	
	
	
	//the number of indirect block pointers you can fit in a double indirect block
	doubleIndSize = blockSize / 4;

	
	freeBlockList = new bool[numBlocks];
	
	if(blank.hasFiles){

	}

	ifs.close();
	
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

	ofstream diskFile;
	diskFile.open(diskName, ios::in | ios::out | ios::binary | ios::ate);


	

	//find index in iNodeList where ssfs file is
	int iNodeIndex;
	for(iNodeIndex = 0; iNodeIndex<256; iNodeIndex++)
		if(iNodeList[iNodeIndex].getFileName() == ssfsFName)
			break;


	iNodeList[iNodeIndex].fSize = getFileSize(unixFName);

											  
	//open the unix file
	ifstream unixFile;
	unixFile.open(unixFName, ios::binary | ios::in);
		  
	
	
	
	
	
									
	char toBeWritten[blockSize];
	
	//used to keep track of file size
	int blocksRead = 0;
	
	
	
	
	//MARK: Need to use fileSize instead of .eof
	while(!unixFile.eof()){
		
		blocksRead++;
		
		//read 1 block of data from unixFile
		cout<<"blockSize: "<<blockSize<<endl;
	
		
		
		
		
		//MARK: BUG!!!! reads blockSize/2 bytes instead of blockSize

		unixFile.read(toBeWritten,blockSize);
		
		cout<<"toBeWritten "<<toBeWritten<<";"<<endl;
		
		
		
	
		for(int blockNum = 0; blockNum<numBlocks; blockNum++){
		
			
			//find free block in freeBlockList
			if(freeBlockList[blockNum] == 0){
				
				freeBlockList[blockNum] = 1;
				
				if(blocksRead < 12){ //direct blocks
					
					iNodeList[iNodeIndex].blockAddressTable[blocksRead] = blockNum;
					
					
					//Write toBeWritten to ( (offset + blockNum)* blockSize)
					diskFile.seekp((OFFSET+blockNum)*blockSize);
				
					cout<<diskFile.tellp()<<endl;

					diskFile.write(toBeWritten,blockSize);

					
					
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
					
					diskFile.seekp((OFFSET+blockNum)*blockSize);
					diskFile.write(toBeWritten,blockSize);

					
					
				
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
	
					
					diskFile.seekp((OFFSET+blockNum)*blockSize);
					diskFile.write(toBeWritten,blockSize);
					
				}
				
				break;
			}
		}
		
	}
	unixFile.close();
	diskFile.close();
}


// if this is not working, read the comments in second for loop below -- most likely cause of problems
void fileSystem::cat(string ssfsFName){
	
	
	cout<<"CAT "<<ssfsFName<<endl;
	
	
	//find iNode in iNodeList
	int iNodeIndex;
	for(iNodeIndex = 0; iNodeIndex<256; iNodeIndex++)
		if(iNodeList[iNodeIndex].getFileName() == ssfsFName)
			break;

	if(iNodeIndex < 256) {
		cout<<"in"<<endl;
		ifstream diskFile;
		diskFile.open(diskName, ios::binary | ios::in | ios::out);

		int bytesRead = 0;
		int directBlocksRead = 0; // keep track of the 12 blocks we must iterate through first
		int indirectBlocksRead = 0; // keep track of how many blocks we've read from the current indirect block being pointed to (changes once we enter double indirect blocks)
		int doubleIndirectBlocksRead = 0; // keep track of what # indirect block we should be looking at from the doubleIndBlock vector in currentNode
		iNode* currentNode = &iNodeList[iNodeIndex]; // store this indexed value just because it gets referenced so often below
		indBlock* currentIndBlock = &currentNode->ib; // start off with the single indirect block assigned to this inode
		// after reading this first indBlock, this value will iterate through the list of indBlocks in the doubleIndBlockPtr vector of indBlocks

		while(bytesRead < currentNode->fSize) {
			bytesRead++;
			
			
			if(directBlocksRead < 12) { // seek to next entry in direct block table
				if(currentNode->blockAddressTable[directBlocksRead] == -1) break;

				diskFile.seekg((OFFSET + currentNode->blockAddressTable[directBlocksRead])* blockSize);
				cout<<(OFFSET + currentNode->blockAddressTable[directBlocksRead])<<endl;

				directBlocksRead++;
			} else if(indirectBlocksRead < currentIndBlock->blockTable.size()){ // seek to the next entry in indirect block table
				if(currentIndBlock->blockTable[indirectBlocksRead] == -1) break;

				diskFile.seekg(OFFSET*blockSize + (currentIndBlock->blockTable[indirectBlocksRead] * blockSize));

				indirectBlocksRead++;
			} else {
				// following three lines allow us to iterate through this vector of indBlocks in the same manner we did above to the initial indBlock
				currentIndBlock = &currentNode->doubleIndBlockTable[doubleIndirectBlocksRead];
				indirectBlocksRead = 0;
				doubleIndirectBlocksRead++;

				continue;
			}

			char buf;
			for(int i = 0; i < blockSize; i++) {
				diskFile.read(&buf, 1); // read one char at a time because we need to identify when we reach the end of the file
				diskFile.seekg(diskFile.cur + 1); // move read pointer to next char in file
				if(buf == '\0') { // *IMPORTANT* \0 acting as a placeholder for whatever the file will be populated with as empty space
					// acting as a sentinel since break will only leave the for loop
					doubleIndirectBlocksRead = currentNode->doubleIndBlockTable.size();

					break;
				}

				cout << buf;
			}
		}

		cout << endl; // spacing after all of the data has been put out with no endls between

		diskFile.close();
	} else { // file was not found in the list
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

		int bytesRemaining = numBytes; // keeps track of how many bytes we still need to read out
		int directBlocksRead = 0; // keep track of the 12 blocks we must iterate through first
		int indirectBlocksRead = 0; // keep track of how many blocks we've read from the current indirect block being pointed to (changes once we enter double indirect blocks)
		int doubleIndirectBlocksRead = 0; // keep track of what # indirect block we should be looking at from the doubleIndBlock vector in currentNode
		int bytesToWrite; // will determine how many bytes we write below after seeking to the correct position in the file
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
					outFile.seekp(OFFSET*blockSize + (currentNode->blockAddressTable[directBlocksRead] * blockSize));

					directBlocksRead++;
				} else if(indirectBlocksRead < currentIndBlock->blockTable.size()){ // seek to the next entry in indirect block table
					outFile.seekp(OFFSET*blockSize + (currentIndBlock->blockTable[indirectBlocksRead] * blockSize));

					indirectBlocksRead++;
				} else {
					// following three lines allow us to iterate through this vector of indBlocks in the same manner we did above to the initial indBlock
					currentIndBlock = &currentNode->doubleIndBlockTable[doubleIndirectBlocksRead];
					indirectBlocksRead = 0;
					doubleIndirectBlocksRead++;

					continue;
				}

			
				// go for the whole block if bytesRemaining is >= blockSize
				bytesToWrite = min(blockSize, bytesRemaining);

				if(startByte) { // adjust number of bytes we must write in this block to be contained within the block still
					outFile.seekp(outFile.cur + startByte);
					if(bytesToWrite + startByte > blockSize) {
						bytesToWrite = blockSize - startByte;
					}

					startByte = 0; // we've already used this by this point
				}

				bytesRemaining -= bytesToWrite;

				// finally, write to the block
				for(int i = 0; i < bytesToWrite; i++) {
					outFile.write(&ch, sizeof(ch));
					outFile.seekp(outFile.cur + 1);
				}
			}
		}

		cout << endl; // spacing after all of the data has been put out with no endls between

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

	ofstream diskFile;
	diskFile.open(diskName, ios::in | ios::out | ios::binary | ios::ate);

	//MARK: Maybe write superblock out to file, just to be safe
	
	
	
	for(int i=0; i<256; i++){
		if(freeiNodeList[i]){
			diskFile.seekp((1+i)*blockSize);

			//TODO: write out all inode information to disk in an easy to parse format
		}
	}
	

	
	
	diskFile.seekp((FREE_BLOCK_LIST_OFFSET)*blockSize);

	//Write freeBlockList out to disk
	for(int i=0; i<numBlocks; i++){
		if(freeBlockList[i])
			diskFile.write("1",sizeof(char));
		else
			diskFile.write("0",sizeof(char));

	}
	
	
	//TODO: Write freeInodeList out to disk
	diskFile.seekp((FREE_INODE_LIST_OFFSET)*blockSize);
	for(int i=0; i<256; i++){
		if(freeiNodeList[i])
			diskFile.write("1",sizeof(char));
		else
			diskFile.write("0",sizeof(char));
		
	}
	
	
	
	diskFile.close();
	
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

 
 */
