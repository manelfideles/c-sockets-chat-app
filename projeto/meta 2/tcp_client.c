/**********************************************************************
 * CLIENT connects to Server (argv[1]) on port argv[2].
 * USAGE: > client <ServerAddr> <Port>
 **********************************************************************/
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "menus.h"
#include "fileIO.h"

#define BUF_SIZE 1024

void sendMsg(int fd, char *buffer);
char *receiveMsg(int fd);
char *getMsgType(char *msg);
char *getMsgContent(char *msg);
char *makeMsg(char *type, char *content);

int main(int argc, char *argv[])
{
  char endServer[100];
  int fd;
  struct sockaddr_in addr;
  struct hostent *hostPtr;

  if (argc != 3)
  {
    printf("Usage: Client <ServerAddr> <Port>\n");
    exit(-1);
  }

  // SETUP
  strcpy(endServer, argv[1]);
  if ((hostPtr = gethostbyname(endServer)) == 0)
    erro("main", "gethostname");

  bzero((void *)&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
  addr.sin_port = htons((short)atoi(argv[2]));

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    erro("main", "socket");
  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    erro("main", "connect");

  // OPERATIONS
  printf("Connected to server at %s:%d\n",
         inet_ntoa(addr.sin_addr),
         ntohs(addr.sin_port));

  while (1)
  {
    adminMenu();
    char *comando = getTextField();
    //printf("Comando: %s--\n", comando);
    sendMsg(fd, comando);

    if (!strcmp(comando, "QUIT"))
      break;

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
      printf("User List: \n");
      printf("%s\n", msgContent);
    }
    else if (!strcmp(msgType, "SUCCESS"))
      printf("%s\n", msgContent);
    else if (!strcmp(msgType, "ERROR"))
      continue;
    else
      sendMsg(fd, makeMsg("ERROR", "Reply not recognized!"));
  }
  close(fd);
  exit(0);
}

void sendMsg(int fd, char *msg)
{
  if (write(fd, msg, 1 + strlen(msg)) == -1)
    erro("[TCP_Client] sendMsg", "write");
}

char *receiveMsg(int fd)
{
  char *buffer = (char *)malloc(BUF_SIZE);
  int nread;
  if ((nread = read(fd, buffer, BUF_SIZE)) == -1)
    erro("[TCP_Client] receiveMsg", "write");
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
  char *msg = (char *)malloc(128);
  sprintf(msg, "%s %s", type, content);
  return msg;
}
