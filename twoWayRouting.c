/*
	Edit By: Dokyung Lee
	Purpose: To simulate routing table protocol
	useful functions (socket, pthread, sigaction, timer, open, close, client, recv, bind, accept, listen, connect, memset, htons/l, ntohs/l, memset, malloc/free)
*/
#include<stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>
#include <ifaddrs.h>

#define NODE_NUMBER 5
#define BUFFER_SIZE 1024
#define PORT_NUMBER 3155
#define IP_LENGTH 16

#define ROUTER 1
#define CLIENT 0
#define SERVER 0

typedef struct
{
	char* destinationIp;
	int destinationPort;
	char* nextIp;
	int nextPort;
	int metric;
}ROUTING_TABLE_ENTRY;

typedef struct
{	//char sourceIp[IP_LENGTH];
	char destinationIp[IP_LENGTH];
	char message[BUFFER_SIZE];
}PACKET;

char* getIpAddress()
{
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    char *addr;
    getifaddrs (&ifap);
    for(ifa = ifap; ifa; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr->sa_family==AF_INET)
        {
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            addr = inet_ntoa(sa->sin_addr);
        }
    }
    freeifaddrs(ifap);
    return addr;
}
const char address[NODE_NUMBER][16]={"220.149.244.211", "220.149.244.212", "220.149.244.213", "220.149.244.214", "220.149.244.215"};
char nextHop[2][16] = {"220.149.244.214", "220.149.244.215"};
//const int port[5] = {3155, 3156, 3157, 3158, 3159};
char* nextIp; 
PACKET sendPacket;
PACKET* receivePacket;
int flag = 0;
char* sendBuffer;
char* receiveBuffer;
	size_t ipLength = IP_LENGTH;
	size_t getlineLength;
	struct sockaddr_in clientSocketAddress[NODE_NUMBER];
	//int serverNumber = 0;
	int clientSocket[NODE_NUMBER];
	int connectionCheck[NODE_NUMBER] = {0};
	char destinationIp[16];

void* routerThreadFunction(void* arg){
	int serverNumber = 0;
	while(1){
		while(flag != ROUTER);
		printf("\nI'm router\n");

		//receivePacket = (PACKET*)receiveBuffer;

		//strcpy(sendPacket.destinationIp, receivePacket->destinationIp);
		//strcpy(sendPacket.message, receivePacket->message);
printf("test : %s, %s\n", receivePacket->destinationIp, receivePacket->message);
		for(int i = 0; i < NODE_NUMBER; i++){
			if(strcmp(address[i], receivePacket->destinationIp) == 0){
				//strcpy(sendPacket.destinationIp, destinationIp);
				serverNumber = i;
				break;
			}
		}
if(connectionCheck[serverNumber] == 0){
			while(connect(clientSocket[serverNumber], (struct sockaddr *)&clientSocketAddress[serverNumber], sizeof(clientSocketAddress[serverNumber])) == -1);
		}

		if(connectionCheck[serverNumber] == 0) connectionCheck[serverNumber] = 1;


		send(clientSocket[serverNumber], receivePacket, sizeof(PACKET), 0);
		printf("send : %s\n", receivePacket->message);
		flag = SERVER;
	}

}

