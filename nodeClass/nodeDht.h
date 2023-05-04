#ifndef nodeInfo_h
#define nodeInfo_h

#include <iostream>
#include <vector>
#include <map>
#include "port.h"
#include "Max_lim.h"

using namespace std;

#define ll long long int

class NodeDht {
private:
	ll id;
	pair< pair<string, int> , ll > predecessor;
	pair< pair<string, int> , ll > successor;
	vector< pair< pair<string, int> , ll > > fingerTable;
	map<ll, string> dictionary;
	vector< pair< pair<string, int> , ll > > successorList;

	bool isInRing;

public:
	SocketAndPort sp;

	NodeDht();

	pair< pair<string, int> , ll > findSuccessor(ll nodeId);
	pair< pair<string, int> , ll > closestPrecedingNode(ll nodeId);
	void fixFingers();
	void stabilize();
	void notify(pair< pair<string, int> , ll > node);
	void checkPredecessor();
	void checkSuccessor();
	void updateSuccessorList();

	void printKeys();
	void storeKey(ll key, string val);
	vector< pair<ll , string> > getAllKeysForSuccessor();
	vector< pair<ll , string> > getKeysForPredecessor(ll nodeId);

	void setSuccessor(string ip, int port, ll hash);
	void setSuccessorList(string ip, int port, ll hash);
	void setPredecessor(string ip, int port, ll hash);
	void setFingerTable(string ip, int port, ll hash);
	void setId(ll id);
	void setStatus();

	ll getId();
	string getValue(ll key);
	vector< pair< pair<string, int> , ll > > getFingerTable();
	pair< pair<string, int> , ll > getSuccessor();
	pair< pair<string, int> , ll > getPredecessor();
	vector< pair< pair<string, int> , ll > > getSuccessorList();
	bool getStatus();
};

#endif