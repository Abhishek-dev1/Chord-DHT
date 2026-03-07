#include "headers.h"
#include "utill.h"
#include <netdb.h>
#define ll long long int
mutex mt;
// To split the input get from terminal
vector<string> utillFunctions::splitCommand(string command){
    vector<string> arguments;
    string arg = "";
    bool inQuotes = false;
    
    for(size_t i = 0; i < command.length(); i++){
        char ch = command[i];
        
        if(ch == '"'){
            inQuotes = !inQuotes;
        }
        else if(ch == ' ' && !inQuotes){
            if(!arg.empty()){
                arguments.push_back(arg);
                arg = "";
            }
        }
        else{
            arg += ch;
        }
    }
    
    if(!arg.empty()){
        arguments.push_back(arg);
    }
    
    return arguments;
}

// Hash genrator for a given key
ll utillFunctions::getHash(string key){
    unsigned char buff[41];
    char finalHash[41];
    string keyHash = "";
    ll mod = pow(2,M);

    //Converting the string to char array as input is char array for SHA-1 Function
    int i;
    unsigned char unsigned_key[key.length()+1];
    for(i=0;i<key.length();i++){
        unsigned_key[i] = key[i];
    }
    unsigned_key[i] = '\0';


    SHA1(unsigned_key,sizeof(unsigned_key),buff);
    for (i = 0; i < M/8; i++) {
        sprintf(finalHash,"%d",buff[i]);
        keyHash += finalHash;
    }

    ll hash = stoll(keyHash) % mod;
    return hash;
}

//Extracting ip and port from the key
pair<string,int> utillFunctions::getIpAndPort(string key){

    int pos = key.find(':');
    string ip = key.substr(0,pos);
    string port = key.substr(pos+1);

    pair<string,int> ipAndPortPair;
    ipAndPortPair.first = ip;
    ipAndPortPair.second = atoi(port.c_str());

    return ipAndPortPair;
}

/* will decide if id is in form of key:value or not */
bool utillFunctions::isKeyValue(string id){
    if(id.rfind("kvh:", 0) == 0){
        return true;
    }

    int pos = id.find(":");
    if(pos == -1) return false;

    for(int i=0;i<pos;i++)
        if( !(id[i] >= 48 && id[i] <= 57) )
            return false;
    return true;
}
//seprate key and value
pair<ll,string> utillFunctions::getKeyAndVal(string keyAndVal){
    if(keyAndVal.rfind("kvh:", 0) == 0){
        int first = keyAndVal.find(':', 4);
        int second = keyAndVal.find(':', first + 1);
        if(first == -1 || second == -1){
            return {-1, ""};
        }

        string key = keyAndVal.substr(first + 1, second - first - 1);
        string val = keyAndVal.substr(second + 1);
        return {stoll(key), val};
    }

    int pos = keyAndVal.find(':');
    string key = keyAndVal.substr(0,pos);
    string val = keyAndVal.substr(pos+1);

    pair<ll,string> keyAndValPair;
    keyAndValPair.first = stoll(key);
    keyAndValPair.second = val;

    return keyAndValPair;
}


//fetch value of a particular key from a node
string utillFunctions::getKeyFromNode(pair< pair<string,int> , ll > node,string keyHash){
    string ip = node.first.first;
    int port = node.first.second;

    struct sockaddr_in serverToConnectTo;
    socklen_t l = sizeof(serverToConnectTo);

    setServerDetails(serverToConnectTo,ip,port);

    int sock = socket(AF_INET,SOCK_DGRAM,0);

    if(sock < 0){
        perror("error: Socket creation failed in getKeyFromNode");
        return "";
    }

    keyHash += "k";

    sendto(sock,keyHash.c_str(),keyHash.length(),0,(struct sockaddr *)&serverToConnectTo,l);

    char valChar[1024];
    int len = recvfrom(sock,valChar,sizeof(valChar)-1,0,(struct sockaddr *)&serverToConnectTo,&l);
    if(len < 0){
        close(sock);
        return "";
    }

    valChar[len] = '\0';

    string val = valChar;

    close(sock);

    return val;
}

// fillng server details that we want to connect
void utillFunctions::setServerDetails(struct sockaddr_in &server,string ip,int port){
    server.sin_family = AF_INET;

    struct in_addr addr;
    /* try dotted-decimal first */
    if(inet_aton(ip.c_str(), &addr) != 0){
        server.sin_addr = addr;
    }
    else{
        /* fallback to DNS lookup for names like 'localhost' */
        struct hostent *he = gethostbyname(ip.c_str());
        if(he == NULL){
            /* if resolution fails, default to loopback */
            server.sin_addr.s_addr = inet_addr("127.0.0.1");
        }
        else{
            server.sin_addr = *(struct in_addr*)he->h_addr;
        }
    }

    server.sin_port = htons(port);
}

