//richiesta studente
//mandare informazione a server universitario
//collegamento segreteria <-> studente, port = 1024
//collegamento segreteria <-> server universitario, port = 1025
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
typedef struct 
{
    unsigned int day;       //27-03-2020 la function atoi cambia la stringa in int
    unsigned int month;
    unsigned int year;    
}Date;

typedef struct{
   int ID;
   char nome[20];
   Date data;
   int numero_prenotati;
}Esame;

//funzione che si collega con il client
void ascolto(int connfd,int listenfd, char * buff,time_t ticks, struct sockaddr_in server)
{
  if ( bind(listenfd, (struct sockaddr *) &server, sizeof(server)) < 0 ) {
    perror("bind");
    exit(1);
  }

  if ( listen(listenfd, 1024) < 0 ) {
    perror("listen");
    exit(1);
  }

  printf("in attesa\n");
  if ( ( connfd = accept(listenfd, (struct sockaddr *) NULL, NULL) ) < 0 ) {
      perror("accept");
      exit(1);
  }
  printf("client accettato\n");
  ticks = time(NULL);
  snprintf(buff, sizeof(buff), "%.24s\r\n",ctime(&ticks)); //%.24s ctime(&ticks)
  if ( write(connfd, buff, strlen(buff)) != strlen(buff)) {
      perror("write");
      exit(1);
  }
  close(connfd);
  printf("socket chiuso\n");
}

//funzione che si collega con il server
void inviaInfo(struct sockaddr_in client, Esame tupla, int listenfd)
{
    //ora creiamo la socket che funge da client

  client.sin_family = AF_INET;
  client.sin_port   = htons(1025);
  client.sin_addr.s_addr = inet_addr("127.0.0.1");
  /*if (inet_pton(AF_INET,"127.0.0.1", &client.sin_addr) < 0) {
      fprintf(stderr,"inet_pton error for \n");
      exit (1);
    }
  */

  if ( bind(listenfd, (struct sockaddr *) &client, sizeof(client)) < 0 ) {
    perror("bind");
    exit(1);
  }
  if ( listen(listenfd, 1025) < 0 ) {
    perror("listen");
    exit(1);
  }
}


Esame inserimento_esame()
{

 //inserimento dei dati nella tupla (buffer) che verrà mandato al server universitario
  Esame tupla ;
  printf("Inserisci ID esame\n");
  scanf("%d",&tupla.ID);
  
  printf("Inserisci nome esame\n");
  scanf("%s",&tupla.nome);

  printf("inserisci giorno esame\n");
  scanf("%d",&tupla.data.day);
  printf("inserisci mese esame\n");
  scanf("%d",&tupla.data.month);
  printf("inserisci anno esame\n");
  scanf("%d",&tupla.data.year);

  tupla.numero_prenotati = 0;

  return tupla;
  
}

int main(int argc, char **argv)
{
  int          listenfd, connfd;
  struct sockaddr_in  server, client;
  char        buff[4096];
  time_t        ticks;  
  if ( ( listenfd = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ) {
    perror("socket");
    exit(1);
    }

  server.sin_family      = AF_INET;
  server.sin_port        = htons(1024);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  /////////////////////////////////////////////////////
  int scelta;
  while (1)
  {
    printf("Selezionare 1 per mandare nuovo esame a server\n inserire 2 per rimanere in attesa");
    scanf("%d", &scelta);
    switch(scelta)
    {
      case 1: 
        //funzione nuovo esame
        inviaInfo(client, tupla, listenfd);
        printf("fare funzione nuovo esame\n");
        break;
      
      case 2: 
        ascolto(connfd,listenfd, buff,ticks,server);
        break;

      default: 
        printf("errore: scelta errrata\n");
        break;
    }

  }

}

