/**********************************************************************
 * CLIENTE liga ao servidor (definido em argv[1]) no porto especificado
 * (em argv[2]), escrevendo a palavra predefinida (em argv[3]).
 * USO: >cliente <enderecoServidor>  <porto>  <Palavra>
 **********************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>

#define BUF_SIZE 1024

void erro(char *msg);
void sendMsg(int fd, char buffer[]);
void receiveMsg(int fd);
void erro(char *msg);

int main(int argc, char *argv[])
{
  char endServer[100];
  char buffer[BUF_SIZE];
  int fd;
  struct sockaddr_in addr;
  struct hostent *hostPtr;

  if (argc != 3)
  {
    printf("Usage: cliente <host> <port>\n");
    exit(-1);
  }

  strcpy(endServer, argv[1]);
  if ((hostPtr = gethostbyname(endServer)) == 0)
    erro("Não consegui obter endereço");

  bzero((void *)&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
  addr.sin_port = htons((short)atoi(argv[2]));

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    erro("Socket");
  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    erro("Connect");

  while (1)
  {
    printf("Insira um comando: ");
    char comando[32];
    if (fgets(comando, sizeof(comando), stdin))
    {
      comando[strcspn(comando, "\r\n")] = 0;
      sendMsg(fd, comando);
      printf("> %s\n", comando);
    }
    else
      erro("fgets");

    receiveMsg(fd);
    if (strcmp(comando, "DADOS") == 0)
    {
      int i = 0;
      while (i < 10)
      {
        scanf("%s", buffer);
        fflush(stdin);
        sendMsg(fd, buffer);
        if (i == 9)
          break;
        else
        {
          printf("Waiting for reply.\n");
          receiveMsg(fd);
        }
        i++;
      }
    }
  }
  close(fd);
  exit(0);
}

void sendMsg(int fd, char msg[])
{
  if (write(fd, msg, 1 + strlen(msg)) == -1)
    erro("na funcao sendMsg");
}

void receiveMsg(int fd)
{
  char buffer[BUF_SIZE];
  int nread;
  if ((nread = read(fd, buffer, BUF_SIZE - 1)) == -1)
    erro("na funcao receiveMsg");
  buffer[nread] = '\0';
  printf("%s", buffer);
  //fflush(stdout);
}

void erro(char *msg)
{
  printf("Erro: %s\n", msg);
  exit(-1);
}