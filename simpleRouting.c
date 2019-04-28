#include <stdio.h>
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
#define PORT_NUMBER 3605
#define IP_LENGTH 16

typedef struct
{
	char* destinationIp;
	int destinationPort;
	char* nextIp;
	int nextPort;
	int metric;
}ROUTING_TABLE_ENTRY;

typedef struct
{
	char destinationIp[IP_LENGTH];
	char sourceIp[IP_LENGTH];
	char message[BUFFER_SIZE];
}PACKET;

PACKET *receivePacket;
int receiveFlag = 0;

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

void *clientThreadFunction(void *arg)
{
	int i;
	char** neighborIpAddress = (char **)arg;
	char* myIpAddress = getIpAddress();
	char* buffer;
	PACKET sendPacket;
	size_t getlineLength;
	int numberOfConnections;
	for(numberOfConnections = 0; neighborIpAddress[numberOfConnections] != NULL; numberOfConnections++);
	struct sockaddr_in clientSocketAddress[numberOfConnections];
	int clientSocket[numberOfConnections];
	for(i = 0; i < numberOfConnections; i++)
	{
		clientSocket[i] = socket(AF_INET, SOCK_STREAM, 0);
		if(clientSocket[i] == -1)
		{
			perror("Client Socket Creation Failure");
			pthread_exit(NULL);
		}
		memset(&clientSocketAddress[i], 0, sizeof(clientSocketAddress[i]));
		clientSocketAddress[i].sin_family = AF_INET;
		clientSocketAddress[i].sin_port = htons(PORT_NUMBER);
		inet_pton(AF_INET, neighborIpAddress[i], &clientSocketAddress[i].sin_addr);
		while(connect(clientSocket[i], (struct sockaddr *)&clientSocketAddress[i], sizeof(clientSocketAddress[i])) == -1);
	}
	while(1)
	{
		memset(&sendPacket, 0, sizeof(PACKET));
		buffer = NULL;
		getline(&buffer, &getlineLength, stdin);
		strcpy(sendPacket.destinationIp, strtok(buffer, "\n"));
		buffer = NULL;
		getline(&buffer, &getlineLength, stdin);
		strcpy(sendPacket.message, buffer);
		strcpy(sendPacket.sourceIp, myIpAddress);
		for(i = 0; i < numberOfConnections; i++)
		{
			send(clientSocket[i], &sendPacket, sizeof(PACKET), 0);
		}
	}
}

void *routerThreadFunction(void *arg)
{
	int i;
	char** neighborIpAddress = (char **)arg;
	char* myIpAddress = getIpAddress();
	PACKET sendPacket;
	int numberOfConnections;
	for(numberOfConnections = 0; neighborIpAddress[numberOfConnections] != NULL; numberOfConnections++);
	struct sockaddr_in clientSocketAddress[numberOfConnections];
	int clientSocket[numberOfConnections];
	for(i = 0; i < numberOfConnections; i++)
	{
		clientSocket[i] = socket(AF_INET, SOCK_STREAM, 0);
		if(clientSocket[i] == -1)
		{
			perror("Client Socket Creation Failure");
			pthread_exit(NULL);
		}
		memset(&clientSocketAddress[i], 0, sizeof(clientSocketAddress[i]));
		clientSocketAddress[i].sin_family = AF_INET;
		clientSocketAddress[i].sin_port = htons(PORT_NUMBER);
		inet_pton(AF_INET, neighborIpAddress[i], &clientSocketAddress[i].sin_addr);
		while(connect(clientSocket[i], (struct sockaddr *)&clientSocketAddress[i], sizeof(clientSocketAddress[i])) == -1);
	}
	while(1)
	{
		memset(&sendPacket, 0, sizeof(PACKET));
		while(receiveFlag != 1);
		strcpy(sendPacket.message, receivePacket->message);
		strcpy(sendPacket.sourceIp, receivePacket->sourceIp);
		strcpy(sendPacket.destinationIp, receivePacket->destinationIp);
		for(i = 0; i < numberOfConnections; i++)
		{
			if(strcmp(sendPacket.destinationIp, neighborIpAddress[i]) == 0)
			{
				printf("%s -> %s\n", sendPacket.sourceIp, sendPacket.destinationIp);
				send(clientSocket[i], &sendPacket, sizeof(PACKET), 0);
			}
		}
		receiveFlag = 0;
	}
}

void *serverThreadFunction(void *arg)
{
	int clientSocket = *(int *)arg;
	char* myIpAddress = getIpAddress();
	char* receiveBuffer = (char *)malloc(sizeof(PACKET));
	while(1)
	{
		recv(clientSocket, receiveBuffer, sizeof(PACKET), 0);
		receivePacket = (PACKET*)receiveBuffer;
		if(strcmp(receivePacket->destinationIp, myIpAddress) == 0)
			printf("%s: %s",receivePacket->sourceIp, receivePacket->message);
		receiveFlag = 1;
	}
}

void main(int argc, char* args[])
{
	int threadNumber = 1;
	pthread_t clientThread;
	pthread_t routerThread;
	pthread_t serverThread[(NODE_NUMBER - 1) * 2];
	pthread_create(&clientThread, NULL, clientThreadFunction, args + 1);
	pthread_create(&routerThread, NULL, routerThreadFunction, args + 1);
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
	while(1)
	{
		if(listen(serverSocket, 0) == -1)
		{
			perror("Server Listen Standby Mode Failure");
			close(serverSocket);
			exit(0);
		}
		clientSocket = accept(serverSocket, (struct sockaddr *)NULL, NULL);
		if(clientSocket == -1)
		{
			perror("Server Connect Accept Failure");
			close(serverSocket);
			exit(0);
		}
		pthread_create(&serverThread[threadNumber++], NULL, serverThreadFunction, &clientSocket);
	}
}