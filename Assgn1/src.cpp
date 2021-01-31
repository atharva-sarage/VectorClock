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
int serverPortSeed,clientPortSeed,m,l1,alpha,n;
set <int> WaitingSet;
set <pair<pair<int,int>,int>> pendingMessageSet;

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

// make a new class to replace the global variables
// class GlobaNodeManager{

// }
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
           
            WaitingSet.erase(id);
            for(int clientCounter=0;clientCounter<inDeg;clientCounter++){ 
                //cout<<"Server "<<id<<": Waiting\n"<<" on port "<<servPort<<endl;
                clientSocketIds[clientCounter] = accept(serverSocket, (struct sockaddr *) &clntAddr, &clntAddrLen);
				if (clientSocketIds[clientCounter] < 0) {
					perror("accept() failed");
					exit(-1);
				}

                char clntIpAddr[INET_ADDRSTRLEN];
                if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr,clntIpAddr, sizeof(clntIpAddr)) != NULL) {
                    printf("----\nHandling client %s %d for %d\n",
                    clntIpAddr, clntAddr.sin_port,id);
                    port_idx[{id,clntAddr.sin_port}] = clientSocketIds[clientCounter];
                } else {
                    puts("----\nUnable to get client IP Address");
                }              
                cout<<"Client Added"<<" "<<port_idx[{id,clntAddr.sin_port}]<<" "<<id<<" "<<clntAddr.sin_port<<endl;                
			}   
        }
		void listenForMessage(int clientId){
            char buffer[BUFSIZE];
            memset(buffer, 0, BUFSIZE);
            ssize_t recvLen ;     
            int socketToListen;
            int clientPortId = clientPortMap[{id,clientId}]; // which socket to listen
            while((socketToListen = port_idx[{id,clientPortId}]) == 0 );
            cout<<"Listening for  message"<<" "<<socketToListen<<"\n";

            while( recvLen =  recv(socketToListen, buffer, BUFSIZE - 1, 0) > 0){
                string message = string(buffer);
                cout<<message<<" %%%% "<<socketToListen<<endl;
                vector <vector <int> > sendersVector = parseString(message);
                for(auto senderVector : sendersVector){
                    for(auto k1:senderVector){
                        cout<<k1<<" * ";
                    }
                    cout<<"###"<<endl;
                    timeVectorLock.lock();
                    for(int i=0;i<n;i++){
                        timeVector[i] = max(timeVector[i] , senderVector[i]);
                    }
                    time_t RecvTime=time(NULL);
                    string formatted_time=Helper::get_formatted_time(RecvTime);
                    string FinalString = "process"+to_string(id) + " receives message m"+to_string(clientId)+to_string(++(recvFreq[clientId])) +" from process"+to_string(clientId)+ " at "+ formatted_time +" , vc: "+ outputTimeVectorString()+" incomming string "+ message +"\n";
                    output<<FinalString;
                    pendingMessageSet.erase({{id,clientId},recvFreq[clientId]});
                    cout<<"Recieved Message Server id::" <<id<<" vc: "<<outputTimeVectorString()<<endl;                

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

            memset(&myOwnAddr, 0, sizeof(myOwnAddr));
            myOwnAddr.sin_family = AF_INET;
			int err2 = inet_pton(AF_INET, SERVERIP, &myOwnAddr.sin_addr.s_addr);
			if (err2 <= 0) {
				perror("inet_pton() failed");
				exit(-1);
			}
			myOwnAddr.sin_port = htons(clientPortSeed+id+10*serverId);
            if (bind(sockfd, (struct sockaddr *) &myOwnAddr, sizeof(myOwnAddr)) < 0) {
                perror("bind() failed");
                exit(-1);
            }
			
            //cout<<id<<" to  "<<serverId<<" "<<myOwnAddr.sin_port<<" "<<serverPort<<endl;

			// Connect to server
			if (connect(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
                cout<<id<<"-----\n";
				perror("connect() failed");
				exit(-1);
			}

            // while sending message from id to serverId this clients port was used
            clientPortMap[{serverId,id}] = myOwnAddr.sin_port ; 
            clientServerSocket[{serverId,id}] = sockfd;
            cout<<id<<" "<<serverId<<" "<<sockfd<<" clientPortMap"<<endl;
			// after delay keep sending messages
        }

        void sendMessage(){
            std::exponential_distribution<double>exponential(1.0/l1); // to generate sleep times having exponential distribution
            for(int i=1;i<=m;i++){
                int randomNumber = Helper::getRandomNumber(1,5);
                if(randomNumber < 3){ // internal event
                    timeVectorLock.lock();
                    timeVector[id-1] =  timeVector[id-1] + 1 ;                  
                    timeVectorLock.unlock();
                    cout<<"Internal Event "<< id<<" "<<outputTimeVectorString()<<endl;
                    string message = compressTimeVector();
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
                    // log event to file
                    //process3 sends message m31 to process2 at 10:02, vc: [0 0 1 0]
                    time_t SendTime=time(NULL);
            		string formatted_time=Helper::get_formatted_time(SendTime);
                    
                    string FinalString = "process"+to_string(id) + " sends message m"+to_string(id)+to_string(++sendFreq[reciever]) +" to process"+to_string(reciever)+ " at "+ formatted_time +" , vc: "+ message+"\n";
                    output<<FinalString;
                    pendingMessageSet.insert({{reciever,id},sendFreq[reciever]});
                    cout<<id<<" "<<" Sending message to "<< reciever<<" "<<message<<"--- "<<recieverSocket<<endl;
                }           
                usleep(exponential(eng)*1000);
            }
		}
        string compressTimeVector(){
            string message;
            for(auto k:timeVector){
                message += to_string(k);
                message += "*";
            }
            return message;
        }
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
        void initClientListnerThreads(){
            for(int i=0;i<inDeg;i++)
                {
					clientListenerThreads[i] = thread(&Node::listenForMessage,this,inDegreeVertices[i]);
                }
        }
		void initConnectionPorts(){
			for(int i=0;i<outDeg;i++){
    			messageSenderThreads[i] = thread(&Node::setUpConnectionPort,this , serverPortSeed+outDegreeVertices[i] , outDegreeVertices[i]);
            }
            for(int i=0;i<outDeg;i++){
                messageSenderThreads[i].join();
            }
		}
        void init(){   
            server = thread(&Node::initServerNode,this);            
        }
      

    public:
    Node(vector<int> inDegreeVertices, vector<int> outDegreeVertices,int id){
        this->inDegreeVertices  = inDegreeVertices;
        this->outDegreeVertices = outDegreeVertices;
        inDeg = inDegreeVertices.size();
        outDeg = outDegreeVertices.size();
        clientSocketIds = new int[inDeg + 1];
        clientListenerThreads = new thread[inDeg + 1];
        messageSenderThreads  = new thread[outDeg + 1];
        sendFreq = new int[n+1];
        recvFreq = new int[n+1];
        memset(sendFreq,0,sizeof(sendFreq));
        memset(recvFreq,0,sizeof(recvFreq));
        this->id = id;  
        timeVector.resize(n);   	        
        init();
    }
    void startListenerThreads(){
        server.join();
        initClientListnerThreads();
    }
    void setUpConnectionPorts(){
        initConnectionPorts();
    }
    void sendMessageThreads(){
        sendMessage();
    }
    ~Node(){
        //server.join();
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
    vector <int> inverseAdjacencyList[n+5];
    vector <int> adjacencyList[n+5];
    Node* nodes[n+5];
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

    srand(time(NULL));
    serverPortSeed = Helper::getRandomNumber(10000,11000);
    clientPortSeed = Helper::getRandomNumber(11000 ,12000);

    for(int i=1;i<=n;i++){      
        nodes[i] = new Node(inverseAdjacencyList[i],adjacencyList[i],i);
    }
    
    while(!WaitingSet.empty());

    for(int i=1;i<=n;i++){
        nodes[i]->setUpConnectionPorts();
    }

    for(int i=1;i<=n;i++){
        nodes[i]->startListenerThreads();
    }  
    for(int i=1;i<=n;i++){
        nodes[i]->sendMessageThreads();
    }
    while(!pendingMessageSet.empty());
    
}