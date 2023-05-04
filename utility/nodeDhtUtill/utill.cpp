#include "headers.h"
#include "utill.h"
#define ll long long int
mutex mt;
// To split the input get from terminal
vector<string> utillFunctions::splitCommand(string command){
    vector<string> arguments;
    int pos = 0;
    do{
        pos = command.find(' ');
        string arg = command.substr(0,pos); 
        arguments.push_back(arg);
        command = command.substr(pos+1);
    }while(pos != -1);
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

    int pos = id.find(":");
    if(pos == -1) return false;

    for(int i=0;i<pos;i++)
        if( !(id[i] >= 48 && id[i] <= 57) )
            return false;
    return true;
}
//seprate key and value
pair<ll,string> utillFunctions::getKeyAndVal(string keyAndVal){

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
        perror("error");
        exit(-1);
    }

    keyHash += "k";

    char keyHashChar[40];
    strcpy(keyHashChar,keyHash.c_str());

    sendto(sock,keyHashChar,strlen(keyHashChar),0,(struct sockaddr *)&serverToConnectTo,l);

    char valChar[100];
    int len = recvfrom(sock,valChar,1024,0,(struct sockaddr *)&serverToConnectTo,&l);

    valChar[len] = '\0';

    string val = valChar;

    close(sock);

    return val;
}

// fillng server details that we want to connect
void utillFunctions::setServerDetails(struct sockaddr_in &server,string ip,int port){
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip.c_str());
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
void utillFunctions::sendKeyToNode(pair< pair<string,int> , ll > node,ll keyHash,string value){
    string ip = node.first.first;
    int port = node.first.second;

    struct sockaddr_in serverToConnectTo;
    socklen_t l = sizeof(serverToConnectTo);

    setServerDetails(serverToConnectTo,ip,port);

    int sock = socket(AF_INET,SOCK_DGRAM,0);

    if(sock < 0){
        perror("error");
        exit(-1);
    }

    string keyAndVal = combineIpAndPort(to_string(keyHash),value);

    char keyAndValChar[100];
    strcpy(keyAndValChar,keyAndVal.c_str());

    sendto(sock,keyAndValChar,strlen(keyAndValChar),0,(struct sockaddr *)&serverToConnectTo,l);

    close(sock);
}


//new joined node get details from successor
void utillFunctions::getKeysFromSuccessor(NodeDht &nodeInfo,string ip,int port){
    struct sockaddr_in serverToConnectTo;
    socklen_t l = sizeof(serverToConnectTo);

    setServerDetails(serverToConnectTo,ip,port);

    int sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock < 0){
        perror("error");
        exit(-1);
    }

    /* node sends msg "getKeys:id" to it's successor to get all keys which belongs to this node now */
    string id = to_string(nodeInfo.getId());


    string msg = "getKeys:" + id;

    char msgChar[40];
    strcpy(msgChar,msg.c_str());

    sendto(sock,msgChar,strlen(msgChar),0,(struct sockaddr *) &serverToConnectTo,l);

    char keysAndValuesChar[2000];
    int len = recvfrom(sock,keysAndValuesChar,2000,0,(struct sockaddr *) &serverToConnectTo,&l);

    keysAndValuesChar[len] = '\0';

    close(sock);

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

    char keysAndValuesChar[2000];
    strcpy(keysAndValuesChar,keysAndValues.c_str());

    sendto(newSock,keysAndValuesChar,strlen(keysAndValuesChar),0,(struct sockaddr *)&client,l);
}

//Sending value to node
void utillFunctions::sendValToNode(NodeDht nodeInfo,int newSock,struct sockaddr_in client,string nodeIdString){
    nodeIdString.pop_back();
    ll key = stoll(nodeIdString);
    string val = nodeInfo.getValue(key);

    socklen_t l = sizeof(client);

    char valChar[100];
    strcpy(valChar,val.c_str());

    sendto(newSock,valChar,strlen(valChar),0,(struct sockaddr *)&client,l);
}

//Send successor id of current node to the contacting node 
void utillFunctions::sendSuccessorId(NodeDht nodeInfo,int newSock,struct sockaddr_in client){

    pair< pair<string,int> , ll > succ = nodeInfo.getSuccessor();
    string succId = to_string(succ.second);
    char succIdChar[40];

    socklen_t l = sizeof(client);

    strcpy(succIdChar,succId.c_str());

    sendto(newSock,succIdChar,strlen(succIdChar),0,(struct sockaddr *)&client,l);

}

