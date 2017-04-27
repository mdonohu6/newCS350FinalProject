//
//  iNode.hpp
//
//
//  Created by oliver on 4/6/17.
//
//

#ifndef iNode_hpp
#define iNode_hpp

#include <stdio.h>
#include <string>

class iNode{
public:
    iNode();
    iNode(int, int[12], int, int, char[32]);
    char * getFileName();
    void setFileName(char* fn);
    
private:
    // fileName is char arry of size 32 bytes, as specified in Program5 instructions
    int fSize;
    int blockAddressTable[12];
    int indBlock;
    int doubleIndBlock;
    char fileName[32];
    
};




#endif /* iNode_hpp */