//To Combine ip and port = ip:port
string utillFunctions::combineIpAndPort(string ip,string port){
    string ipAndPort = "";
    int i=0;

    for(i=0;i<ip.size();i++){
        ipAndPort += ip[i];
    }

    ipAndPort += ':';

    for(i=0;i<port.size();i++){
        ipAndPort += port[i];
    }

    return ipAndPort;
}
//Send key to node who requested for it
void utillFunctions::sendKeyToNode(pair< pair<string,int> , ll > node,ll keyHash,string value,int ttl){
    string ip = node.first.first;
    int port = node.first.second;

    struct sockaddr_in serverToConnectTo;
    socklen_t l = sizeof(serverToConnectTo);

    setServerDetails(serverToConnectTo,ip,port);

    int sock = socket(AF_INET,SOCK_DGRAM,0);

    if(sock < 0){
        perror("error: Socket creation failed in sendKeyToNode");
        return;
    }

    if(ttl < 0) ttl = 0;
    string keyAndVal = "kvh:" + to_string(ttl) + ":" + to_string(keyHash) + ":" + value;

    sendto(sock,keyAndVal.c_str(),keyAndVal.length(),0,(struct sockaddr *)&serverToConnectTo,l);

    close(sock);
}


//new joined node get details from successor
void utillFunctions::getKeysFromSuccessor(NodeDht &nodeInfo,string ip,int port){
    struct sockaddr_in serverToConnectTo;
    socklen_t l = sizeof(serverToConnectTo);

    setServerDetails(serverToConnectTo,ip,port);

    int sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock < 0){
        perror("Error: Socket creation failed in getKeysFromSuccessor");
        return;
    }

    /* node sends msg "getKeys:id" to it's successor to get all keys which belongs to this node now */
    string id = to_string(nodeInfo.getId());

    string msg = "getKeys:" + id;

    sendto(sock,msg.c_str(),msg.length(),0,(struct sockaddr *) &serverToConnectTo,l);

    char keysAndValuesChar[2001];
    int len = recvfrom(sock,keysAndValuesChar,sizeof(keysAndValuesChar)-1,0,(struct sockaddr *) &serverToConnectTo,&l);

    close(sock);

    if(len < 0){
        return;
    }

    keysAndValuesChar[len] = '\0';

    string keysAndValues = keysAndValuesChar;

    vector< pair<ll,string> > keysAndValuesVector = seperateKeysAndValues(keysAndValues);

    for(int i=0;i<keysAndValuesVector.size();i++){
        nodeInfo.storeKey(keysAndValuesVector[i].first , keysAndValuesVector[i].second);
    }

}

// Separating and storing values in vector k1:v1,k2:v2.....
vector< pair<ll,string> > utillFunctions::seperateKeysAndValues(string keysAndValues){
    int size = keysAndValues.size();
    int i = 0;
    vector< pair<ll,string> > res;

    while(i < size){
        string key = "";
        while(i < size && keysAndValues[i] != ':'){
            key += keysAndValues[i];
            i++;
        }
        i++;

        string val = "";
        while(i < size && keysAndValues[i] != ';'){
            val += keysAndValues[i];
            i++;
        }
        i++;

        res.push_back({stoll(key),val});
    }

    return res;
}

// => Separating and storing values in vector ip1:port1,ip2:port2.....
vector< pair<string,int> > utillFunctions::seperateSuccessorList(string succList){
    int size = succList.size();
    int i = 0;
    vector< pair<string,int> > res;

    while(i < size){
        string ip = "";
        while(i < size && succList[i] != ':'){
            ip += succList[i];
            i++;
        }
        i++;

        string port = "";
        while(i < size && succList[i] != ';'){
            port += succList[i];
            i++;
        }
        i++;

        res.push_back({ip,stoi(port)});
    }

    return res;
}



// Get all keys from predecessor leaving the ring 
void utillFunctions::storeAllKeys(NodeDht &nodeInfo,string keysAndValues){
    int pos = keysAndValues.find("storeKeys");

    vector< pair<ll,string> > res = seperateKeysAndValues(keysAndValues.substr(0,pos));

    for(int i=0;i<res.size();i++){
        nodeInfo.storeKey(res[i].first,res[i].second);
    }
}

// Send all the keys to the newly connected node which now belong to it
void utillFunctions::sendNeccessaryKeys(NodeDht &nodeInfo,int newSock,struct sockaddr_in client,string nodeIdString){
    socklen_t l = sizeof(client);

    int pos = nodeIdString.find(':');

    ll nodeId = stoll(nodeIdString.substr(pos+1));

    vector< pair<ll , string> > keysAndValuesVector = nodeInfo.getKeysForPredecessor(nodeId);

    string keysAndValues = "";

    /* will arrange all keys and val in form of key1:val1;key2:val2; */
    for(int i=0;i<keysAndValuesVector.size();i++){
        keysAndValues += to_string(keysAndValuesVector[i].first) + ":" + keysAndValuesVector[i].second;
        keysAndValues += ";";
    }

    sendto(newSock,keysAndValues.c_str(),keysAndValues.length(),0,(struct sockaddr *)&client,l);
}

