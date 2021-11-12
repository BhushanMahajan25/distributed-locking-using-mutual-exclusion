#include "../headers/common.hpp"

queue<int*>clientSockQ;

void enqueue(int* pClient){
    clientSockQ.push(pClient);
}

int* dequeue(){
    if(!clientSockQ.empty()){
        int* pClient = clientSockQ.front();
        clientSockQ.pop();
        return pClient;
    }
    return NULL;
}