#include<bits/stdc++.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;
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
int main(){
    int a = 10;
    int b =htons(a);
    int c = ntohs(b);
    vector< vector<int> > ans = parseString("[10 0 10][11 4 5][50 6 7000]");
    for(auto k:ans){
        for(auto l:k)
            cout<<l<<" ";
        cout<<endl;
    }
}