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
map <pair<int,int>,int> clientPortMap;
map <pair<int,int>,int> port_idx;
int serverPortSeed,clientPortSeed,m,l1,alpha;
set <int> WaitingSet;
int random(int a, int b) {    
    int out = a + rand() % (b - a + 1);
    return out;
}
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
                cout<<"Server "<<id<<": Waiting\n"<<" on port "<<servPort<<endl;
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
            cout<<"***"<<id<<" "<<clientId<<" "<<clientPortId<<" "<<port_idx[{id,clientPortId}]<<endl;
            while((socketToListen = port_idx[{id,clientPortId}]) == 0 );
            cout<<"Listening for  message"<<" "<<socketToListen<<"\n";

            while( recvLen =  recv(socketToListen, buffer, BUFSIZE - 1, 0) > 0){
                cout<<"Recieved Message"<<" "<<buffer<<endl;                
			    //fputs(buffer, stdout);
            }

        }
		void sendMessage(int serverPort , int serverId){
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
			
            cout<<id<<" to  "<<serverId<<" "<<myOwnAddr.sin_port<<" "<<serverPort<<endl;

			// Connect to server
			if (connect(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
                cout<<id<<"-----\n";
				perror("connect() failed");
				exit(-1);
			}

            // while sending message from id to serverId this clients port was used
            clientPortMap[{serverId,id}] = myOwnAddr.sin_port ; 
            cout<<serverId<<" "<<id<<" "<<myOwnAddr.sin_port<<" clientPortMap"<<endl;
			// after delay keep sending messages

			// while(1){
			// 	ssize_t sentLen = send(sockfd, "sendString", 1, 0);
			// }
            ssize_t sentLen = send(sockfd, "finally", strlen("finally"), 0);
            cout<<"Sending message "<< sockfd<<endl;

            sleep(1);

            ssize_t sentLen2 = send(sockfd, "finally2", strlen("finally2"), 0);
            cout<<"Sending message "<< sockfd<<endl;

		}
        void initClientListnerThreads(){
            for(int i=0;i<inDeg;i++)
                {
                    cout<<"$$$\n";
					clientListenerThreads[i] = thread(&Node::listenForMessage,this,inDegreeVertices[i]);
                }
        }
		void initMessageSenderThreads(){
			for(int i=0;i<outDeg;i++)
				{
                    cout<<id<<" to "<<outDegreeVertices[i]<<" "<<serverPortSeed + outDegreeVertices[i]<<"\n";
					messageSenderThreads[i] = thread(&Node::sendMessage,this , serverPortSeed+outDegreeVertices[i] , outDegreeVertices[i]);
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
        cout<<inDeg<<" indeg\n";
        outDeg = outDegreeVertices.size();
        clientSocketIds = new int[inDeg + 1];
        clientListenerThreads = new thread[inDeg + 1];
        messageSenderThreads  = new thread[outDeg + 1]; 
        this->id = id;     	        
        init();
    }
    void startListenerThreads(){
        initClientListnerThreads();
    }
    void startSenderThreads(){
        initMessageSenderThreads();
    }
    ~Node(){
        server.join();
        for(int i=0;i<inDeg;i++)
            clientListenerThreads[i].join();

        for(int i=0;i<outDeg;i++)
            messageSenderThreads[i].join();
    }


};

int main()
{

    int n;
    ifstream input("inp-params.txt"); // take input from inp-params.txt
    string str2;        
    getline(input,str2);
    n = 3;
    
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
    serverPortSeed = random(10000,11000);
    clientPortSeed = random(11000 ,12000);

    for(int i=1;i<=n;i++){      
        nodes[i] = new Node(inverseAdjacencyList[i],adjacencyList[i],i);
    }
    
    while(!WaitingSet.empty());


    for(int i=1;i<=n;i++){
        nodes[i]->startSenderThreads();
    }
    sleep(5);
    for(int i=1;i<=n;i++){
        nodes[i]->startListenerThreads();
    }  

    // don't terminate untill all the messages are done
    sleep(10) ;
}