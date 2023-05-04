#ifndef utill_h
#define utill_h

#include <iostream>

#include "nodeDht.h"

using namespace std;

#define ll long long int

class utillFunctions{

	public:

		vector<string> splitCommand(string command);

		string combineIpAndPort(string ip,string port);

		vector< pair<ll,string> > seperateKeysAndValues(string keysAndValues);

		vector< pair<string,int> > seperateSuccessorList(string succList);
		string splitSuccessorList(vector< pair< pair<string,int> , ll > > list);
		
		ll getHash(string key);
		pair<string,int> getIpAndPort(string key);
		
		bool isKeyValue(string id);

		bool isNodeAlive(string ip,int port);
		
		void setServerDetails(struct sockaddr_in &server,string ip,int port);
		void setTimer(struct timeval &timer);

		void sendNeccessaryKeys(NodeDht &nodeInfo,int newSock,struct sockaddr_in client,string nodeIdString);
		void sendKeyToNode(pair< pair<string,int> , ll > node,ll keyHash,string value);
		void sendValToNode(NodeDht nodeInfo,int newSock,struct sockaddr_in client,string nodeIdString);
		string getKeyFromNode(pair< pair<string,int> , ll > node,string keyHash);
		pair<ll,string> getKeyAndVal(string keyAndVal);
		void getKeysFromSuccessor(NodeDht &nodeInfo,string ip,int port);
		void storeAllKeys(NodeDht &nodeInfo,string keysAndValues);

		pair< pair<string,int> , ll > getPredecessorNode(string ip,int port,string ipClient,int ipPort,bool forStabilize);
		ll getSuccessorId(string ip,int port);

		void sendPredecessor(NodeDht nodeInfo,int newSock,struct sockaddr_in client);
		void sendSuccessor(NodeDht nodeInfo,string nodeIdString,int newSock,struct sockaddr_in client);
		void sendSuccessorId(NodeDht nodeInfo,int newSock,struct sockaddr_in client);
		void sendAcknowledgement(int newSock,struct sockaddr_in client);
		//Finger Table
		vector< pair<string,int> > getSuccessorListFromNode(string ip,int port);
		void sendSuccessorList(NodeDht &nodeInfo,int sock,struct sockaddr_in client);
};


#endif