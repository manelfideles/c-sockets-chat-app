#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "menus.h"
#include "fileIO.h"

#define BUFLEN 512
#define PORT 100 // Porto para recepção das mensagens
#define SERVER_IP "193.136.212.129"
#define MY_IP "193.136.212.194" // change according to docker's ipAddr

void sendMsg(int fd, char *msg, struct sockaddr_in addr, socklen_t slen);
void sendToPeer(int fd, char *msg, struct sockaddr_in *addr, socklen_t slen);
char *receiveMsg(int fd, struct sockaddr_in serverAddr);
char *rcvFromPeer(int fd, struct sockaddr_in *addr, socklen_t slen);
char *getMsgType(char *msg);
char *getMsgContent(char *msg);
char *makeMsg(char *type, char *content);
User *login(int fd, struct sockaddr_in serverAddr, int slen);
char *requestPeerIp(int fd, char *destinationUserId, struct sockaddr_in serverAddr, int slen);

int main()
{
    struct sockaddr_in serverAddr;
    struct sockaddr_in *peerAddr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
    struct sockaddr_in myAddr; // p2p
    int fd, slen = sizeof(serverAddr);

    // Flow-control flags
    int quit = 0, loggedIn = 0;

    // Socket fd
    if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        erro("main", "socket");

    // Fill socket address struct
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Fill and bind myAddr to socket
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(PORT);
    myAddr.sin_addr.s_addr = inet_addr(MY_IP);

    if (bind(fd, (struct sockaddr *)&myAddr, sizeof(myAddr)) == -1)
        erro("p2p", "bind");

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
                    printf("Chat with [userId]: ");
                    char *destinationUserId = getTextField();

                    // Fill peerAddr
                    peerAddr->sin_family = AF_INET;
                    peerAddr->sin_port = htons(PORT);
                    peerAddr->sin_addr.s_addr = inet_addr(requestPeerIp(fd, destinationUserId, serverAddr, slen));

                    printf("myAddr: %s:%d\n", inet_ntoa(myAddr.sin_addr),
                           ntohs(myAddr.sin_port));

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
                                sendToPeer(fd, makeMsg("P2P_CHAT", buf), peerAddr, slen); // send to server
                            }

                            // Receive first
                            char destBuf[128], msg[128];
                            char *reply = rcvFromPeer(fd, peerAddr, slen);
                            char *replyContent = getMsgContent(strdup(reply));
                            sscanf(replyContent, "( %s ) %[^\\0]", destBuf, msg);
                            printf("%s: %s\n", destinationUserId, msg);
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

char *receiveMsg(int fd, struct sockaddr_in addr)
{

    char *buffer = (char *)malloc(BUFLEN);
    int nread;
    socklen_t slen = sizeof(addr);
    if ((nread = recvfrom(fd, buffer, BUFLEN, 0, (struct sockaddr *)&addr, (socklen_t *)&slen)) == -1)
        erro("[UDP_Client] receiveMsg", "recvfrom");
    buffer[nread] = '\0';
    //printf("Received: %s--\n", buffer);
    fflush(stdout);
    return buffer;
}

void sendToPeer(int fd, char *msg, struct sockaddr_in *addr, socklen_t slen)
{
    int nread;
    msg[strlen(msg)] = '\0';
    if ((nread = sendto(fd, msg, BUFLEN, 0, (struct sockaddr *)addr, slen)) == -1)
        erro("[UDP_Server] sendMsg", "sendto");
}

char *rcvFromPeer(int fd, struct sockaddr_in *addr, socklen_t slen)
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

char *requestPeerIp(int fd, char *destinationUserId, struct sockaddr_in serverAddr, int slen)
{
    // send to server
    sendMsg(fd, makeMsg("P2P_CHAT", destinationUserId), serverAddr, slen);
    char *reply = receiveMsg(fd, serverAddr);
    char *replyType = getMsgType(strdup(reply));

    // peer ipAddr
    char *peerIpAddr = NULL;
    if (!strcmp(replyType, "#P2P_CHAT"))
        peerIpAddr = getMsgContent(strdup(reply));
    return peerIpAddr;
}