// To find successor of contacting node and send it's ip:port to it 
void utillFunctions::sendSuccessor(NodeDht nodeInfo,string nodeIdString,int newSock,struct sockaddr_in client){
    
    ll nodeId = stoll(nodeIdString);

    socklen_t l = sizeof(client);
    
    // find successor of the joining node 
    pair< pair<string,int> , ll > succNode;
    succNode = nodeInfo.findSuccessor(nodeId);

    // get Ip and port of successor as ip:port in char array to send 
    char ipAndPort[40];
    string succIp = succNode.first.first;
    string succPort = to_string(succNode.first.second);
    strcpy(ipAndPort,combineIpAndPort(succIp,succPort).c_str());

    //send ip and port info to the respective node 
    sendto(newSock, ipAndPort, strlen(ipAndPort), 0, (struct sockaddr*) &client, l);

}

//send ip:port of predecessor of current node to contacting node 
void utillFunctions::sendPredecessor(NodeDht nodeInfo,int newSock,struct sockaddr_in client){
    
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

        char ipAndPortChar[40];
        strcpy(ipAndPortChar,ipAndPort.c_str());

        sendto(newSock, ipAndPortChar, strlen(ipAndPortChar), 0, (struct sockaddr*) &client, l);

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
        perror("error");
        exit(-1);
    }

    setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timer,sizeof(struct timeval));

    if(sock < -1){
        cout<<"socket error";
        perror("error");
        exit(-1);
    }

    char msg[] = "finger";

    if (sendto(sock, msg, strlen(msg) , 0, (struct sockaddr*) &serverToConnectTo, l) == -1){
        cout<<"sending failed"<<sock<<endl;
        perror("error");
        exit(-1);
    }

    char succIdChar[40];

    int len = recvfrom(sock,succIdChar,1024,0,(struct sockaddr*) &serverToConnectTo, &l);

    close(sock);

    if(len < 0){
        return -1;
    }

    succIdChar[len] = '\0';

    return atoll(succIdChar);

}

void utillFunctions::setTimer(struct timeval &timer){
    timer.tv_sec = 0;
    timer.tv_usec = 100000;
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
        perror("error");
        exit(-1);
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


    char ipAndPortChar[40];
    strcpy(ipAndPortChar,msg.c_str());

    if (sendto(sock, ipAndPortChar, strlen(ipAndPortChar), 0, (struct sockaddr*) &serverToConnectTo, l) < 0){
        cout<<"error"<<sock<<endl;
        perror("error");
        exit(-1);
    }

    int len = recvfrom(sock, ipAndPortChar, 1024, 0, (struct sockaddr *) &serverToConnectTo, &l);
    close(sock);

    if(len < 0){
        pair< pair<string,int> , ll > node;
        node.first.first = "";
        node.first.second = -1;
        node.second = -1;
        return node;
    }

    ipAndPortChar[len] = '\0';

    string ipAndPort = ipAndPortChar;
    ll hash;
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
        perror("error");
        exit(-1);
    }

    setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timer,sizeof(struct timeval));

    char msg[] = "sendSuccList";

    sendto(sock,msg,strlen(msg),0,(struct sockaddr *)&serverToConnectTo,l);

    char succListChar[1000];
    int len = recvfrom(sock,succListChar,1000,0,(struct sockaddr *)&serverToConnectTo,&l);

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

    char successorListChar[1000];
    strcpy(successorListChar,successorList.c_str());

    sendto(sock,successorListChar,strlen(successorListChar),0,(struct sockaddr *)&client,l);

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
    struct sockaddr_in serverToConnectTo;
    socklen_t l = sizeof(serverToConnectTo);

    setServerDetails(serverToConnectTo,ip,port);

    //set timer for socket 
    struct timeval timer;
    setTimer(timer);


    int sock = socket(AF_INET,SOCK_DGRAM,0);

    if(sock < 0){
        perror("error");
        exit(-1);
    }

    //set timer on this socket 
    setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timer,sizeof(struct timeval));

    char msg[] = "alive";
    sendto(sock,msg,strlen(msg),0,(struct sockaddr *)&serverToConnectTo,l);

    char response[5];
    int len = recvfrom(sock,response,2,0,(struct sockaddr *)&serverToConnectTo,&l);

    close(sock);

    //node is still active 
    if(len >= 0){
        return true;
    }
    else
        return false;
}

