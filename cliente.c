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

const char* SABME = "SABME";
const char* UA = "UA";
const char* DISC = "DISC";

int sockfd, portno, n, m, o, p;
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
		n = write(sockfd,mensagem,B_SIZE);
	   		
	   	if(n < 0){
	   		perror("ERROR ao escrever socket");
			exit(1);		
		}
		printf("Cliente enviando: %s\n", mensagem);
		state = st;
	} else {
		printf("Cliente enviando: %s\n", mensagem);
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

	n = read(sockfd,buffer_recebe,B_SIZE);
    
    if (n < 0){
		perror("ERROR ao ler do socket");
		exit(1);
	}
	
	printf("Recebido: %s\n",buffer_recebe);

}

int startsWith(const char *str, const char *pre) {
    size_t lenpre = strlen(pre),lenstr = strlen(str);
    return lenstr < lenpre ? FALSE : strncmp(pre, str, lenpre) == 0;
}

int main(int argc, char *argv[]) {
	
	struct sockaddr_in serv_addr;
	struct hostent *server;

	buffer_recebe = malloc(sizeof(char)*B_SIZE);

	srand(time(NULL));

   if (argc < 3) {
      fprintf(stderr,"usage %s hostname port\n", argv[0]);
      exit(0);
   }
	
   portno = atoi(argv[2]);

   
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR ao abrir o socket");
      exit(1);
   }
	
   server = gethostbyname(argv[1]);
   
   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }
   
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);
   
   
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR de conexão");
      exit(1);
   }
   
   	int connected = TRUE;
	int c;
	char* msg;
   	while(connected){
   		switch (state) {
			case 1:
				recebe();
				state = 2;
			break;
			case 2:
				envia(UA, 3);
			break;
			case 3:
				recebe();
				if (startsWith(buffer_recebe, DISC)) {
					state = 12;
				} else {
					NR = (inteiro(buffer_recebe[2])+1)%10;
					NS = inteiro(buffer_recebe[4]);
					state = 5;
				}
			break;
			case 4:
				recebe();
				state = 12;
			break;
			case 5:
				c = rand()%100;
				if (c<5) {
					envia(DISC, 6);
				} else if (c<20) {
					msg = rnr(NR,"");
					envia(msg, 7);
				} else if (c<35) {
					msg = rr(NR,"");
					envia(msg, 9);
				} else {
					msg = i(NS,NR);
					envia(msg, 9);
				}
			break;
			case 6:
				recebe();
				state = 13;
			break;
			case 7:
				recebe();
				state = 8;
			break;
			case 8:
				c = rand()%100;
				if (c<30) {
					msg = rnr(NR,",F");
					envia(msg, 7);
				} else {
					msg = rr(NR,",F");
					envia(msg, 9);
				}
			break;
			case 9:
				recebe();
				if (startsWith(buffer_recebe, "I,")) {
					NR = (inteiro(buffer_recebe[2])+1)%10;
					NS = inteiro(buffer_recebe[4]);
					state = 5;
				} else if (startsWith(buffer_recebe, "RR,")) {
					NS = inteiro(buffer_recebe[3]);
					state = 5;
				} else if (startsWith(buffer_recebe, "RNR,")) {
					NS = inteiro(buffer_recebe[4]);
					state = 10;
				} else {
					state = 12;
				}
			break;
			case 10:
				msg = rr(NR,",P");
				envia(msg, 11);
			break;
			case 11:
				recebe();
				if (startsWith(buffer_recebe, "RNR,")) {
					NS = inteiro(buffer_recebe[4]);
					state = 10;
				} else {
					NS = inteiro(buffer_recebe[3]);
					state = 5;
				}
			break;
			case 12:
				envia(UA, 13);
			break;
			case 13:
				connected = FALSE;
			break;
   		}
   	}

   	shutdown(sockfd, 1);
	printf("Cliente encerrando conexão.");

    return 0;
}