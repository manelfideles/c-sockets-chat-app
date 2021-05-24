#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "fileIO.h"

#define BUFLEN 512
#define SERVER_PORT 100
#define MAX_CLIENTS 32
#define SERVER_IP "10.90.0.1"
#define DBPATH "userDB.txt"

void sendMsg(int fd, char *msg, struct sockaddr_in *addr, socklen_t slen);
char *receiveMsg(int fd, struct sockaddr_in *addr, socklen_t slen);
char *getMsgType(char *msg);
char *getMsgContent(char *msg);
char *makeMsg(char *type, char *content);
int clientInArray(struct sockaddr_in *addr, struct sockaddr_in **allClients, int clientCounter); /* param: client, array of clients */
struct sockaddr_in *findClientAddr(char *ipAddr, struct sockaddr_in **allClients, int clientCounter);
void printClientAddrs(struct sockaddr_in **allClients, int clientCounter);
struct sockaddr_in **allocateClientArray(int nelem);
int isEqual(char *x, char *y);

int main()
{
	int fd;
	struct sockaddr_in serverAddr;
	struct sockaddr_in *clientAddr;
	socklen_t slen = sizeof(struct sockaddr_in);
	struct sockaddr_in **allClients = allocateClientArray(MAX_CLIENTS);
	int clientCounter = 0;

	// Cria um socket para recepção de pacotes UDP
	if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		erro("main", "socket");

	// Preenchimento da socket address struct
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	// Associa socket à informação de endereço
	if (bind(fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
		erro("main", "bind");

	printf("Server waiting for incoming messages...\n\n");
	while (1)
	{
		clientAddr = allClients[clientCounter];
		char *rcvMsg = receiveMsg(fd, clientAddr, slen);
		char *msgType = getMsgType(strdup(rcvMsg));
		char *msgContent = getMsgContent(strdup(rcvMsg));

		/*
		printf("\nMsg: %s--\nType: %s--\nContent: %s--\nSender: %s:%d--\n---------------\n",
			   rcvMsg,
			   msgType,
			   msgContent,
			   inet_ntoa(clientAddr->sin_addr),
			   ntohs(clientAddr->sin_port));
		*/

		if (clientCounter == 0 || !clientInArray(clientAddr, allClients, clientCounter))
		{
			clientCounter++;
			printf("New client %s added to client array!\n", inet_ntoa(clientAddr->sin_addr));
			printClientAddrs(allClients, clientCounter);
		}

		if (!strcmp(msgType, "LOGIN"))
		{
			char userId[64], password[64];
			sscanf(msgContent, "%s %s", userId, password);
			User *u = searchUserOnFile(DBPATH, userId);
			char *reply;
			if (u && !strcmp(password, u->password))
				reply = makeMsg("LOGIN", userToString(u));
			else
				reply = makeMsg("ERROR", "Oops! Something went wrong :(");
			sendMsg(fd, reply, clientAddr, slen);
			free(reply);
			free(u);
		}
		else if (!strcmp(msgType, "CS_CHAT"))
		{
			// sending what? to whom?
			char destinationUserId[128], msg[128];
			sscanf(msgContent, "( %s ) %s", destinationUserId, msg);
			User *u = searchUserOnFile(DBPATH, destinationUserId);
			if (u)
			{
				struct sockaddr_in *csAddr = findClientAddr(u->ipAddr, allClients, clientCounter);
				if (csAddr)
				{
					sendMsg(fd, makeMsg("#CS_CHAT", msgContent), csAddr, slen);
					//free(csAddr);
				}
				//free(u);
			}
			else
				sendMsg(fd, makeMsg("#CS_CHAT", "User doesn't exist! Try again."), clientAddr, slen);
		}
		else if (!strcmp(msgType, "P2P_DESTREQ"))
		{
			User *u = searchUserOnFile(DBPATH, msgContent); // msgContent -> userId
			printf("Sending ipAddr of %s to address %s\n", u->userId, inet_ntoa(clientAddr->sin_addr));
			sendMsg(fd, makeMsg("P2P_DESTREP", u->ipAddr), clientAddr, slen);
			free(u);
		}
		// else if (!strcmp(msgType, "")) {;}
		// else {;}
	}
	close(fd);
	return 0;
}

void sendMsg(int fd, char *msg, struct sockaddr_in *addr, socklen_t slen)
{
	int nread;
	msg[strlen(msg)] = '\0';
	if ((nread = sendto(fd, msg, BUFLEN, 0, (struct sockaddr *)addr, slen)) == -1)
		erro("[UDP_Server] sendMsg", "sendto");
}

char *receiveMsg(int fd, struct sockaddr_in *addr, socklen_t slen)
{
	char *buffer = (char *)malloc(BUFLEN);
	ssize_t nread;
	if ((nread = recvfrom(fd, buffer, BUFLEN, 0, (struct sockaddr *)addr, (socklen_t *)&slen)) == -1)
		erro("[UDP_Client] receiveMsg", "recvfrom");
	buffer[nread] = '\0';

	fflush(stdout);
	return buffer;
}

char *getMsgType(char *msg)
{
	char *type = (char *)malloc(64);
	sscanf(msg, "%s", type);
	return type;
}

char *getMsgContent(char *msg)
{
	char *content = (char *)malloc(256);
	strtok(msg, " ");
	content = strtok(NULL, "\0");
	//printf("%s\n", content);
	return content;
}

char *makeMsg(char *type, char *content)
{
	char *msg = (char *)malloc(BUFLEN);
	sprintf(msg, "%s %s", type, content);
	msg[strlen(msg)] = '\0';
	return msg;
}

int clientInArray(struct sockaddr_in *addr, struct sockaddr_in **allClients, int clientCounter)
{
	//printf("inet_ntoa(addr->sin_addr): %s\n", inet_ntoa(addr->sin_addr));
	char *clientAddr = strdup(inet_ntoa(addr->sin_addr));
	//printf("clientAddr: %s\n", clientAddr);
	for (int i = 0; i < clientCounter; i++)
	{
		//printf("allClients[%d] - clientAddr: %s - %s\n", i, inet_ntoa(allClients[i]->sin_addr), clientAddr);
		if (!strcmp(strdup(inet_ntoa(allClients[i]->sin_addr)), clientAddr))
			return 1;
	}
	return 0;
}

struct sockaddr_in *findClientAddr(char *ipAddr, struct sockaddr_in **allClients, int clientCounter)
{
	for (int i = 0; i < clientCounter; i++)
		if (!strcmp(strdup(inet_ntoa(allClients[i]->sin_addr)), ipAddr))
			return allClients[i];
	return NULL;
}

void printClientAddrs(struct sockaddr_in **allClients, int clientCounter)
{
	printf("Client Addresses:\n");
	for (int i = 0; i < clientCounter; i++)
		printf("Client %d: %s\n", i, inet_ntoa(allClients[i]->sin_addr));
	printf("-----------------\n\n");
}

struct sockaddr_in **allocateClientArray(int nelem)
{
	struct sockaddr_in **allClients = (struct sockaddr_in **)malloc(sizeof(struct sockaddr_in *) * nelem);
	for (int i = 0; i < nelem; i++)
		allClients[i] = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
	return allClients;
}
