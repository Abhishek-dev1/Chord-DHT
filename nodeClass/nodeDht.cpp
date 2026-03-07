#include <iostream>
#include <ctime>
#include "headers.h"
#include "Max_lim.h"
#include "nodeDht.h"
#include "functions.h"
#include "utill.h"
using namespace std;

NodeDht::NodeDht() {
	fingerTable = vector< pair< pair<string, int> , ll > >(M + 1);
	successorList = vector< pair< pair<string, int> , ll > >(R + 1);
	isInRing = false;
	shouldShutdown = false;
}
void NodeDht::setSuccessor(string ip, int port, ll hash) {
	lock_guard<recursive_mutex> lock(dataMutex);
	successor.first.first = ip;
	successor.first.second = port;
	successor.second = hash;
}
void NodeDht::setStatus() {
	lock_guard<recursive_mutex> lock(dataMutex);
	isInRing = true;
}

void NodeDht::clearStatus() {
	lock_guard<recursive_mutex> lock(dataMutex);
	isInRing = false;
}

void NodeDht::setId(ll nodeId) {
	lock_guard<recursive_mutex> lock(dataMutex);
	id = nodeId;
}

vector< pair< pair<string, int> , ll > > NodeDht::getFingerTable() {
	lock_guard<recursive_mutex> lock(dataMutex);
	return fingerTable;
}

ll NodeDht::getId() {
	lock_guard<recursive_mutex> lock(dataMutex);
	return id;
}
pair< pair<string, int> , ll > NodeDht::getSuccessor() {
	lock_guard<recursive_mutex> lock(dataMutex);
	return successor;
}

pair< pair<string, int> , ll > NodeDht::getPredecessor() {
	lock_guard<recursive_mutex> lock(dataMutex);
	return predecessor;
}

vector< pair< pair<string, int> , ll > > NodeDht::getSuccessorList() {
	lock_guard<recursive_mutex> lock(dataMutex);
	return successorList;
}

bool NodeDht::getStatus() {
	lock_guard<recursive_mutex> lock(dataMutex);
	return isInRing;
}

string NodeDht::getValue(ll key) {
	lock_guard<recursive_mutex> lock(dataMutex);
	if (dictionary.find(key) != dictionary.end()) {
		return dictionary[key];
	}
	else
		return "";
}
void NodeDht::setFingerTable(string ip, int port, ll hash) {
	lock_guard<recursive_mutex> lock(dataMutex);
	for (int i = 1; i <= M; i++) {
		fingerTable[i] = make_pair(make_pair(ip, port), hash);
	}
}

void NodeDht::storeKey(ll key, string val) {
	lock_guard<recursive_mutex> lock(dataMutex);
	dictionary[key] = val;
}

void NodeDht::setSuccessorList(string ip, int port, ll hash) {
	lock_guard<recursive_mutex> lock(dataMutex);
	for (int i = 1; i <= R; i++) {
		successorList[i] = make_pair(make_pair(ip, port), hash);
	}
}

void NodeDht::setPredecessor(string ip, int port, ll hash) {
	lock_guard<recursive_mutex> lock(dataMutex);
	predecessor.first.first = ip;
	predecessor.first.second = port;
	predecessor.second = hash;
}





void NodeDht::printKeys() {
	lock_guard<recursive_mutex> lock(dataMutex);
	map<ll, string>::iterator it;

	for (it = dictionary.begin(); it != dictionary.end() ; it++) {
		cout << it->first << " " << it->second << endl;
	}
}

void NodeDht::updateSuccessorList() {
	pair< pair<string, int> , ll > currSuccessor;
	{
		lock_guard<recursive_mutex> lock(dataMutex);
		currSuccessor = successor;
	}

	utillFunctions help;

	vector< pair<string, int> > list = help.getSuccessorListFromNode(currSuccessor.first.first, currSuccessor.first.second);

	if (list.size() != R)
		return;

	lock_guard<recursive_mutex> lock(dataMutex);
	successorList[1] = currSuccessor;

	for (int i = 2; i <= R; i++) {
		successorList[i].first.first = list[i - 2].first;
		successorList[i].first.second = list[i - 2].second;
		successorList[i].second = help.getHash(list[i - 2].first + ":" + to_string(list[i - 2].second));
	}

}


