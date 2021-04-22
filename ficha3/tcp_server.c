/*******************************************************************************
 * SERVIDOR no porto 9000, à escuta de novos clientes.  Quando surgem
 * novos clientes os dados por eles enviados são lidos e descarregados no ecran.
 *******************************************************************************/
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <signal.h>

#define SERVER_PORT 9000
#define BUF_SIZE 1024

void handleClient(int fd);
void sendMsg(int fd, char msg[]);
char *receiveMsg(int fd);
int execDados(int fd, int dados[], int n);
int execSoma(int dados[], int n);
float execMedia(int dados[], int n);
int isNumber(char s[]);
void erro(char *msg);
void raiseSigint();
void sigintHandler(int fd, int *dados, char *buffer);
void printNumbers(int *dados);

int clientCount = 0;
int sigint = 0;

int main()
{
  // SETUP
  int fd, client;
  struct sockaddr_in addr, client_addr;
  int client_addr_size;

  // preenchimento da struct
  bzero((void *)&addr, sizeof(addr)); // limpa memoria onde vai estar o addr - medida de precaução
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(SERVER_PORT);

  // criação da socket e bind da mesma a um addr
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    erro("na funcao socket");

  // reutilizar sockets
  // int yes = 1;
  // if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
  //   erro("na funcao setsockopt");

  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    erro("na funcao bind");

  // escuta na socket definida
  if (listen(fd, 5) < 0)
    erro("na funcao listen");

  printf("Server listening...\n");
  while (1)
  {
    //clean finished child processes, avoiding zombies
    //must use WNOHANG or would block whenever a child process was working
    while (waitpid(-1, NULL, WNOHANG) > 0)
      ;

    //wait for new connection - espera bloqueante
    client = accept(fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size);
    clientCount++;

    if (client > 0)
    {
      if (fork() == 0)
      {
        handleClient(client);
        close(fd);
        exit(0);
      }
      close(client);
    }
  }
  return 0;
}

void handleClient(int fd)
{
  char *buffer = NULL;
  int n = 10;
  int *dados = (int *)malloc(n * sizeof(int));
  int isEmpty = 1;

  signal(SIGINT, raiseSigint);
  while (!sigint)
  {
    buffer = receiveMsg(fd);
    if (strcmp(buffer, "DADOS") == 0)
    {
      if ((isEmpty = execDados(fd, dados, n)) == 1)
      {
        memset(dados, 0, sizeof(int) * n);
        sendMsg(fd, "Erro: Input invalido.\n");
      }
    }
    else if (strcmp(buffer, "SOMA") == 0)
    {
      if (!isEmpty)
      {
        sprintf(buffer, "%d\n", execSoma(dados, n));
        sendMsg(fd, buffer);
        //buffer[0] = '\0';
      }
      else
        sendMsg(fd, "Erro: Insira dados.\n");
    }
    else if (strcmp(buffer, "MEDIA") == 0)
    {
      if (!isEmpty)
      {
        sprintf(buffer, "%.2f\n", execMedia(dados, n));
        sendMsg(fd, buffer);
        //buffer[0] = '\0';
      }
      else
        sendMsg(fd, "Erro: Insira dados.\n");
    }
    else if (strcmp(buffer, "SAIR") == 0)
    {
      sendMsg(fd, "SAIR\n");
      free(dados);
      free(buffer);
      break;
    }
    else
      sendMsg(fd, "Erro: Comando nao existe.\n");
  }
  printf("Quitting the program...\n");
  sigintHandler(fd, dados, buffer);
}

void sendMsg(int fd, char msg[])
{
  if (write(fd, msg, 1 + strlen(msg)) == -1)
    erro("na funcao write");
}

char *receiveMsg(int fd)
{
  char *buffer = (char *)malloc(BUF_SIZE);
  int nread = read(fd, buffer, BUF_SIZE - 1);
  buffer[nread] = '\0';

  printf("Received: %s\n", buffer);
  fflush(stdout);
  return buffer;
}

int execDados(int fd, int *dados, int n)
{
  int i = 0;
  char buf[32];

  while (i < n)
  {
    sprintf(buf, "Insira o numero %d:\n", i + 1);
    sendMsg(fd, buf);
    char *reply = receiveMsg(fd);
    if (isNumber(reply))
      dados[i] = atoi(reply);
    else
      return 1;
    i++;
  }
  printf("DADOS: ");
  printNumbers(dados);
  return 0;
}

int execSoma(int dados[], int n)
{
  int sum = 0;
  for (int i = 0; i < n; i++)
    sum += dados[i];
  return sum;
}

void printNumbers(int *dados)
{
  printf("[");
  while (dados)
    printf("%d", *(dados++));
  printf("]\n");
}

float execMedia(int dados[], int n)
{
  return execSoma(dados, n) / n;
}

int isNumber(char s[])
{
  for (int i = 0; i < strlen(s); i++)
    if (!isdigit(s[i]))
      return 0;
  return 1;
}

void erro(char *msg)
{
  printf("Erro: %s\n", msg);
  exit(-1);
}

void raiseSigint()
{
  sigint = 1;
}

void sigintHandler(int fd, int *dados, char *buffer)
{
  if (dados)
    free(dados);
  if (buffer)
    free(buffer);
  close(fd);
  exit(0);
  //printf("Program terminated.\n");
}