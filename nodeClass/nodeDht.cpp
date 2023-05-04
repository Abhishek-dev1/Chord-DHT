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
}
void NodeDht::setSuccessor(string ip, int port, ll hash) {
	successor.first.first = ip;
	successor.first.second = port;
	successor.second = hash;
}
void NodeDht::setStatus() {isInRing = true;}

void NodeDht::setId(ll nodeId) {id = nodeId;}

vector< pair< pair<string, int> , ll > > NodeDht::getFingerTable() {return fingerTable;}

ll NodeDht::getId() {
	return id;
}
pair< pair<string, int> , ll > NodeDht::getSuccessor() {return successor;}

pair< pair<string, int> , ll > NodeDht::getPredecessor() {return predecessor;}

vector< pair< pair<string, int> , ll > > NodeDht::getSuccessorList() {return successorList;}

bool NodeDht::getStatus() {return isInRing;}

string NodeDht::getValue(ll key) {
	if (dictionary.find(key) != dictionary.end()) {
		return dictionary[key];
	}
	else
		return "";
}
void NodeDht::setFingerTable(string ip, int port, ll hash) {
	for (int i = 1; i <= M; i++) {
		fingerTable[i] = make_pair(make_pair(ip, port), hash);
	}
}

void NodeDht::storeKey(ll key, string val) {dictionary[key] = val;}

void NodeDht::setSuccessorList(string ip, int port, ll hash) {
	for (int i = 1; i <= R; i++) {
		successorList[i] = make_pair(make_pair(ip, port), hash);
	}
}

void NodeDht::setPredecessor(string ip, int port, ll hash) {
	predecessor.first.first = ip;
	predecessor.first.second = port;
	predecessor.second = hash;
}





void NodeDht::printKeys() {
	map<ll, string>::iterator it;

	for (it = dictionary.begin(); it != dictionary.end() ; it++) {
		cout << it->first << " " << it->second << endl;
	}
}

void NodeDht::updateSuccessorList() {

	utillFunctions help;

	vector< pair<string, int> > list = help.getSuccessorListFromNode(successor.first.first, successor.first.second);

	if (list.size() != R)
		return;

	successorList[1] = successor;

	for (int i = 2; i <= R; i++) {
		successorList[i].first.first = list[i - 2].first;
		successorList[i].first.second = list[i - 2].second;
		successorList[i].second = help.getHash(list[i - 2].first + ":" + to_string(list[i - 2].second));
	}

}


//Send all keys of this node to it's successor after it leaves the ring
vector< pair<ll , string> > NodeDht::getAllKeysForSuccessor() {
	map<ll, string>::iterator it;
	vector< pair<ll , string> > res;

	for (it = dictionary.begin(); it != dictionary.end() ; it++) {
		res.push_back(make_pair(it->first , it->second));
		dictionary.erase(it);
	}

	return res;
}

vector< pair<ll , string> > NodeDht::getKeysForPredecessor(ll nodeId) {
	map<ll, string>::iterator it;

	vector< pair<ll , string> > res;
	for (it = dictionary.begin(); it != dictionary.end() ; it++) {
		ll keyId = it->first;

		// if predecessor's id is more than current node's id
		if (id < nodeId) {
			if (keyId > id && keyId <= nodeId) {
				res.push_back(make_pair(keyId , it->second));
				dictionary.erase(it);
			}
		}

		// if predecessor's id is less than current node's id
		else {
			if (keyId <= nodeId || keyId > id) {
				res.push_back(make_pair(keyId , it->second));
				dictionary.erase(it);
			}
		}
	}

	return res;
}

