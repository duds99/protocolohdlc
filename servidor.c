#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <time.h>
#include <string.h>

#define B_SIZE 256
#define TRUE 1
#define FALSE 0

//função para comparar strings

const char* SABME = "SABME";
const char* UA = "UA";
const char* DISC = "DISC";


int sockfd, newsockfd, portno, clilen;
struct sockaddr_in serv_addr, cli_addr;
int n, m, pid;
int NR = 0, NS = 0;
int state = 1;
char nsNrString[B_SIZE];
char nrString[B_SIZE];
char* buffer_recebe;
char buffer_envia[B_SIZE];

int inteiro(char c) {
	return c - '0';
}

char* i(int ns, int nr) {
	buffer_envia[0] = 0;
	sprintf(nsNrString, "%d,%d", ns, nr);
	strcat(buffer_envia, "I,");
	strcat(buffer_envia, nsNrString);
	
	return buffer_envia;
}

char* rr(int nr, char* pf) {
	buffer_envia[0] = 0;
	sprintf(nrString, "%d", nr);
	strcat(buffer_envia, "RR,");
	strcat(buffer_envia, nrString);
	strcat(buffer_envia, pf);

	return buffer_envia;
}

char* rnr(int nr, char* pf) {
	buffer_envia[0] = 0;
	sprintf(nrString, "%d", nr);
	strcat(buffer_envia, "RNR,");
	strcat(buffer_envia, nrString);
	strcat(buffer_envia, pf);
	return buffer_envia;
}

void envia(const char * mensagem, int st){

	int r = rand()%100;

	if (r>10) {
		n = write(newsockfd,mensagem,B_SIZE);
	   		
	   	if(n < 0){
	   		perror("ERROR ao escrever socket");
			exit(1);		
		}
		printf("Servidor enviando: %s\n", mensagem);
		state = st;
	} else {
		printf("Servidor enviando: %s\n", mensagem);
		for (int j=0;j<5;j++) {
			printf(".");
			sleep(1);
		}
		printf(".\n");
		printf("---Timeout...---\n");
		if (startsWith(mensagem, DISC)) {
			state = 4;
		}
	}
}

void recebe(){

	n = read(newsockfd,buffer_recebe,B_SIZE);
    
    if (n < 0){
		perror("ERROR ao ler do socket");
		exit(1);
	}
	
	printf("Recebido: %s\n",buffer_recebe);

}

int startsWith(const char *str, const char *pre){
    size_t lenpre = strlen(pre),lenstr = strlen(str);
    return lenstr < lenpre ? FALSE : strncmp(pre, str, lenpre) == 0;
}

int main( int argc, char *argv[] ) {

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	buffer_recebe = malloc(sizeof(char)*B_SIZE);

	srand(time(NULL));

	if (sockfd < 0) {
		perror("ERROR na criação socket");
		exit(1);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = 5031;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR no binding");
		exit(1);
	}

	listen(sockfd,2);
	clilen = sizeof(cli_addr); 
    printf("Servidor funcionando...........................\n        esperando conexão na porta estabelecida: %d\n",portno);

    newsockfd 	= accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		
	if(newsockfd<0){  
		perror("Error ao aceitar");  
	}else{
		printf("Conexão client aceita...\n");
	}

	int connected = TRUE;
	int c;
	char* msg;
	while (connected){	

		switch (state) {
			case 1:
				envia(SABME, 2);
			break;
			case 2:
				recebe();
				state = 3;
			break;
			case 3:
				c = rand()%100;
				if (c>5) {
					msg = i(NS,NR);
					envia(msg, 5);
				} else {
					envia(DISC, 12);
				}
			break;
			case 4:
				envia(DISC, 12);
			break;
			case 5:
				recebe();
				if (startsWith(buffer_recebe, DISC)) {
					state = 6;
				} else if (startsWith(buffer_recebe, "RNR")) {
					NS = inteiro(buffer_recebe[4]);
					state = 7;
				} else if (startsWith(buffer_recebe, "RR")) {
					NS = inteiro(buffer_recebe[3]);
					state = 9;
				} else if (startsWith(buffer_recebe, "I,")) {
					NR = (inteiro(buffer_recebe[2])+1)%10;
					NS = inteiro(buffer_recebe[4]);
					state = 9;
				}
			break;
			case 6:
				envia(UA, 13);
			break;
			case 7:
				msg = rr(NR,",P");
				envia(msg, 8);
			break;
			case 8:
				recebe();
				if (startsWith(buffer_recebe, "RNR")) {
					NS = inteiro(buffer_recebe[4]);
					state = 7;
				} else {
					NS = inteiro(buffer_recebe[3]);
					state = 9;
				}
			break;
			case 9:
				c = rand()%100;
				if (c<5) {
					envia(DISC,12);
				} else if (c<20) {
					msg = rnr(NR, "");
					envia(msg, 10);
				} else if (c<35) {
					msg = rr(NR, "");
					envia(msg, 5);
				} else {
					msg = i(NS,NR);
					envia(msg, 5);
				}
			break;
			case 10:
				recebe();
				NS = inteiro(buffer_recebe[3]);
				state = 11;
			break;
			case 11:
				c = rand()%100;
				if (c<30) {
					msg = rnr(NR,",F");
					envia(msg, 10);
				} else {
					msg = rr(NR,",F");
					envia(msg, 5);
				}
			break;
			case 12:
				recebe();
				state = 13;
			break;
			case 13:
				connected = FALSE;
			break;
		}

	} 

	shutdown(newsockfd, 1);
	printf("Servidor encerrando conexão.");

}

