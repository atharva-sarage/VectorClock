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
  vector< vector<pair<int,int>> > parseString2(string str){
    vector< vector<pair<int,int>> > senderTimeVector;
    for(int i=0;i<str.size();i++){
        if(str[i]=='['){
            i++;
            string temp,temp2;
            vector <pair<int,int>> tempVec;
            while(str[i] != ']'){
                if(str[i] == '(') i++;
                while(str[i] != ','){
                    temp+=str[i];
                    i++;
                }

                if(str[i] == ',')i++;
                while(str[i] != ')'){
                    temp2+=str[i];
                    i++;
                }
                if(str[i]==')')i++;
                tempVec.push_back({stoi(temp),stoi(temp2)});
                temp.clear();
                temp2.clear();
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
    vector< vector<pair<int,int>> > ans = parseString2("[(10,20)(330,40)][(1,5)(7,8)]");
    for(auto k:ans){
        for(auto l:k)
            cout<<l.first<<" "<<l.second<<" * ";
        cout<<endl;
    }
}