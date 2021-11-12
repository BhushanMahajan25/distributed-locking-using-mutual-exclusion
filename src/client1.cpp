#include<fstream>
#include "../headers/common.hpp"


// This function is cited from "https://stackoverflow.com/questions/2390912/checking-for-an-empty-file-in-c"
bool is_file_empty(fstream& pFile){
    return pFile.peek() == ifstream::traits_type::eof();
}

/** @brief Function to perform operations on file after access grant
 *  @param filePath string path of file
 */
void readWriteData(string filePath){
	string line;
	fstream sharedFile;
	sharedFile.open(filePath, ios::in);
	getline(sharedFile, line);
	cout<<endl<<"Reading....Counter is:: "<<line<<endl;
	sharedFile.close();

	int counter = stoi(line);
	line.empty();
	counter += 7;

	//sleep(1);

	sharedFile.open(filePath, ios::out);
	sharedFile << to_string(counter);
	sharedFile.close();

	//sleep(1);

	sharedFile.open(filePath, ios::in);
	getline(sharedFile, line);
	cout<<"Updated Counter is:: "<<counter<<endl;
	sharedFile.close();
}

int main(int argc, char **argv){

    // exit the program if invalid arguments are passed
	if(argc != 3 ){
		cout<<"ERROR: Invalid Arguments! Please run the program with args as ./<exe file> <IP address> <co-ordinator port>"<<endl;
		exit(EXIT_FAILURE);
	}

	int clientFd;
	int serverPort = stoi(argv[2]);
	char* serverAddress = argv[1];
	struct sockaddr_in serverSocketAddr;

	// Create socket (IPv4, stream-based, protocol set to TCP)
	if((clientFd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		cout<<"ERROR:: Client failed to create socket"<<endl;
		exit(EXIT_FAILURE);
	}
	
	// Configure co-ordinator socket address structure (init to zero, IPv4,
	// network byte order for port and address) 
	bzero(&serverSocketAddr, sizeof(serverSocketAddr));
	serverSocketAddr.sin_family = AF_INET;
	serverSocketAddr.sin_port = htons(serverPort);
	serverSocketAddr.sin_addr.s_addr = inet_addr(serverAddress);

	// Connect socket to co-ordinator
	if(connect(clientFd, (struct sockaddr*) &serverSocketAddr, sizeof(serverSocketAddr)) < 0){
		cout<<"ERROR:: Client failed to connect to "<<serverAddress<<" : "<<serverPort<<endl;
		close(clientFd);
		exit(EXIT_FAILURE);
	} else {
		cout<<"SUCCESS:: Client connected to "<<serverAddress<<" : "<<serverPort<<endl;
		fstream sharedFile;
		string filePath = "input-files/sharedFile.txt";
        char buffer[1024];

		//while(true){
			bzero(buffer,1024);
			strcpy(buffer, "REQUEST");
			if(send(clientFd, (void*) &buffer, sizeof(buffer), 0) < 0){
				cout<<"ERROR:: Cannot send REQUEST to co-ordinator!!"<<endl;
			}else {
				cout<<"SUCCESS:: REQUEST sent to the co-ordinator "<<endl;
				bzero(buffer, 1024);
				if(recv(clientFd, (void*) &buffer, sizeof(buffer), 0) < 0){
					cout<<"ERROR:: Cannot receive data from the co-ordinator !"<<endl;
				}
				else {
					cout<<"SUCCESS:: Data received from the co-ordinator:: "<<buffer<<endl;
					if(strcmp(buffer,"OK") == 0){
						readWriteData(filePath);
						bzero(buffer, 1024);
						strcpy(buffer, "RELEASE");
						if(send(clientFd, (void*) &buffer, sizeof(buffer), 0) < 0){
							cout<<"ERROR:: Cannot send RELEASE to co-ordinator!!"<<endl;
						}else {
							cout<<"SUCCESS:: RELEASE sent to the co-ordinator "<<endl;
							//break;
						}
					}
				}
			}
		//}
	}

	// Close the socket and return 0
	close(clientFd);
	return 0; 
}