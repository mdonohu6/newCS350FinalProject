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
    fSize = -1;  //in blocks 
    for(int i = 0; i < 12; i++) blockAddressTable[i] = -1;
    ib.pointer = -1;
    doubleIndBlock = -1;
	fileName[0] = '~';
	fileName[1] = '\0';

}


// best to copy vals like this than to assign pointers to the original arrays
iNode::iNode(int a_fSize, int a_blockAddressTable[12], int a_indBlock, int a_doubleIndBlock, char a_fileName[32]) {
    fSize = a_fSize;
    for(int i = 0; i < 12; i++) blockAddressTable[i] = a_blockAddressTable[i];
    ib.pointer = a_indBlock;
    doubleIndBlock = a_doubleIndBlock;
    for(int i = 0; i < 32; i++) fileName[i] = a_fileName[i];
}


iNode::iNode(char a_fileName[32]) {
	fSize = 0;
	for(int i = 0; i < 12; i++) blockAddressTable[i] = -1;
	ib.pointer = -1;
	doubleIndBlock = -1;
	for(int i = 0; i < 32; i++) fileName[i] = a_fileName[i];
	
}


char * iNode::getFileName() {
    return fileName;
}

void iNode::setFileName(char* fn) {
    strcpy(fileName, fn);
}
