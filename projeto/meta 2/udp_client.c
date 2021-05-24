#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "menus.h"
#include "fileIO.h"

#define BUFLEN 512
#define SERVER_PORT 100 // Porto para recepção das mensagens
#define SERVER_IP "193.136.212.129"

void sendMsg(int fd, char *msg, struct sockaddr_in addr, socklen_t slen);
char *receiveMsg(int fd, struct sockaddr_in serverAddr);
char *getMsgType(char *msg);
char *getMsgContent(char *msg);
char *makeMsg(char *type, char *content);
User *login(int fd, struct sockaddr_in serverAddr, int slen);

int main()
{
    struct sockaddr_in serverAddr, p2pAddr;
    int fd, slen = sizeof(serverAddr);

    // Flow-control flags
    int quit = 0, loggedIn = 0;

    // Socket fd
    if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        erro("main", "socket");

    // Fill socket address struct
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    while (!quit)
    {
        // LOGIN
        loggedIn = 0;
        User *u = NULL;
        while (!loggedIn)
        {
            mainMenu();
            int mainMenuOpt = getOption();
            if (mainMenuOpt == 0) // Login
            {
                if ((u = login(fd, serverAddr, slen)) != NULL)
                    loggedIn = 1;
            }
            else if (mainMenuOpt == 1) // Quit
            {
                quit = 1;
                break;
            }
        }
        if (loggedIn)
        {
            int commMethod;
            do
            {
                authorizedCommsMenu(u);
                commMethod = getOption();
                if (commMethod == 0) // Client-Server
                {
                    printf("Chat with [userId]: ");
                    char *destinationUserId = getTextField();
                    while (1)
                    {
                        printf("Send message [-1 to exit, 0 to receive message]: ");
                        char *msg = getTextField();
                        if (!strcmp(msg, "-1")) // Leave
                        {
                            free(msg);
                            free(destinationUserId);
                            break;
                        }
                        else // Chat
                        {
                            if (strcmp(msg, "0") != 0)
                            {
                                // Send first
                                char buf[512];
                                sprintf(buf, "( %s ) %s", destinationUserId, msg);
                                buf[strlen(buf) + 1] = '\0';
                                sendMsg(fd, makeMsg("CS_CHAT", buf), serverAddr, slen); // send to server
                            }

                            // Receive first
                            char destBuf[128], msg[128];
                            char *reply = receiveMsg(fd, serverAddr);
                            char *replyContent = getMsgContent(strdup(reply));
                            sscanf(replyContent, "( %s ) %[^\\0]", destBuf, msg);
                            printf("%s: %s\n", destinationUserId, msg);
                        }
                    }
                }
                else if (commMethod == 1) // P2P
                {
                    // pede ao servidor o address e porto udp do destino (clientes ouvem tds no mm port#)
                    printf("Destination [userId]: ");
                    char *destinationUserId = getTextField();
                    sendMsg(fd, makeMsg("P2P_DESTREQ", destinationUserId), serverAddr, slen); // send to server

                    char *reply = receiveMsg(fd, serverAddr);
                    char *replyType = getMsgType(strdup(reply));
                    char *replyContent = getMsgContent(strdup(reply)); // peer ipAddr
                    if (!strcmp(replyType, "P2P_DESTREP"))
                    {
                        // to receive and send to peer
                        p2pAddr.sin_family = AF_INET;
                        p2pAddr.sin_port = htons(SERVER_PORT);
                        p2pAddr.sin_addr.s_addr = inet_addr(replyContent);
                        while (1)
                        {
                            printf("Message [-1 to exit]: ");
                            char *msg = getTextField();
                            if (!strcmp(msg, "-1"))
                            {
                                free(msg);
                                free(destinationUserId);
                                break;
                            }
                            sendMsg(fd, msg, p2pAddr, slen); // send to peer
                            free(destinationUserId);
                            free(msg);

                            char *reply = receiveMsg(fd, p2pAddr);
                            printf("%s: %s\n", destinationUserId, replyContent);
                            free(reply);
                            free(replyContent);
                        }
                    }
                }
                else if (commMethod == 2) // Multicast
                {
                    // se opt == mc:
                    //   se eu quiser criar um grupo -> envia req de mc, reply é um mcaddr qualquer
                    //   se eu quiser receber msgs de um grupo -> inserir ip do grupo
                }
                else if (commMethod == 4) // Return to Login
                    break;
            } while (commMethod != 4);
        }
    }
    printf("Client left.\n");
    close(fd);
    return 0;
}

void sendMsg(int fd, char *msg, struct sockaddr_in addr, socklen_t slen)
{
    int nread;
    msg[strlen(msg)] = '\0';
    if ((nread = sendto(fd, msg, strlen(msg) + 1, 0, (struct sockaddr *)&addr, slen)) == -1)
        erro("[UDP_Client] sendMsg", "sendto");
}

char *receiveMsg(int fd, struct sockaddr_in serverAddr)
{
    char *buffer = (char *)malloc(BUFLEN);
    int nread;
    socklen_t slen = sizeof(serverAddr);
    if ((nread = recvfrom(fd, buffer, BUFLEN, 0, (struct sockaddr *)&serverAddr, (socklen_t *)&slen)) == -1)
        erro("[UDP_Client] receiveMsg", "recvfrom");
    buffer[nread] = '\0';
    //printf("Received: %s--\n", buffer);
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

User *login(int fd, struct sockaddr_in serverAddr, int slen)
{
    // Send credentials
    User *u = NULL;
    while (1)
    {
        char *credentials = loginMenu();
        sendMsg(fd, makeMsg("LOGIN", credentials), serverAddr, slen);
        free(credentials);

        // Reply contains user data
        char *loginReply = receiveMsg(fd, serverAddr);
        char *loginReplyContent = getMsgContent(strdup(loginReply));
        if (!strcmp(getMsgType(strdup(loginReply)), "LOGIN"))
        {
            u = stringToUser(loginReplyContent, ",");
            printf("Login successful! Welcome %s!\n", u->userId);
            return u;
        }
        else
            printf("%s\n", loginReplyContent);
    }
}