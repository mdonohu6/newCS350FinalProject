//
//  iNode.cpp
//
//
//  Created by oliver on 4/6/17.
//
//

#include "iNode.hpp"

// default all values
iNode::iNode() {
    fSize = -1;
    for(int i = 0; i < 12; i++) blockAddressTable[i] = -1;
    indBlock = -1;
    doubleIndBlock = -1;
    for(int i = 0; i < 32; i++) fileName[i] = 'x';
}

// best to copy vals like this than to assign pointers to the original arrays
iNode::iNode(int a_fSize, int a_blockAddressTable[12], int a_indBlock, int a_doubleIndBlock, char a_fileName[32], int d) {
    fSize = a_fSize;
    for(int i = 0; i < 12; i++) blockAddressTable[i] = a_blockAddressTable[i];
    indBlock = a_indBlock;
    doubleIndBlock = a_doubleIndBlock;
    for(int i = 0; i < 32; i++) fileName[i] = a_fileName[i];
	indBlockTable = new int[d];
}


iNode::iNode(char a_fileName[32], int d) {
	fSize = 0;
	for(int i = 0; i < 12; i++) blockAddressTable[i] = -1;
	indBlock = -1;
	doubleIndBlock = -1;
	for(int i = 0; i < 32; i++) fileName[i] = a_fileName[i];
	indBlockTable = new int[d];
	
}


char * iNode::getFileName() {
    return fileName;
}

void iNode::setFileName(char* fn) {
    strcpy(fileName, fn);
}
