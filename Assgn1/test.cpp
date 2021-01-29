#include<bits/stdc++.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;
int main(){
    int a = 10;
    int b =htons(a);
    int c = ntohs(b);
    cout<<a<<" "<<b<<" "<<c<<endl;
}