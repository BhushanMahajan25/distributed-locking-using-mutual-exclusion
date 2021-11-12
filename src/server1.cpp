// Original berkley server
/** @file serverMain.cpp
 * 	@brief This file contains multi-threaded server socket communicaion with clients.
 * The server reads and stores the user records from Records.txt
 * Performs transaction operations as per transaction data received by client.
 * Seperate function to add interst rate amount to all users accounts.
 * Handles all the operations with mutiple threads and effectivly uses mutex lock/unlock mechanism
 */

#include "../headers/common.hpp"
#include<pthread.h>
#include<iostream>
using namespace std;
#define THREAD_POOL_SIZE 64
#define MAX 9999

int csFlag = 0;

int serverFd = 0, clientSocketFd = 0;
int clientFdArr[MAX] = {-1};
int threadCounter = 0;
bool nextStep = true;

queue<int>clientQ;	//Queue for client requesting the shared access;

pthread_t threadPool[THREAD_POOL_SIZE];
pthread_mutex_t csMutex, qMutex;
vector<pthread_mutex_t> mx; // vector of mutex
pthread_cond_t csConditionVar = PTHREAD_COND_INITIALIZER;
pthread_cond_t cnd = PTHREAD_COND_INITIALIZER; // condition variable

void handleReadWriteConnection(int*);
void* threadFunction(void*);

/** @brief Assigns a thread from thread-pool to recently connected client
 * If a client is not connected then the thread waits until it gets signal of client connection.
 * This saves computations as otherwise the thread would have checked, if client connection is established, recurrsively.
*/
//https://www.youtube.com/watch?v=P6Z5K8zmEmc&list=PL9IEJIKnBJjFZxuqyJ9JqVYmuFZHr7CFM&index=7
void* threadFunction(void* arg){
	while(true){
		int* pClient;
		pthread_mutex_lock(&mx.at(0));
		if((pClient = dequeue()) == NULL){
			pthread_cond_wait(&cnd, &mx.at(0));
			//try again
			pClient = dequeue();
		}
		pthread_mutex_unlock(&mx.at(0));
		handleReadWriteConnection(pClient);
		close(*pClient);
	}
	return NULL;
}



/** @brief handles send/receive operations of socket.
 * This function receives transaction data from client 
 * and performs corresponding transaction function with the help of lock/unclock mechanism
 * @param pClientSocketFd integer pointer pointing to client socketfd
*/
void handleReadWriteConnection(int *pClientSocketFd){
	char buffer[1024];
	bzero(buffer, 1024);

	//while(true){
		if(recv(*pClientSocketFd, (void*) &buffer, sizeof(buffer), 0) < 0){
			cout<<"ERROR:: Cannot read data from client"<<endl;
		} 
		else{ 
			cout<<"SUCCESS:: Received data from client:: "<<buffer<<endl;
			if(strcmp(buffer,"REQUEST") == 0){
				cout<<"File Access Request by Client process P:: "<<*pClientSocketFd<<endl;
				cout<<"current csFlag:: "<<csFlag<<endl;
				pthread_mutex_lock(&csMutex);
				if(csFlag == 1){	//When there is someone already in critical section
					
					cout<<"Pushing client "<<*pClientSocketFd<<" in Q"<<endl;
					clientQ.push(*pClientSocketFd);
					pthread_cond_wait(&csConditionVar, &csMutex);
					//pthread_mutex_lock(&csMutex);
					// while(nextStep != true){
					// 	continue;
					// }
					if(!clientQ.empty()){
						csFlag = 1;
						nextStep = false;
						int grantedClientFD = clientQ.front();
						cout<<"after waiting... now granting access to client:: "<<grantedClientFD<<endl;
						clientQ.pop();
						bzero(buffer, 1024);
						strcpy(buffer, "OK");
						if(send(grantedClientFD, (void*) &buffer, sizeof(buffer), 0) < 0){
							cout<<"ERROR:: Cannot send OK response to client!!"<<endl;
						}else {
							cout<<"SUCCESS:: Permission granted for client:: "<<grantedClientFD<<endl;
						}
					}
				}
				else {
					cout<<"This is the first client to request the file!"<<endl;
					cout<<"csFlag:: "<<csFlag<<endl;
					csFlag = 1;
					nextStep = false;
					bzero(buffer, 1024);
					strcpy(buffer, "OK");
					if(send(*pClientSocketFd, (void*) &buffer, sizeof(buffer), 0) < 0){
						cout<<"ERROR:: Cannot send OK response to client!!"<<endl;
					}else {
						cout<<"SUCCESS:: Permission granted for client:: "<<*pClientSocketFd<<endl;
					}
				}
				//pthread_mutex_unlock(&csMutex);
				pthread_mutex_unlock(&csMutex);
			}
			if(strcmp(buffer, "RELEASE") == 0){
				pthread_mutex_lock(&csMutex);
				if(csFlag == 1){
					csFlag = 0;
					cout<<"File released by by Client Process:: "<<*pClientSocketFd<<endl;
					close(*pClientSocketFd);
					nextStep = true;
					
				}
				pthread_cond_signal(&csConditionVar);
				pthread_mutex_unlock(&csMutex);
			}
		}
	//}
	pthread_exit(NULL);
}

