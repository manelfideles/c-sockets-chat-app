#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "menus.h"
#include "fileIO.h"

#define PORT 9876
#define BUFLEN 512
#define CLIENT_IP "127.0.0.1"
#define DBPATH "userDB.txt"

void sendMsg(int fd, char *msg, struct sockaddr_in *addr, socklen_t slen);
char *receiveMsg(int fd, struct sockaddr_in *addr, socklen_t slen);
char *getMsgType(char *msg);
char *getMsgContent(char *msg);
char *makeMsg(char *type, char *content);

int main()
{
	int fd;
	struct sockaddr_in serverAddr;
	struct sockaddr_in *clientAddr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
	socklen_t slen = sizeof(struct sockaddr_in);

	// Cria um socket para recepção de pacotes UDP
	if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		erro("main", "socket");

	// Preenchimento da socket address struct
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	//serverAddr.sin_addr.s_addr = inet_addr(CLIENT_IP);

	// Associa socket à informação de endereço
	if (bind(fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
		erro("main", "bind");

	printf("Server waiting for incoming messages...\n");
	while (1)
	{
		char *rcvMsg = receiveMsg(fd, clientAddr, slen);
		char *msgType = getMsgType(strdup(rcvMsg));
		char *msgContent = getMsgContent(strdup(rcvMsg));

		/*
		printf("Msg: %s--\nType: %s--\nContent: %s--\nSender: %s:%d--\n",
			   rcvMsg,
			   msgType,
			   msgContent,
			   inet_ntoa(clientAddr->sin_addr),
			   ntohs(clientAddr->sin_port));
		*/

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
			while (1)
			{
				// sscanf no msg content para ler o destino
				// query a bd pelo ip associado ao destino
				// montar a struct sockaddr_in
				// enviar/receber dessa struct
				break;
			}
		}
		else if (!strcmp(msgType, "P2P_DESTREQ"))
		{
			// query na bd pelo ip associado ao destino
			// enviar so o addr (type: P2P_DESTREP) ex: "127.0.0.2" (pq o port é smp o mm)
		}
		// else if (!strcmp(msgType, ""))
		// {
		// 	;
		// }
		// else
		// {
		// 	;
		// }
	}
	close(fd);
	return 0;
}

void sendMsg(int fd, char *msg, struct sockaddr_in *addr, socklen_t slen)
{
	int nread;
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

	//printf("Received: %s--\n", buffer);
	printf("Sender: %s:%d--\n",
		   inet_ntoa(addr->sin_addr),
		   ntohs(addr->sin_port));
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