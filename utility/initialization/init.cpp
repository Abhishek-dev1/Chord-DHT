#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <thread>
#include "init.h"
#include "port.h"
#include "functions.h"
#include "utill.h"
#include "nodeDht.h"

using namespace std;

void initialize(){
	
	NodeDht nodeInfo = NodeDht();

	//open a socket to listen to other nodes 
	nodeInfo.sp.specifyPortServer();

	cout<<"listening at port number "<<nodeInfo.sp.getPortNumber()<<endl;

	cout<<"showHelp\n";

	string command;

	while(1){
		cout<<"> ";
		getline(cin,command);

		//find space in command and seperate arguments
		utillFunctions help = utillFunctions();
		vector<string> arguments = help.splitCommand(command);

		string arg = arguments[0];
		if(arguments.size() == 1){

			//creates a node
			if(arg == "create"){
				//if already in the ring then
				if(nodeInfo.getStatus() == 1){
					cout<<"Already on the ring\n";
				}
				else{
					thread first(create,ref(nodeInfo));
					first.detach();
				}
			}

			//To print
			else if(arg == "printstate"){
				if(nodeInfo.getStatus() == 0){
					cout<<"Error! Not in the ring !\n";
				}
				else
					printState(nodeInfo);
			}

			//leaves 
			else if(arg == "leave"){
				leave(nodeInfo);
				nodeInfo.sp.closeSocket();
				return;
			}

			//print current port number
			else if(arg == "port"){
				cout<<nodeInfo.sp.getPortNumber()<<endl;
			}

			//print keys present in this node 
			else if(arg == "print"){
				if(nodeInfo.getStatus() == false){
					cout<<"Error not in the ring\n";
				}
				else
					nodeInfo.printKeys();
			}

			else if(arg == "help"){
				showHelp();
			}

			else{
				cout<<"Invalid\n";
			}
		}

		else if(arguments.size() == 2){

			if(arg == "port"){
				if(nodeInfo.getStatus() == true){
					cout<<":( port number can't be change !\n";
				}
				else{
					int newPortNo = atoi(arguments[1].c_str());
					nodeInfo.sp.changePortNumber(newPortNo);
				}
			}

			else if(arg == "get"){
				if(nodeInfo.getStatus() == false){
					cout<<"Error not in the ring\n";
				}
				else
					get(arguments[1],nodeInfo);
			}

			else{
				cout<<"Invalid Command\n";
			}
		}

		else if(arguments.size() == 3){

			if(arg == "join"){
				if(nodeInfo.getStatus() == true){
					cout<<"Error already on the ring\n";
				}
				else
					join(nodeInfo,arguments[1],arguments[2]);
			}

			//putting the key value to required node
			else if(arg == "put"){
				if(nodeInfo.getStatus() == false){
					cout<<"Error not in the ring\n";
				}
				else
					put(arguments[1],arguments[2],nodeInfo);
			}

			else{
				cout<<"Invalid\n";
			}
		}

		else{
			cout<<"Invalid Format\n";
		}
	}
	
}