int main(int argc, char **argv) {
	// exit the program if invalid arguments are passed
	if(argc != 3 ){
        cout<<"ERROR: Invalid Arguments! Please run the program with args as ./<executable server file> <IP address> <server port>"<<endl;
        exit(EXIT_FAILURE);
    }

	/*socket variables*/
	int serverPort = stoi(argv[2]);
	int setReuseAddr = 1; // ON == 1  
	int maxPendingConnections = 1;
	char* serverIP = argv[1];

	struct sockaddr_in server;
	struct sockaddr_in client;
	socklen_t clientAddrLen; 

	//initialize the vector of mutexes with 0
	mx.resize(MAX);
	for(int i = 0; i < MAX; i++){
		pthread_mutex_init(&mx.at(i), 0);
	}

	//create threads in thread_pool and reuse them for future client connections
	for(int i = 0; i < THREAD_POOL_SIZE; i++){
		pthread_create(&threadPool[i], NULL, threadFunction, NULL);
	}

	// create socket (IPv4, stream-based, protocol set to TCP)
	if((serverFd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		cout<<"ERROR:: Server failed to create the listening socket"<<endl;
		exit(EXIT_FAILURE);
	}

	// set socket option to reuse the address passed in terminal args
	if(setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &setReuseAddr, sizeof(setReuseAddr)) != 0){
		cout<<"ERROR:: Server failed to set SO_REUSEADDR socket option"<<endl;
	}
	// set socket option to reuse the address passed in terminal args
	// wrote seperate if block fo SO_REUSEPORT because SO_REUSEADDR | SO_REUSEPORT in single if block was producing errors
	if(setsockopt(serverFd, SOL_SOCKET, SO_REUSEPORT, &setReuseAddr, sizeof(setReuseAddr)) != 0){
		cout<<"ERROR:: Server failed to set SO_REUSEPORT socket option"<<endl;
	}

	// configure server socket address structure (init to zero, IPv4,
	// network byte order for port and address)
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(serverIP);
	server.sin_port = htons(serverPort); //use port specified in terminal args

	// bind the socket
	if(::bind(serverFd, (struct sockaddr*) &server, sizeof(server)) < 0){ //c++11
		cout<<"ERROR:: Server failed to bind"<<endl;
		exit(EXIT_FAILURE);
	}

	// listen on the socket for up to some maximum pending connections
	if(listen(serverFd, maxPendingConnections) < 0){
		cout<<"ERROR:: Server failed to listen"<<endl;
		exit(EXIT_FAILURE);
	} else{
		cout<<"SUCCESS:: Server listening for a connection on port:: "<<serverPort<<endl;
	}

	// get the size client's address struct
	clientAddrLen = sizeof(client);

	while(true){
		// accept a new client
		if((clientSocketFd = accept(serverFd, (struct sockaddr*) &client, &clientAddrLen)) < 0){
			cout<<"ERROR:: Server accept failed"<<endl;
		} else{
			cout<<"SUCCESS:: Server accepted a client! "<<clientSocketFd<<endl;

			//using deque STL to store the connection so that a thread from thread pool can find it.
			int* pClient = new int;
			*pClient = clientSocketFd;
			pthread_mutex_lock(&mx.at(threadCounter));
			enqueue(pClient);
            clientFdArr[threadCounter] = *pClient;
			pthread_cond_signal(&cnd);
			pthread_mutex_unlock(&mx.at(threadCounter));
			threadCounter++;
		}
	}
	// Close the socket
	for(int fd=0; fd<threadCounter; fd++){
		close(clientFdArr[fd]);
	}
	close(serverFd);
	return 0;
}