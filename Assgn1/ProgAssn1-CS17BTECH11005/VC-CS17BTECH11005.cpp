#include<bits/stdc++.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define BUFSIZE 1024
#define SERVERIP "127.0.0.1"
using namespace std;
map <pair<int,int>,int> clientPortMap,clientServerSocket;
map <pair<int,int>,int> port_idx;
map <pair<int,int>,int> sockfdMap;
std::default_random_engine eng;
ofstream output; 
int serverPortSeed,clientPortSeed,m,l1,n;
double alpha;
set <int> WaitingSet;
vector <int> serverSocketfds;
mutex waitingSetLock;
/**
 * Helper Class for get the formatted time in HH:MM:SS 
 * */
class Helper {
    public:
    static string get_formatted_time(time_t t1) // gives formatted time in HH::MM::SS
    {
        struct tm* t2=localtime(&t1);
        char buffer[20];
        sprintf(buffer,"%d : %d : %d",t2->tm_hour,t2->tm_min,t2->tm_sec);
        return buffer;
    }
    static int getRandomNumber(int a, int b) 
    {    
        int out = a + rand() % (b - a + 1);
        return out;
    }
};

class Node{

    int id;
    int inDeg;
    int outDeg;    
    int portNo;
    int serverSocket;
	int serverPort;
	int clientCounter = 1;
    vector<int> inDegreeVertices ,outDegreeVertices;
	thread* clientListenerThreads;
	thread* messageSenderThreads;
    int* clientSocketIds ;
    thread server; 
    vector <int> timeVector; 
    mutex timeVectorLock;
    int* sendFreq;
    int* recvFreq;
    int internalFreq = 0;
    mutex setLock;

