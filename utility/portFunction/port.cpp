#include <iostream>

#include "headers.h"
#include "port.h"

//Port number generator
void SocketAndPort::specifyPortServer(){

	srand(time(0));
	portNoServer = rand() % 65536;
	if(portNoServer < 1024)
		portNoServer += 1024;

	socklen_t len = sizeof(current);

	sock = socket(AF_INET,SOCK_DGRAM,0);
	current.sin_family = AF_INET;
	current.sin_port = htons(portNoServer);
	current.sin_addr.s_addr = inet_addr("127.0.0.1");

	if( bind(sock,(struct sockaddr *)&current,len) < 0){
		perror("ERROR! Binding the port");
		exit(-1);
	}

}
bool SocketAndPort::portInUse(int portNo){
	int newSock = socket(AF_INET,SOCK_DGRAM,0);

	struct sockaddr_in newCurr;
	socklen_t len = sizeof(newCurr);
	newCurr.sin_port = htons(portNo);
	newCurr.sin_family = AF_INET;
	newCurr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	if( bind(newSock,(struct sockaddr *)&newCurr,len) < 0){
		perror("error");
		return true;
	}
	else{
		close(newSock);
		return false;
	}
}


void SocketAndPort::changePortNumber(int newPortNumber){
	if(newPortNumber < 1024 || newPortNumber > 65535){
		cout<<"Not a valid Port number\n";
	}
	else{
		if(portInUse(newPortNumber) ){
			cout<<"Port already in Use\n";
		}
		else{
			close(sock);
			socklen_t len = sizeof(current);
			sock = socket(AF_INET,SOCK_DGRAM,0); 
			current.sin_port = htons(newPortNumber);
			if( bind(sock,(struct sockaddr *)&current,len) < 0){
				perror("ERROR! Binding To Port");
				current.sin_port = htons(portNoServer);
			}
			else{
				portNoServer = newPortNumber;
				cout<<"Port number changed to : "<<portNoServer<<endl;
			}
		}
	}
}

//Ipv4 address 
string SocketAndPort::getIpAddress(){
	string ip = inet_ntoa(current.sin_addr);
	return ip;
}

//Port number on which it's listening
int SocketAndPort::getPortNumber(){
	return portNoServer;
}

//socket file descriptor 
int SocketAndPort::getSocketFd(){
	return sock;
}

void SocketAndPort::closeSocket(){
	close(sock);
}
