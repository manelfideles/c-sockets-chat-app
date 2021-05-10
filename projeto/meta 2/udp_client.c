#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "menus.h"
#include "fileIO.h"

#define BUFLEN 512
#define PORT 9876 // Porto para recepção das mensagens
#define SERVER_IP "127.0.0.1"

void sendMsg(int fd, char *msg, struct sockaddr_in addr, socklen_t slen);
char *receiveMsg(int fd, struct sockaddr_in serverAddr);
char *getMsgType(char *msg);
char *getMsgContent(char *msg);
char *makeMsg(char *type, char *content);

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
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP); // depois mudar para INADDR_ANY

    while (!quit)
    {
    // LOGIN
    Login:;
        User *u = NULL;
        while (!loggedIn)
        {
            mainMenu();
            int mainMenuOpt = getOption();
            if (mainMenuOpt == 0) // Login
            {
                // Send credentials
                char *credentials = loginMenu();
                sendMsg(fd, makeMsg("LOGIN", credentials), serverAddr, slen);
                free(credentials);

                // Reply contains user data
                char *loginReply = receiveMsg(fd, serverAddr);
                char *loginReplyContent = getMsgContent(strdup(loginReply));
                if (!strcmp(getMsgType(strdup(loginReply)), "LOGIN"))
                {
                    printf("Login successful!\n");
                    u = stringToUser(loginReplyContent, ",");
                    printf("Welcome %s!\n", u->userId);
                    loggedIn = 1;
                }
                else
                    printf("%s\n", loginReplyContent);
            }
            else if (mainMenuOpt == 1) // Quit
            {
                quit = 1;
                break;
            }
        }

    AuthorizedCommunications:;
        authorizedCommsMenu(u);
        int commMethod = getOption();
        if (commMethod == 0) // Client-Server
        {
            printf("Destination [userId]: ");
            char *destinationUserId = getTextField();
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

                // "[destination] msg"
                char buf[512];
                sprintf(buf, "[%s] %s", destinationUserId, msg);
                sendMsg(fd, makeMsg("CS_CHAT", buf), serverAddr, slen); // send to server
                free(destinationUserId);
                free(msg);

                char *reply = receiveMsg(fd, serverAddr);
                char *replyContent = getMsgContent(strdup(reply));
                printf("%s: %s\n", destinationUserId, replyContent);
                free(reply);
                free(replyContent);
            }
            goto AuthorizedCommunications;
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
                p2pAddr.sin_port = htons(PORT);
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
                    sendMsg(fd, msg, p2pAddr, slen); // send to server
                    free(destinationUserId);
                    free(msg);
                    char *reply = receiveMsg(fd, p2pAddr);
                    printf("%s: %s\n", destinationUserId, replyContent);
                    free(reply);
                    free(replyContent);
                }
                goto AuthorizedCommunications;
            }
        }
        else if (commMethod == 2) // Multicast
        {
            // se opt == mc:
            //   se eu quiser criar um grupo -> envia req de mc, reply é um mcaddr qualquer
            //   se eu quiser receber msgs de um grupo -> inserir ip do grupo
        }
        else if (commMethod == 3)
            goto Login;
        else
            goto AuthorizedCommunications;

        /* 
        char *rcvMsg = receiveMsg(fd, serverAddr);
        char *msgType = getMsgType(strdup(rcvMsg));
        char *msgContent = getMsgContent(strdup(rcvMsg));
        printf("Msg: %s--\nType: %s--\nContent: %s--\nSender: %s:%d--\n",
               rcvMsg,
               msgType,
               msgContent,
               inet_ntoa(serverAddr.sin_addr),
               ntohs(serverAddr.sin_port));

        if (!strcmp(msgType, "C-S"))
        {
            ;
        }
        if (!strcmp(msgType, "P2P"))
        {
            ;
        }
        if (!strcmp(msgType, "MCAST"))
        {
            ;
        }
        else
            sendMsg(fd, makeMsg("ERROR", "Reply not recognized!"), serverAddr);
        */
    }
    printf("Client left.\n");
    close(fd);
    return 0;
}

void sendMsg(int fd, char *msg, struct sockaddr_in addr, socklen_t slen)
{
    int nread;
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