    private:
        void initServerNode(){

            in_port_t servPort = serverPortSeed + id; // Local port

            // create socket for incoming connections           
            if ((serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
                perror("socket() failed");
                exit(-1);
            }

            // Set local parameters
            struct sockaddr_in servAddr;
            memset(&servAddr, 0, sizeof(servAddr));
            servAddr.sin_family = AF_INET;
            servAddr.sin_addr.s_addr = htons(INADDR_ANY);
            servAddr.sin_port = htons(servPort);

            // Bind to the local address
            if (bind(serverSocket, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
                perror("bind() failed");
                exit(-1);
            }
            // Listen to the client
            if (listen(serverSocket, inDeg) < 0) {
                perror("listen() failed");
                exit(-1);
            }

           
			// initialize clientSocket Id's 
			
			struct sockaddr_in clntAddr;
			socklen_t clntAddrLen = sizeof(clntAddr);


            // not necessarily connect in usual order so do propper maping of port to clientSocketid
           
            waitingSetLock.lock();
            // Add this id to the waiting set as the server is now in listening state
            WaitingSet.erase(id); 
            waitingSetLock.unlock();

            // Clients not necessarily connect in usual order so 
            // we need to do propper maping of clients port to clientSocketid
            for(int clientCounter=0;clientCounter<inDeg;clientCounter++){ 
                // clientSocketIds[clientCounter] will store the socket file discripter
                // for this connection
                clientSocketIds[clientCounter] = accept(serverSocket, (struct sockaddr *) &clntAddr, &clntAddrLen);
				if (clientSocketIds[clientCounter] < 0) {
					perror("accept() failed");
					exit(-1);
				}

                // now we need to which client connected 
                // port_idx map will store the socket file descriptor for the connection between
                // server id and clinet with port clntAddr.sin_port
                // While listening for a message we wil require this to get the corresponding
                // socket where the server can listen
                char clntIpAddr[INET_ADDRSTRLEN];
                if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr,clntIpAddr, sizeof(clntIpAddr)) != NULL) {
                    port_idx[{id,clntAddr.sin_port}] = clientSocketIds[clientCounter];
                } else {
                    puts("----\nUnable to get client IP Address");
                }              
			}   
        }
		void listenForMessage(int clientId){
            // buffer will store the message 
            char buffer[BUFSIZE];
            memset(buffer, 0, BUFSIZE); // reset the buffer
            ssize_t recvLen ;     
            int socketToListen;
            // clientPortMap will give the client's port for the connection between 
            // server id and client with id clientId
            int clientPortId = clientPortMap[{id,clientId}]; // which socket to listen
            // We need clientPort as it is unique for every connection with different server 
            
            // port_idx will give the socket file descriptor for connection between server id
            // and client with clientport clientPortId

            while((socketToListen = port_idx[{id,clientPortId}]) == 0 );

            while( recvLen =  recv(socketToListen, buffer, BUFSIZE - 1, 0) > 0){
                string message = string(buffer);
                vector <vector <int> > sendersVector = parseString(message);
                // As multive send calls can be handled by single recv we need to handle for 
                // multiple send calls. 
                // sendersVector is a vector of vector of pairs 
                // where pairs are the updated index which the server needs to update
                for(auto senderVector : sendersVector){
                   
                    timeVectorLock.lock();
                    for(int i=0;i<n;i++){
                        timeVector[i] = max(timeVector[i] , senderVector[i]);
                    }
                    time_t RecvTime=time(NULL);
                    string formatted_time=Helper::get_formatted_time(RecvTime);
                    string FinalString = "process"+to_string(id) + " receives message m"+to_string(clientId)+to_string(++(recvFreq[clientId])) +" from process"+to_string(clientId)+ " at "+ formatted_time +" , vc: "+ outputTimeVectorString()+"\n";
                    output<<FinalString;
                    timeVectorLock.unlock();
                    memset(buffer, 0, BUFSIZE);
                }
            }

        }
		void setUpConnectionPort(int serverPort , int serverId){
					//Creat a socket
			int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (sockfd < 0) {
				perror("socket() failed");
				exit(-1);
			}	

			
			// Set the server address
			struct sockaddr_in servAddr , myOwnAddr;
			memset(&servAddr, 0, sizeof(servAddr));           
			servAddr.sin_family = AF_INET;
			int err = inet_pton(AF_INET, SERVERIP, &servAddr.sin_addr.s_addr);
			if (err <= 0) {
				perror("inet_pton() failed");
				exit(-1);
			}
			servAddr.sin_port = htons(serverPort);

            // Also create sockaddr_in struct for the client thread whihc will connect to server
            // myOwnAddr will store the details of the client thread
            memset(&myOwnAddr, 0, sizeof(myOwnAddr));
            myOwnAddr.sin_family = AF_INET;
			int err2 = inet_pton(AF_INET, SERVERIP, &myOwnAddr.sin_addr.s_addr);
			if (err2 <= 0) {
				perror("inet_pton() failed");
				exit(-1);
			}

            // clients port clientPortSeed+id+100*serverId to uniquely define each port
			myOwnAddr.sin_port = htons(clientPortSeed+id+10*serverId);
            if (bind(sockfd, (struct sockaddr *) &myOwnAddr, sizeof(myOwnAddr)) < 0) {
                perror("bind() failed");
                exit(-1);
            }
			
			// Connect to server
			if (connect(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
				perror("connect() failed");
				exit(-1);
			}

            // while sending message from id to serverId this clients port was used
            // It will be used by server "serverId" to get the socket file descriptor where it will listen
            // for messages from this client "id"
            clientPortMap[{serverId,id}] = myOwnAddr.sin_port ; 
            // records clients socket , serverid and id will uniquely define the socket file descriptor
            // while sending message to server serverId, client id will use this socket.
            clientServerSocket[{serverId,id}] = sockfd;
        }

        void sendMessage(){
            std::exponential_distribution<double>exponential(1.0/l1); // to generate sleep times having exponential distribution
            for(int i=1;i<=m;i++){
                timeVectorLock.lock();
                timeVector[id-1] =  timeVector[id-1] + 1 ;                  
                timeVectorLock.unlock();
                int randomNumber = Helper::getRandomNumber(1,5);
                // Since alpha is 1.5 [1-3] will correspond to internal event and 
                // [4,5] will correspond to send event
                if(randomNumber < 3){ // internal event
                   
                    string message = outputTimeVectorString();
                    time_t InternalEventTime=time(NULL);
            		string formatted_time=Helper::get_formatted_time(InternalEventTime);                    
                    string FinalString = "process"+to_string(id) + " executes internal event e"+to_string(id)+to_string(++internalFreq) +" at "+ formatted_time +" , vc: "+ message+"\n";
                    output<<FinalString;
                }else{ // message send                    
                    int randomOutDegreeIndex = Helper::getRandomNumber(0,outDeg-1);
                    int reciever = outDegreeVertices[randomOutDegreeIndex] ;
                    int recieverSocket = clientServerSocket[{reciever,id}];
                    string message = outputTimeVectorString();
                    ssize_t sentLen = send(recieverSocket,message.c_str(), strlen(message.c_str()), 0);
                    time_t SendTime=time(NULL);
            		string formatted_time=Helper::get_formatted_time(SendTime);
                    
                    string FinalString = "process"+to_string(id) + " sends message m"+to_string(id)+to_string(++sendFreq[reciever]) +" to process"+to_string(reciever)+ " at "+ formatted_time +" , vc: "+ message+"\n";
                    output<<FinalString;
                }           
                usleep(exponential(eng)*1000); // sleep for random time
            }
		}
        // parses the string to returns vector of vector
        // as multilple send can be handled by single recv
        vector< vector<int> > parseString(string str){
            vector< vector<int> > senderTimeVector;
            for(int i=0;i<str.size();i++){
                if(str[i]=='['){
                    i++;
                    string temp;
                    vector <int> tempVec;
                    while(str[i] != ']'){
                        while(str[i] != ' ' && str[i]!= ']'){
                            temp+=str[i];
                            i++;
                        }
                        if(str[i]==' ') i++;
                        tempVec.push_back(stoi(temp));
                        temp.clear();
                    }
                    senderTimeVector.push_back(tempVec);
                }               
            }
            return senderTimeVector;
        }

         // outputs the time vector as a string
        string outputTimeVectorString(){
            string message;
            message += '[';
            for(auto k:timeVector){
                message+= to_string(k);
                message+= ' ';
            }
            
            message += ']';

            return message;
        }

        // creates listner thread total no is given by indegree of this node in the graph
        void initClientListnerThreads(){
            for(int i=0;i<inDeg;i++)
                {
					clientListenerThreads[i] = thread(&Node::listenForMessage,this,inDegreeVertices[i]);
                }
        }

        // We initialize connection ports for the message sender threads
        // total no given by outdegree of node in graph
		void initConnectionPorts(){
			for(int i=0;i<outDeg;i++){
    			messageSenderThreads[i] = thread(&Node::setUpConnectionPort,this , serverPortSeed+outDegreeVertices[i] , outDegreeVertices[i]);
            }
            for(int i=0;i<outDeg;i++){
                messageSenderThreads[i].join();
            }
		}

        // we initialize the server's port
        void init(){   
            server = thread(&Node::initServerNode,this);            
        }
      

    public:
    Node(vector<int> inDegreeVertices, vector<int> outDegreeVertices,int id){
        this->inDegreeVertices  = inDegreeVertices; // indegree vertices in graph
        this->outDegreeVertices = outDegreeVertices; // outdegree vertices in graph
        inDeg = inDegreeVertices.size();
        outDeg = outDegreeVertices.size();
        clientSocketIds = new int[inDeg + 1];  // client sockets
        clientListenerThreads = new thread[inDeg + 1]; // threads memory allocation
        messageSenderThreads  = new thread[outDeg + 1];
        // will keep track of how many times message has been sent to process i
        sendFreq = new int[n+1];

        // will keep track of how many times message has been recieved from process i
        recvFreq = new int[n+1];

        for(int i=1;i<=n;i++){
            sendFreq[i] = recvFreq[i] = 0;
        }
        this->id = id;  
        timeVector.resize(n);   	        
        init();
    }
    void startListenerThreads(){ // server setup completed create listner threads
        server.join();
        initClientListnerThreads();
    }
    void setUpConnectionPorts(){ // setup conneciton ports for different recievers (outdegree)
        initConnectionPorts();
    }
    void sendMessageThreads(){ // start message sender thread
        sendMessage();
    }
    ~Node(){
        for(int i=0;i<inDeg;i++)
            clientListenerThreads[i].join();

        for(int i=0;i<outDeg;i++)
            messageSenderThreads[i].join();
    }
};

int main()
{

    ifstream input("inp-params.txt"); // take input from inp-params.txt
    output.open("Log.txt");
    string str2;        
    input>>n>>l1>>alpha>>m;
    
    input.ignore();
    vector <int> inverseAdjacencyList[n+5]; // to keep track of nodes that will send message to me
    vector <int> adjacencyList[n+5]; // to keep track of nodes whom I will send messages
    Node* nodes[n+5]; // Create n nodes

    // Input Handling
    for(int i=1;i<=n;i++){
        string str;   
        getline(input, str);
        std::istringstream line(str);
        int flag = 0;           
        while(getline(line, str, ' ')) {
            int temp = stoi(str);
            if(flag){
                adjacencyList[i].push_back(temp);
                inverseAdjacencyList[temp].push_back(i);
            }
            flag++;
        } 
        WaitingSet.insert(i);        
    }

    // Create random serverPortSeed and ClientPortSeed 
    // Using this as base seed client and server threads will compute their port numbers
    srand(time(NULL));
    serverPortSeed = Helper::getRandomNumber(10000,20000);
    clientPortSeed = Helper::getRandomNumber(20000,30000);
    int totalIndeg = 0;
    for(int i=1;i<=n;i++){      
        nodes[i] = new Node(inverseAdjacencyList[i],adjacencyList[i],i); // create a node 
    }
   
    while(!WaitingSet.empty()); // Wait till the constructor has finished and server nodes are setup

    for(int i=1;i<=n;i++){
        nodes[i]->setUpConnectionPorts();
    }
    for(int i=1;i<=n;i++){
        nodes[i]->startListenerThreads();
    }  
    for(int i=1;i<=n;i++){
        nodes[i]->sendMessageThreads();
    }

    for(auto k:serverSocketfds) // close server sockets , client sockets are already closed
        close(k);
       
}