//Send all keys of this node to it's successor after it leaves the ring
vector< pair<ll , string> > NodeDht::getAllKeysForSuccessor() {
	lock_guard<recursive_mutex> lock(dataMutex);
	map<ll, string>::iterator it;
	vector< pair<ll , string> > res;

	// Collect all keys first, THEN erase to avoid iterator invalidation
	for (it = dictionary.begin(); it != dictionary.end() ; ++it) {
		res.push_back(make_pair(it->first , it->second));
	}

	// Now erase all keys
	dictionary.clear();

	return res;
}

vector< pair<ll , string> > NodeDht::getKeysForPredecessor(ll nodeId) {
	lock_guard<recursive_mutex> lock(dataMutex);
	map<ll, string>::iterator it;
	vector< pair<ll , string> > res;
	vector<ll> keysToDelete;

	for (it = dictionary.begin(); it != dictionary.end() ; it++) {
		ll keyId = it->first;

		// Collect keys that should be moved
		bool shouldMove = false;

		// if predecessor's id is more than current node's id
		if (id < nodeId) {
			if (keyId > id && keyId <= nodeId) {
				shouldMove = true;
			}
		}
		// if predecessor's id is less than current node's id
		else {
			if (keyId <= nodeId || keyId > id) {
				shouldMove = true;
			}
		}

		if (shouldMove) {
			res.push_back(make_pair(keyId , it->second));
			keysToDelete.push_back(keyId);
		}
	}

	// Now erase the keys after iteration completes
	for (int i = 0; i < keysToDelete.size(); i++) {
		dictionary.erase(keysToDelete[i]);
	}

	return res;
}

pair< pair<string, int> , ll > NodeDht::findSuccessor(ll nodeId) {
	ll currId;
	pair< pair<string, int> , ll > currSuccessor;
	pair< pair<string, int> , ll > currPredecessor;
	{
		lock_guard<recursive_mutex> lock(dataMutex);
		currId = id;
		currSuccessor = successor;
		currPredecessor = predecessor;
	}

	pair < pair<string, int> , ll > self;
	self.first.first = sp.getIpAddress();
	self.first.second = sp.getPortNumber();
	self.second = currId;

	if (nodeId > currId && nodeId <= currSuccessor.second) {
		return currSuccessor;
	}

	else if (currId == currSuccessor.second || nodeId == currId) {
		return self;
	}

	else if (currSuccessor.second == currPredecessor.second) {
		if (currSuccessor.second >= currId) {
			if (nodeId > currSuccessor.second || nodeId < currId)
				return self;
		}
		else {
			if ((nodeId > currId && nodeId > currSuccessor.second) || (nodeId < currId && nodeId < currSuccessor.second))
				return currSuccessor;
			else
				return self;
		}
	}

	else {

		pair < pair<string, int> , ll > node = closestPrecedingNode(nodeId);
		if (node.second == currId) {
			return currSuccessor;
		}
		else {

			/* connect to node which will now find the successor */
			struct sockaddr_in serverToConnectTo;
			socklen_t len = sizeof(serverToConnectTo);

			string ip;
			int port;

			//if this node couldn't find closest preciding node for given node id then now ask it's successor to do so
			if (node.second == -1) {
				node = currSuccessor;
			}

			utillFunctions help;

			help.setServerDetails(serverToConnectTo, node.first.first, node.first.second);

			//set timer on this socket
			struct timeval timer;
			help.setTimer(timer);

			int sockT = socket(AF_INET, SOCK_DGRAM, 0);

			setsockopt(sockT, SOL_SOCKET, SO_RCVTIMEO, (char*)&timer, sizeof(struct timeval));

			if (sockT < 0) {
				cout << "socket cre error";
				perror("error: Socket creation failed in findSuccessor");
				pair < pair<string, int> , ll > node;
				node.first.first = "";
				node.second = -1;
				node.first.second = -1;
				return node;
			}

			// send the node id to the other node
			string nodeIdStr = to_string(nodeId);
			sendto(sockT, nodeIdStr.c_str(), nodeIdStr.length(), 0, (struct sockaddr*) &serverToConnectTo, len);

			// receive ip and port of node successor as ip:port
			char recvBuf[1024];

			int l = recvfrom(sockT, recvBuf, 1023, 0, (struct sockaddr *) &serverToConnectTo, &len);

			close(sockT);

			if (l < 0) {
				pair < pair<string, int> , ll > node;
				node.first.first = "";
				node.second = -1;
				node.first.second = -1;
				return node;
			}

			recvBuf[l] = '\0';

			//set ip,port and hash for this node and return it
			string key = recvBuf;
			ll hash = help.getHash(key);
			pair<string, int> ipAndPortPair = help.getIpAndPort(key);
			node.first.first = ipAndPortPair.first;
			node.first.second = ipAndPortPair.second;
			node.second = hash;

			return node;
		}
	}
	return std::make_pair(std::make_pair("", -1), -1);
}

