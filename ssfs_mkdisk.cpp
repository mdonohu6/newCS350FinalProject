//
//  ssfs_mkdisk.cpp
//  
//
//  Created by oliver on 4/24/17.
//
//


#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>

using namespace std;

struct Superblock{
	int numBlocks;
	int blockSize;
	int offset; // in bytes
	int hasFiles;
	
};



/*
 _______________________________
|								|
|			Superblock			|
|  								|
|	int:numBlocks				|	<-----  size: 1 block
|	int:blockSize				|
|	int:offset					|
|	int:hasFiles				|
|_______________________________|
|								|
|			iNodeList			|   <----- size: 256 blocks
|								|
|_______________________________|
|								|
|			freeBlockList		|   <----- size: 1 block
|_______________________________|
|								|
|			freeiNodeList		|   <----- size: 1 block
|								|
|-------------------------------|	<-------
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
|_______________________________|
 
 
 */

int main(int argc, char** argv){
	
	int numBlocks, blockSize;	
	std::string fName;
	
	if(argc > 3)
		fName = argv[3];
	else
		fName = "DISK";

	numBlocks = atoi(argv[1]);
	blockSize = atoi(argv[2]);

	
	// create a binary file that we can write output to	

	// to create a file of numBlocks * blockSize, just open a binary file, and then 
	// write numBlocks * blockSize worth of bytes of garbarge, like the char 'x'

	// ofs file object that is the DISK FILE...we write to DISK FILE using ofs
	// ifs is used for reading from our DISK FILE
	std::ofstream ofs; 
	std::ifstream ifs;
	ofs.open(argv[3], std::ios::binary | std::ios::out);
	ifs.open(argv[3], std::ios:: binary | std::ios::in);
	

	Superblock sb;	
	sb.hasFiles = 0;
	sb.numBlocks = numBlocks;
	sb.blockSize = blockSize;
	sb.offset = 258 * blockSize;

	// write the entire superblock to the disk file
	ofs.write(reinterpret_cast<char*>(&sb), sizeof(Superblock));

	char junk[1] = {'z'};
	char junk_2[1] = {'x'};
	char read_buf[1];
	// fill entire file with the char z
	
	// this for loop seeks to the point right after the very 
	// first block (which contains the superblock)
	// and writes z's into the file
	for (int i = blockSize; i < (numBlocks + 258) * blockSize; i++) {
		if( i < (numBlocks * blockSize -1)) {
			// fill whole file with z's
			ofs.seekp(i);
			ofs.write(junk, sizeof(char));
			
		} else {
			// put an x in the last spot of the file
			ofs.seekp(i);
			ofs.write(junk_2, sizeof(char));
		}
	}
	ofs.close();

	ifs.seekg(0);

	//Superblock blank;
	//ifs.read(reinterpret_cast<char*>(&blank),sizeof(Superblock));

	//cout << "reading from file, Superblock has numBlocks = " << blank.numBlocks << endl;
	//cout << "reading from file, Superblock has blockSize = " << blank.blockSize << endl;
	
	ofs.close();  



	  if (blockSize < 128 )
        perror ("Block Size cannot be less than 128 bytes.");
    else if (blockSize > 512)
        perror ("Block Size cannot be greater than 512 bytes.");

	cout << "made it to the bottom " << endl;
    
	//TODO
	// write out Superblock (first 1 or 2 block)
	// block size, numBlocks, boolean hasFiles
	
}
