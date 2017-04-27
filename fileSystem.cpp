//
//  fileSystem.cpp
//  
//
//  Created by oliver on 4/24/17.
//
//

#include "fileSystem.hpp"
using namespace std;




//DUPLICATE CODE REMOVE LATER


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

	if(blank.hasFiles){
		//look through the disk and read data into our class
	}
}

void fileSystem::create(string ssfsFName){
	
}
void fileSystem::import(string ssfsFName, string unixFName){
	
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
	
}

void fileSystem::shutdown(){
	
}