pair< pair<string, int> , ll > NodeDht::closestPrecedingNode(ll nodeId) {
	vector< pair< pair<string, int> , ll > > localFingerTable;
	ll currId;
	{
		lock_guard<recursive_mutex> lock(dataMutex);
		localFingerTable = fingerTable;
		currId = id;
	}

	utillFunctions help;

	for (int i = M; i >= 1; i--) {
		if (localFingerTable[i].first.first == "" || localFingerTable[i].first.second == -1 || localFingerTable[i].second == -1) {
			continue;
		}

		if (localFingerTable[i].second > currId && localFingerTable[i].second < nodeId) {
			return localFingerTable[i];
		}
		else {

			ll successorId = help.getSuccessorId(localFingerTable[i].first.first, localFingerTable[i].first.second);

			if (successorId == -1)
				continue;

			if (localFingerTable[i].second > successorId) {
				if ((nodeId <= localFingerTable[i].second && nodeId <= successorId) || (nodeId >= localFingerTable[i].second && nodeId >= successorId)) {
					return localFingerTable[i];
				}
			}
			else if (localFingerTable[i].second < successorId && nodeId > localFingerTable[i].second && nodeId < successorId) {
				return localFingerTable[i];
			}

			pair< pair<string, int> , ll > predNode = help.getPredecessorNode(localFingerTable[i].first.first, localFingerTable[i].first.second, "", -1, false);
			ll predecessorId = predNode.second;

			if (predecessorId != -1 && localFingerTable[i].second < predecessorId) {
				if ((nodeId <= localFingerTable[i].second && nodeId <= predecessorId) || (nodeId >= localFingerTable[i].second && nodeId >= predecessorId)) {
					return predNode;
				}
			}
			if (predecessorId != -1 && localFingerTable[i].second > predecessorId && nodeId >= predecessorId && nodeId <= localFingerTable[i].second) {
				return predNode;
			}
		}
	}
	// if above conditions will not work then return this
	//(just for removing error occuring due to no return statement of this function i guess it never come here)
	pair< pair<string, int> , ll > node;
	node.first.first = "";
	node.first.second = -1;
	node.second = -1;
	return node;
}

void NodeDht::stabilize() {
	utillFunctions help;
	string ownIp = sp.getIpAddress();
	int ownPort = sp.getPortNumber();
	ll currId;
	pair< pair<string, int> , ll > currSuccessor;
	pair< pair<string, int> , ll > currPredecessor;
	{
		lock_guard<recursive_mutex> lock(dataMutex);
		currId = id;
		currSuccessor = successor;
		currPredecessor = predecessor;
	}

	// Recover from accidental self-loop successor when a predecessor exists.
	if (currSuccessor.second == currId && currPredecessor.second != -1 && currPredecessor.second != currId) {
		lock_guard<recursive_mutex> lock(dataMutex);
		successor = currPredecessor;
		currSuccessor = currPredecessor;
	}

	// If we still only know ourselves, nothing to stabilize remotely yet.
	if (currSuccessor.second == currId) {
		return;
	}

	if (help.isNodeAlive(currSuccessor.first.first, currSuccessor.first.second) == false)
		return;

	//get predecessor of successor
	pair< pair<string, int> , ll > predNode = help.getPredecessorNode(currSuccessor.first.first, currSuccessor.first.second, ownIp, ownPort, true);

	ll predecessorHash = predNode.second;

	if (predecessorHash == -1)
		return;

	bool shouldUpdate = false;
	if (currId < currSuccessor.second) {
		shouldUpdate = (predecessorHash > currId && predecessorHash < currSuccessor.second);
	}
	else if (currId > currSuccessor.second) {
		shouldUpdate = (predecessorHash > currId || predecessorHash < currSuccessor.second);
	}

	if (shouldUpdate) {
		lock_guard<recursive_mutex> lock(dataMutex);
		successor = predNode;
	}


}