void *clientThreadFunction(void *arg)
{
	//char* ipAddress = nextIp;//(char *)arg;
	int serverNumber;	
	for(int i = 0; i < NODE_NUMBER; i++){ 

		clientSocket[i] = socket(AF_INET, SOCK_STREAM, 0);

		if(clientSocket[i] == -1)
		{
			perror("Client Socket Creation Failure");
			pthread_exit(NULL);
		}

		memset(&clientSocketAddress[i], 0, sizeof(clientSocketAddress[i]));
		clientSocketAddress[i].sin_family = AF_INET;
		clientSocketAddress[i].sin_port = htons(PORT_NUMBER);
		inet_pton(AF_INET, address[i], &clientSocketAddress[i].sin_addr);
	}
 
	
	//sending
	while(1)
	{	
		//while(flag != CLIENT);

		//if(flag == CLIENT){
			printf("input destination ip : ");
			gets(destinationIp);
		//}else if(flag == ROUTER){
		//	strcpy(destinationIp, receivePacket->destinationIp);
		//}

		for(int i = 0; i < NODE_NUMBER; i++){
			if(strcmp(address[i], destinationIp) == 0){
				strcpy(sendPacket.destinationIp, destinationIp);
				serverNumber = i;
				break;
			}
		}

		if(connectionCheck[serverNumber] == 0){
			while(connect(clientSocket[serverNumber], (struct sockaddr *)&clientSocketAddress[serverNumber], sizeof(clientSocketAddress[serverNumber])) == -1);
		}

		if(connectionCheck[serverNumber] == 0) connectionCheck[serverNumber] = 1;

		sendBuffer = NULL;
	
		//if(flag == CLIENT){
			printf("message : ");
			gets(sendPacket.message);
			//getline(&sendBuffer, &getlineLength, stdin);
		//}else if(flag == ROUTER){
		//	strcpy(sendPacket->message, receivePacket->message);	
		//}

		send(clientSocket[serverNumber], &sendPacket, sizeof(PACKET), 0);
		printf("send : %s to %s\n", sendPacket.message, sendPacket.destinationIp);
		
		//if(strcmp(sendBuffer, "exit\n") == 0)
		//	break;
	
		//if(flag != ROUTER) flag = CLIENT;
	}
//sending
	for(int i = 0; i < NODE_NUMBER; i++)close(clientSocket[i]);
	pthread_exit(NULL);
}

void *serverThreadFunction(void *arg)
{
	receiveBuffer = (char *)malloc(sizeof(PACKET));
	int clientSocket = *(int*)arg;
//receive
	
	flag = SERVER;

	while(1)
	{	
		recv(clientSocket, receiveBuffer, sizeof(PACKET), 0);
		receivePacket = (PACKET*)receiveBuffer;

		if(strcmp(receivePacket->destinationIp, getIpAddress()) != 0){
			//printf("to router : %s\n", receivePacket->message);
			//strcpy(sendPacket.destinationIp, receivePacket->destinationIp);
			//strcpy(sendPacket.message, receivePacket->message);
			flag = ROUTER;
		}
		//while(flag == ROUTER);
		printf("test\n");
		if(flag == SERVER){
			printf("receive : %s\n", receivePacket->message);
			//if(strcmp(receiveBuffer, "exit\n") == 0)
			//	break;
		}
;
	}
//receive
	//close(serverSocket);
	pthread_exit(NULL);
}

void main(int argc, char* args[])
{
	int threadNumber = 1;
	pthread_t clientThread;
	pthread_t serverThread[NODE_NUMBER];
	pthread_t routerThread;
	pthread_create(&clientThread, NULL, clientThreadFunction, args[1]);
	struct sockaddr_in serverSocketAddress;
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	int clientSocket;
	if(serverSocket == -1)
	{
		perror("Server Socket Creation Failure");
		pthread_exit(NULL);
	}
	memset(&serverSocketAddress, 0, sizeof(serverSocketAddress));
	serverSocketAddress.sin_family = AF_INET;
	serverSocketAddress.sin_addr.s_addr = htons(INADDR_ANY);
	serverSocketAddress.sin_port = htons(PORT_NUMBER);
	if(bind(serverSocket, (struct sockaddr *)&serverSocketAddress, sizeof(serverSocketAddress)) == -1)
	{
		perror("Server Bind Failure");
		close(serverSocket);
		pthread_exit(NULL);
	}

//	pthread_create(&serverThread, NULL, serverThreadFunction, NULL);
	pthread_create(&routerThread, NULL, routerThreadFunction, NULL);
	while(1){
	
		if(listen(serverSocket, 0) == -1){
			perror("Server Listen Standby Mode Failure");
			close(serverSocket);
			pthread_exit(NULL);
		}
	
		clientSocket = accept(serverSocket, (struct sockaddr *)NULL, NULL);
		if(clientSocket == -1){
			perror("Server Connect Accept Failure");
			close(serverSocket);
			pthread_exit(NULL);
		}
		printf("thread %d create\n");
		pthread_create(&serverThread[threadNumber++], NULL, serverThreadFunction, &clientSocket);
	
	}
	//pthread_join(clientThread, NULL);
	//pthread_join(serverThread, NULL);
	return;
}
