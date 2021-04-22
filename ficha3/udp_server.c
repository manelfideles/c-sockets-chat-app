#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFLEN 512 // Tamanho do buffer
#define PORT 9876  // Porto para recepção das mensagens

void erro(char *s)
{
	perror(s);
	exit(1);
}

int main(void)
{
	struct sockaddr_in si_minha, si_outra;

	int s, recv_len, send_len;
	socklen_t slen = sizeof(si_outra);
	char buf[BUFLEN];

	// Cria um socket para recepção de pacotes UDP
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		erro("Erro na criação do socket");
	}

	// Preenchimento da socket address structure
	si_minha.sin_family = AF_INET;
	si_minha.sin_port = htons(PORT);
	si_minha.sin_addr.s_addr = htonl(INADDR_ANY);

	// Associa o socket à informação de endereço
	if (bind(s, (struct sockaddr *)&si_minha, sizeof(si_minha)) == -1)
		erro("Erro no bind");

	while (1)
	{
		printf("Server ready.\n");
		printf("Waiting for incoming messages...\n");
		// Espera recepção de mensagem (a espera é bloqueante)
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *)&si_outra, (socklen_t *)&slen)) == -1)
			erro("Erro no recvfrom");

		// Para ignorar o restante conteúdo (anterior do buffer)
		buf[recv_len] = '\0';

		int totalChars = 0;

		for (int i = 0; buf[i] != '\0'; i++)
			totalChars++;

		printf("Recebi uma mensagem do sistema com o endereço %s e o porto %d\n", inet_ntoa(si_outra.sin_addr), ntohs(si_outra.sin_port));
		printf("Conteúdo da mensagem: %s\n", buf);

		bzero((void *)&buf, sizeof(buf));
		char *rep = (char *)malloc(256 * sizeof(char));
		sprintf(rep, "The number of chars is %d\n", totalChars);
		memcpy(buf, rep, strlen(rep) + 1);

		if ((send_len = sendto(s, buf, BUFLEN, 0, (struct sockaddr *)&si_outra, slen)) == -1)
			erro("Erro no sendto");
	}

	// Fecha socket e termina programa
	close(s);
	return 0;
}
