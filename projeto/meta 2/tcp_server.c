/*******************************************************************************
 * SERVIDOR no porto 9000, à escuta de novos clientes.  Quando surgem
 * novos clientes os dados por eles enviados são lidos e descarregados no ecran.
 *******************************************************************************/
#include <sys/socket.h>
#include <arpa/inet.h>
#include <wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include "fileIO.h"
#include "structUser.h"

#define SERVER_PORT 9000
#define SERVER_IP "127.0.0.1"
#define BUF_SIZE 1024
#define DBPATH "userDB.txt"

void handleClient(int fd, struct sockaddr_in client_addr);
void sendMsg(int fd, char msg[]);
char *receiveMsg(int fd);
char *getMsgType(char *msg);
char *getMsgContent(char *msg);
char *makeMsg(char *type, char *content);
void cleanup(int fd);

int main()
{
  // SETUP
  int fd, client;
  struct sockaddr_in addr, client_addr;
  int client_addr_size;

  // preenchimento da struct
  bzero((void *)&addr, sizeof(addr)); // limpa memoria onde vai estar o addr - medida de precaução
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(SERVER_IP);
  addr.sin_port = htons(SERVER_PORT);

  // create socket fd
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    erro("main", "socket");

  // reutilizar sockets
  int yes = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
    erro("main", "setsockopt");

  // bind socket to addr
  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    erro("main", "bind");

  // listen on opened socket
  if (listen(fd, 5) < 0)
    erro("main", "listen");

  printf("Server listening...\n");
  while (1)
  {
    //clean finished child processes, avoiding zombies
    //must use WNOHANG or would block whenever a child process was working
    while (waitpid(-1, NULL, WNOHANG) > 0)
      ;

    //wait for new connection - espera bloqueante
    client = accept(fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size);

    if (client > 0)
    {
      if (fork() == 0)
      {
        close(fd);
        handleClient(client, client_addr);
        exit(0);
      }
      close(client);
    }
  }
  printf("Server exiting.\n");
  close(client);
  cleanup(fd);
  printf("Server exited.\n");
  return 0;
}

void handleClient(int fd, struct sockaddr_in client_addr)
{
  printf("[TCP] Client at %s:%d has established a connection!\n",
         inet_ntoa(client_addr.sin_addr),
         ntohs(client_addr.sin_port));

  while (1)
  {
    char *rcvMsg = receiveMsg(fd);
    char *msgType = getMsgType(strdup(rcvMsg));
    char *msgContent = getMsgContent(strdup(rcvMsg));

    /*
    printf("Msg: %s--\nType: %s--\nContent: %s--\n",
           rcvMsg,
           msgType,
           msgContent);
    */

    if (!strcmp(msgType, "LIST"))
    {
      char *userList = printUsers(DBPATH);
      sendMsg(fd, makeMsg("LIST", userList));
      free(userList);
    }
    else if (!strcmp(msgType, "ADD"))
    {
      User *u = stringToUser(msgContent, " ");
      if (addUserToFile(DBPATH, u))
        sendMsg(fd, makeMsg("SUCCESS", "User added to database."));
      else
        sendMsg(fd, makeMsg("ERROR", "Can't add requested user."));
      free(u);
    }
    else if (!strcmp(msgType, "DEL"))
    {
      User *u;
      if ((u = searchUserOnFile(DBPATH, msgContent)))
      {
        if (deleteUserFromFile(DBPATH, msgContent))
          sendMsg(fd, makeMsg("SUCCESS", "User removed from database."));
        else
          sendMsg(fd, makeMsg("SUCCESS", "Couldn't remove user from database."));
        free(u);
      }
      else
        sendMsg(fd, makeMsg("ERROR", "User doesn't exist."));
    }
    else if (!strcmp(msgType, "QUIT"))
    {
      break;
    }
    else
    {
      sendMsg(fd, makeMsg("ERROR", "Command doesn't exist."));
    }
  }
  cleanup(fd);
  printf("Client at %s:%d has left.\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
  exit(0);
}

void sendMsg(int fd, char *msg)
{
  if (write(fd, msg, 1 + strlen(msg)) == -1)
    erro("[TCP_Server] sendMsg", "write");
}

char *receiveMsg(int fd)
{
  char *buffer = (char *)malloc(BUF_SIZE);
  int nread;
  if ((nread = read(fd, buffer, BUF_SIZE)) == -1)
    erro("[TCP_Server] receiveMsg", "write");
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
  char *msg = (char *)malloc(BUF_SIZE);
  sprintf(msg, "%s %s", type, content);
  msg[strlen(msg)] = '\0';
  return msg;
}

void cleanup(int fd)
{
  /*
  Free stuff.
  */
  close(fd);
}