pair< pair<string, int> , ll > NodeDht::findSuccessor(ll nodeId) {

	pair < pair<string, int> , ll > self;
	self.first.first = sp.getIpAddress();
	self.first.second = sp.getPortNumber();
	self.second = id;

	if (nodeId > id && nodeId <= successor.second) {
		return successor;
	}

	else if (id == successor.second || nodeId == id) {
		return self;
	}

	else if (successor.second == predecessor.second) {
		if (successor.second >= id) {
			if (nodeId > successor.second || nodeId < id)
				return self;
		}
		else {
			if ((nodeId > id && nodeId > successor.second) || (nodeId < id && nodeId < successor.second))
				return successor;
			else
				return self;
		}
	}

	else {

		pair < pair<string, int> , ll > node = closestPrecedingNode(nodeId);
		if (node.second == id) {
			return successor;
		}
		else {

			/* connect to node which will now find the successor */
			struct sockaddr_in serverToConnectTo;
			socklen_t len = sizeof(serverToConnectTo);

			string ip;
			int port;

			//if this node couldn't find closest preciding node for given node id then now ask it's successor to do so
			if (node.second == -1) {
				node = successor;
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
				perror("error");
				exit(-1);
			}

			// send the node id to the other node
			char nodeIdChar[40];
			strcpy(nodeIdChar, to_string(nodeId).c_str());
			sendto(sockT, nodeIdChar, strlen(nodeIdChar), 0, (struct sockaddr*) &serverToConnectTo, len);

			// receive ip and port of node successor as ip:port
			char ipAndPort[40];

			int l = recvfrom(sockT, ipAndPort, 1024, 0, (struct sockaddr *) &serverToConnectTo, &len);

			close(sockT);

			if (l < 0) {
				pair < pair<string, int> , ll > node;
				node.first.first = "";
				node.second = -1;
				node.first.second = -1;
				return node;
			}

			ipAndPort[l] = '\0';

			//set ip,port and hash for this node and return it
			string key = ipAndPort;
			ll hash = help.getHash(ipAndPort);
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
	utillFunctions help;

	for (int i = M; i >= 1; i--) {
		if (fingerTable[i].first.first == "" || fingerTable[i].first.second == -1 || fingerTable[i].second == -1) {
			continue;
		}

		if (fingerTable[i].second > id && fingerTable[i].second < nodeId) {
			return fingerTable[i];
		}
		else {

			ll successorId = help.getSuccessorId(fingerTable[i].first.first, fingerTable[i].first.second);

			if (successorId == -1)
				continue;

			if (fingerTable[i].second > successorId) {
				if ((nodeId <= fingerTable[i].second && nodeId <= successorId) || (nodeId >= fingerTable[i].second && nodeId >= successorId)) {
					return fingerTable[i];
				}
			}
			else if (fingerTable[i].second < successorId && nodeId > fingerTable[i].second && nodeId < successorId) {
				return fingerTable[i];
			}

			pair< pair<string, int> , ll > predNode = help.getPredecessorNode(fingerTable[i].first.first, fingerTable[i].first.second, "", -1, false);
			ll predecessorId = predNode.second;

			if (predecessorId != -1 && fingerTable[i].second < predecessorId) {
				if ((nodeId <= fingerTable[i].second && nodeId <= predecessorId) || (nodeId >= fingerTable[i].second && nodeId >= predecessorId)) {
					return predNode;
				}
			}
			if (predecessorId != -1 && fingerTable[i].second > predecessorId && nodeId >= predecessorId && nodeId <= fingerTable[i].second) {
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

	//get predecessor of successor

	utillFunctions help;

	string ownIp = sp.getIpAddress();
	int ownPort = sp.getPortNumber();

	if (help.isNodeAlive(successor.first.first, successor.first.second) == false)
		return;

	//get predecessor of successor
	pair< pair<string, int> , ll > predNode = help.getPredecessorNode(successor.first.first, successor.first.second, ownIp, ownPort, true);

	ll predecessorHash = predNode.second;

	if (predecessorHash == -1 || predecessor.second == -1)
		return;

	if (predecessorHash > id || (predecessorHash > id && predecessorHash < successor.second) || (predecessorHash < id && predecessorHash < successor.second)) {
		successor = predNode;
	}


}

//check if current node predecessor is still alive
void NodeDht::checkPredecessor() {
	if (predecessor.second == -1)
		return;

	utillFunctions help;
	string ip = predecessor.first.first;
	int port = predecessor.first.second;

	if (help.isNodeAlive(ip, port) == false) {
		// if node has same successor and predecessor then set node as it's successor itself
		if (predecessor.second == successor.second) {
			successor.first.first = sp.getIpAddress();
			successor.first.second = sp.getPortNumber();
			successor.second = id;
			setSuccessorList(successor.first.first, successor.first.second, id);
		}
		predecessor.first.first = "";
		predecessor.first.second = -1;
		predecessor.second = -1;
	}

}

// check if current node's successor is still alive
void NodeDht::checkSuccessor() {
	if (successor.second == id)
		return;

	utillFunctions help;
	string ip = successor.first.first;
	int port = successor.first.second;

	if (help.isNodeAlive(ip, port) == 0) {
		successor = successorList[2];
		updateSuccessorList();
	}

}

void NodeDht::notify(pair< pair<string, int> , ll > node) {

	// To get id of node and predecessor
	ll predecessorHash = predecessor.second;
	ll nodeHash = node.second;

	predecessor = node;

	// if node successor is node itself then set its successor to this node
	if (successor.second == id) {
		successor = node;
	}
}

void NodeDht::fixFingers() {

	utillFunctions help;
	int next = 1;
	ll mod = pow(2, M);

	while (next <= M) {
		if (help.isNodeAlive(successor.first.first, successor.first.second) == false)
			return;

		ll newId = id + pow(2, next - 1);
		newId = newId % mod;
		pair< pair<string, int> , ll > node = findSuccessor(newId);
		if (node.first.first == "" || node.second == -1 || node.first.second == -1 )
			break;
		fingerTable[next] = node;
		next++;
	}

}