//check if current node predecessor is still alive
void NodeDht::checkPredecessor() {
	pair< pair<string, int> , ll > currPredecessor;
	pair< pair<string, int> , ll > currSuccessor;
	ll currId;
	{
		lock_guard<recursive_mutex> lock(dataMutex);
		currPredecessor = predecessor;
		currSuccessor = successor;
		currId = id;
	}

	if (currPredecessor.second == -1)
		return;

	utillFunctions help;
	string ip = currPredecessor.first.first;
	int port = currPredecessor.first.second;

	if (help.isNodeAlive(ip, port) == false) {
		lock_guard<recursive_mutex> lock(dataMutex);
		// if node has same successor and predecessor then set node as it's successor itself
		if (currPredecessor.second == currSuccessor.second) {
			successor.first.first = sp.getIpAddress();
			successor.first.second = sp.getPortNumber();
			successor.second = currId;
			for (int i = 1; i <= R; i++) {
				successorList[i] = successor;
			}
		}
		predecessor.first.first = "";
		predecessor.first.second = -1;
		predecessor.second = -1;
	}

}

// check if current node's successor is still alive
void NodeDht::checkSuccessor() {
	pair< pair<string, int> , ll > currSuccessor;
	pair< pair<string, int> , ll > fallbackSuccessor;
	ll currId;
	{
		lock_guard<recursive_mutex> lock(dataMutex);
		currSuccessor = successor;
		fallbackSuccessor = successorList[2];
		currId = id;
	}

	if (currSuccessor.second == currId)
		return;

	utillFunctions help;
	string ip = currSuccessor.first.first;
	int port = currSuccessor.first.second;

	if (help.isNodeAlive(ip, port) == 0) {
		{
			lock_guard<recursive_mutex> lock(dataMutex);
			successor = fallbackSuccessor;
		}
		updateSuccessorList();
	}

}

void NodeDht::notify(pair< pair<string, int> , ll > node) {
	lock_guard<recursive_mutex> lock(dataMutex);

	// To get id of node and predecessor
	ll predecessorHash = predecessor.second;
	ll nodeHash = node.second;

	if (nodeHash == -1) {
		return;
	}

	bool shouldUpdate = false;
	if (predecessorHash == -1 || predecessorHash == id) {
		shouldUpdate = true;
	}
	else if (predecessorHash < id) {
		shouldUpdate = (nodeHash > predecessorHash && nodeHash < id);
	}
	else {
		shouldUpdate = (nodeHash > predecessorHash || nodeHash < id);
	}

	if (shouldUpdate) {
		predecessor = node;
	}

	// if node successor is node itself then set its successor to this node
	if (shouldUpdate && successor.second == id) {
		successor = node;
	}
}

void NodeDht::fixFingers() {
	utillFunctions help;
	int next = 1;
	ll mod = pow(2, M);
	ll currId = getId();

	while (next <= M) {
		pair< pair<string, int> , ll > currSuccessor = getSuccessor();
		if (help.isNodeAlive(currSuccessor.first.first, currSuccessor.first.second) == false)
			return;

		ll newId = currId + pow(2, next - 1);
		newId = newId % mod;
		pair< pair<string, int> , ll > node = findSuccessor(newId);
		if (node.first.first == "" || node.second == -1 || node.first.second == -1 )
			break;
		{
			lock_guard<recursive_mutex> lock(dataMutex);
			fingerTable[next] = node;
		}
		next++;
	}

}

// Check if a key belongs to this node based on Chord protocol
bool NodeDht::keyBelongsToThisNode(ll keyId) {
	lock_guard<recursive_mutex> lock(dataMutex);
	ll predId = predecessor.second;
	
	// If predecessor is not set, key belongs to this node only in certain cases
	if (predId == -1) {
		// Node is alone in the ring or not fully stabilized
		return keyId == id || (keyId > id || keyId <= successor.second);
	}
	
	// Normal case: key belongs to this node if it's between predecessor and this node
	if (predId < id) {
		// Normal case: predId < id
		return keyId > predId && keyId <= id;
	} else {
		// Wrap-around case: predId > id
		return keyId > predId || keyId <= id;
	}
}

// Check if node should shutdown
bool NodeDht::isShutdownRequested() {
	lock_guard<recursive_mutex> lock(dataMutex);
	return shouldShutdown;
}

// Request node to shutdown gracefully
void NodeDht::requestShutdown() {
	lock_guard<recursive_mutex> lock(dataMutex);
	shouldShutdown = true;
}