//Sending value to node
void utillFunctions::sendValToNode(NodeDht &nodeInfo,int newSock,struct sockaddr_in client,string nodeIdString){
    nodeIdString.pop_back();
    ll key = stoll(nodeIdString);
    string val = nodeInfo.getValue(key);

    socklen_t l = sizeof(client);

    sendto(newSock,val.c_str(),val.length(),0,(struct sockaddr *)&client,l);
}

//Send successor id of current node to the contacting node 
void utillFunctions::sendSuccessorId(NodeDht &nodeInfo,int newSock,struct sockaddr_in client){

    pair< pair<string,int> , ll > succ = nodeInfo.getSuccessor();
    string succId = to_string(succ.second);

    socklen_t l = sizeof(client);

    sendto(newSock,succId.c_str(),succId.length(),0,(struct sockaddr *)&client,l);

}

// To find successor of contacting node and send it's ip:port to it 
void utillFunctions::sendSuccessor(NodeDht &nodeInfo,string nodeIdString,int newSock,struct sockaddr_in client){
    
    ll nodeId = stoll(nodeIdString);

    socklen_t l = sizeof(client);
    
    // find successor of the joining node 
    pair< pair<string,int> , ll > succNode;
    succNode = nodeInfo.findSuccessor(nodeId);

    // get Ip and port of successor as ip:port string 
    string succIp = succNode.first.first;
    string succPort = to_string(succNode.first.second);
    string ipAndPort = combineIpAndPort(succIp,succPort);

    //send ip and port info to the respective node 
    sendto(newSock, ipAndPort.c_str(), ipAndPort.length(), 0, (struct sockaddr*) &client, l);

}

//send ip:port of predecessor of current node to contacting node 
void utillFunctions::sendPredecessor(NodeDht &nodeInfo,int newSock,struct sockaddr_in client){
    
    pair< pair<string,int> , ll > predecessor = nodeInfo.getPredecessor();
    
    string ip = predecessor.first.first;
    string port = to_string(predecessor.first.second);

    socklen_t l = sizeof(client);

    /* if predecessor is nil */
    if(ip == ""){
        sendto(newSock, "", 0, 0, (struct sockaddr*) &client, l);       
    }

    else{
        string ipAndPort = combineIpAndPort(ip,port);
        sendto(newSock, ipAndPort.c_str(), ipAndPort.length(), 0, (struct sockaddr*) &client, l);

    }
}

// get successor id of the node having ip address as ip and port num as port 
ll utillFunctions::getSuccessorId(string ip,int port){
    
    struct sockaddr_in serverToConnectTo;
    socklen_t l = sizeof(serverToConnectTo);

    setServerDetails(serverToConnectTo,ip,port);

    //set timer for socket 
    struct timeval timer;
    setTimer(timer);

    int sock = socket(AF_INET,SOCK_DGRAM,0);

    if(sock < 0){
        perror("Error: Socket creation failed in getSuccessorId");
        return -1;
    }

    setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timer,sizeof(struct timeval));

    char msg[] = "finger";

    if (sendto(sock, msg, strlen(msg) , 0, (struct sockaddr*) &serverToConnectTo, l) == -1){
        perror("Error: sendto failed in getSuccessorId");
        close(sock);
        return -1;
    }

    char succIdChar[1024];

    int len = recvfrom(sock,succIdChar,sizeof(succIdChar)-1,0,(struct sockaddr*) &serverToConnectTo, &l);

    close(sock);

    if(len < 0){
        return -1;
    }

    succIdChar[len] = '\0';

    return atoll(succIdChar);

}

void utillFunctions::setTimer(struct timeval &timer){
    timer.tv_sec = 1;
    timer.tv_usec = 0;
}

