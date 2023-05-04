#ifndef functions_h
#define functions_h

#include <iostream>
#include "port.h"
#include "nodeDht.h"

using namespace std;

#define ll long long int

void put(string key,string value,NodeDht &nodeInfo);
void get(string key,NodeDht nodeInfo);
void create(NodeDht &nodeInfo);
void join(NodeDht &nodeInfo,string ip,string port);
void printState(NodeDht nodeInfo);
void listenTo(NodeDht &nodeInfo);
void doStabilize(NodeDht &nodeInfo);
void callNotify(NodeDht &nodeInfo,string ipAndPort);
void callFixFingers(NodeDht &nodeInfo);
void doTask(NodeDht &nodeInfo,int newSock,struct sockaddr_in client,string nodeIdString);
void leave(NodeDht &nodeInfo);
void showHelp();

#endif