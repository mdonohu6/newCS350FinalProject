//
//  ssfs.cpp
//  
//
//  Created by oliver on 4/20/17.
//
//

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <stdlib.h>
#include "fileSystem.hpp"

using namespace std;

void split(string in, vector<string> out){
	stringstream stream(in);
	string tempString;
	
	while(getline(stream,tempString,' ')) {
		out.push_back(tempString);
	}
	
	cerr << "Here" << endl;

}
void parseInputFile(char* diskName, char* fName){
	
	ifstream input;
  input.open(fName, fstream::in);
	
	string command;
	vector <string> command_split;
	
	fileSystem fileSys(diskName);
	
	while(!input.eof()){
		
		string ssfsFName, unixFName;
		
		int startByte, numBytes;
		
		char ch;
		
		
		getline(input, command);
		
		split(command,command_split);
		
		if(command_split[0] == "CREATE"){
			
			ssfsFName = command_split[1];
			
			fileSys.create(ssfsFName);	
		}
		else if(command_split[0] == "IMPORT"){
			ssfsFName = command_split[1];
			unixFName = command_split[2];
			
			
			fileSys.import(ssfsFName,unixFName);
			
			
		}
		else if(command_split[0] == "CAT"){
			ssfsFName = command_split[1];
			
			fileSys.cat(ssfsFName);
			
			
		}
		else if(command_split[0] == "DELETE"){
			ssfsFName = command_split[1];
			fileSys.del(ssfsFName);

			
		}
		else if(command_split[0] == "WRITE"){
			ssfsFName = command_split[1];
			ch = command_split[2][0];
			startByte = atoi(command_split[3].c_str());
			numBytes = atoi(command_split[4].c_str());
			
			fileSys.write(ssfsFName, ch, startByte, numBytes);
			

		}
		else if(command_split[0] == "READ"){
			ssfsFName = command_split[1];
			startByte = atoi(command_split[3].c_str());
			numBytes = atoi(command_split[4].c_str());
			
			fileSys.read(ssfsFName, startByte, numBytes);
			
			
		}
		else if(command_split[0] == "LIST"){
			fileSys.list();
			
			
		}
		else if(command_split[0] == "SHUTDOWN"){
			fileSys.shutdown();
			
			
		}	
		command_split.clear();
	}
}

int main(int argc, char **argv){
	
    ifstream input1;    // input2, input3, input4;   we'll work with one input file for now

	
	//make fileSystem object


/*
    if(argc > 3){
        input2.open(argv[3], fstream::in);
		create thread
 
    }
    if(argc > 4){
        input3.open(argv[4], fstream::in);
    }
    if(argc > 3){
        input4.open(argv[5], fstream::in);
    }
*/
	
    parseInputFile(argv[1],argv[2]);
	
	return 0;

}