//get predecessor node (ip:port) of the node having ip and port 
pair< pair<string,int> , ll > utillFunctions::getPredecessorNode(string ip,int port,string ipClient,int portClient,bool forStabilize){

    struct sockaddr_in serverToConnectTo;
    socklen_t l = sizeof(serverToConnectTo);

    setServerDetails(serverToConnectTo,ip,port);

    //set timer for socket 
    struct timeval timer;
    setTimer(timer);

    int sock = socket(AF_INET,SOCK_DGRAM,0);

    if(sock < 0){
        perror("error: Socket creation failed in getPredecessorNode");
        pair< pair<string,int> , ll > node;
        node.first.first = "";
        node.first.second = -1;
        node.second = -1;
        return node;
    }

    setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timer,sizeof(struct timeval));

    string msg = "";
    //p1 = stabilize so notify node as well 
    //p2 = that just send predecessor of node ip:port, do not call notify 
    
    if(forStabilize == 1){
        msg = combineIpAndPort(ipClient,to_string(portClient));
        msg += "p1";
    }

    else msg = "p2";


    if (sendto(sock, msg.c_str(), msg.length(), 0, (struct sockaddr*) &serverToConnectTo, l) < 0){
        perror("Error: sendto failed in getPredecessorNode");
        close(sock);
        pair< pair<string,int> , ll > node;
        node.first.first = "";
        node.first.second = -1;
        node.second = -1;
        return node;
    }

    char recvBuf[1024];
    int len = recvfrom(sock, recvBuf, sizeof(recvBuf)-1, 0, (struct sockaddr *) &serverToConnectTo, &l);
    close(sock);

    if(len < 0){
        pair< pair<string,int> , ll > node;
        node.first.first = "";
        node.first.second = -1;
        node.second = -1;
        return node;
    }

    recvBuf[len] = '\0';
    string ipAndPort = recvBuf;
    pair<string,int> ipAndPortPair;

    pair< pair<string,int> , ll > node;

    if(ipAndPort == ""){
        node.first.first = "";
        node.first.second = -1;
        node.second = -1;
    }

    else{
        ipAndPortPair = getIpAndPort(ipAndPort);
        node.first.first = ipAndPortPair.first;
        node.first.second = ipAndPortPair.second;
        node.second = getHash(ipAndPort);
    }

    return node;
}

//Get successor list from node having ip and port 
vector< pair<string,int> > utillFunctions::getSuccessorListFromNode(string ip,int port){

    struct sockaddr_in serverToConnectTo;
    socklen_t l = sizeof(serverToConnectTo);

    setServerDetails(serverToConnectTo,ip,port);

    //set timer for socket 
    struct timeval timer;
    setTimer(timer);
    

    int sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock < 0){
        perror("Error: Socket creation failed in getSuccessorListFromNode");
        vector< pair<string,int> > list;
        return list;
    }

    setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timer,sizeof(struct timeval));

    char msg[] = "sendSuccList";

    sendto(sock,msg,strlen(msg),0,(struct sockaddr *)&serverToConnectTo,l);

    char succListChar[1001];
    int len = recvfrom(sock,succListChar,sizeof(succListChar)-1,0,(struct sockaddr *)&serverToConnectTo,&l);

    close(sock);


    if(len < 0){
        vector< pair<string,int> > list;
        return list;
    }

    succListChar[len] = '\0';

    string succList = succListChar;

    vector< pair<string,int> > list = seperateSuccessorList(succList);

    return list;

}

//Send node's successor list to the contacting node 
void utillFunctions::sendSuccessorList(NodeDht &nodeInfo,int sock,struct sockaddr_in client){
    socklen_t l = sizeof(client);

    vector< pair< pair<string,int> , ll > > list = nodeInfo.getSuccessorList();

    string successorList = splitSuccessorList(list);

    sendto(sock,successorList.c_str(),successorList.length(),0,(struct sockaddr *)&client,l);

}

//Combine successor list in form of ip1:port1,ip2:port2..
string utillFunctions::splitSuccessorList(vector< pair< pair<string,int> , ll > > list){
    string res = "";

    for(int i=1;i<=R;i++){

        res = res + list[i].first.first + ":" + to_string(list[i].first.second) + ";";
    }

    return res;
}

//Send acknowledgement to contacting node that this node is still alive
void utillFunctions::sendAcknowledgement(int newSock,struct sockaddr_in client){
    socklen_t l = sizeof(client);
    sendto(newSock,"1",1,0,(struct sockaddr*)&client,l);
}

//Check if node having ip and port is still alive or not 
bool utillFunctions::isNodeAlive(string ip,int port){
    for(int attempt = 0; attempt < 3; attempt++){
        struct sockaddr_in serverToConnectTo;
        socklen_t l = sizeof(serverToConnectTo);

        setServerDetails(serverToConnectTo,ip,port);

        //set timer for socket 
        struct timeval timer;
        setTimer(timer);

        int sock = socket(AF_INET,SOCK_DGRAM,0);

        if(sock < 0){
            perror("Error: Socket creation failed in isNodeAlive");
            return false;
        }

        //set timer on this socket 
        setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timer,sizeof(struct timeval));

        char msg[] = "alive";
        sendto(sock,msg,strlen(msg),0,(struct sockaddr *)&serverToConnectTo,l);

        char response[5];
        int len = recvfrom(sock,response,2,0,(struct sockaddr *)&serverToConnectTo,&l);
        close(sock);

        if(len >= 0){
            return true;
        }
    }

    return false;
}
