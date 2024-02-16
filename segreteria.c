
//richiesta studente
//mandare informazione a server universitario

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


void ascolto(int connfd,int listenfd, char * buff,time_t ticks)
{
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

void inserimento_esame()
{
  /*
    1. creare puntatore file

  */

}
int main(int argc, char **argv)
{
  int          listenfd, connfd;
  struct sockaddr_in  servaddr;
  char        buff[4096];
  time_t        ticks;  
  if ( ( listenfd = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ) {
    perror("socket");
    exit(1);
    }

  servaddr.sin_family      = AF_INET;
  servaddr.sin_port        = htons(1024);
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  if ( bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
    perror("bind");
    exit(1);
  }
  if ( listen(listenfd, 1024) < 0 ) {
    perror("listen");
    exit(1);
  }

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
        printf("fare funzione nuovo esame\n");
        break;
      
      case 2: 
        ascolto(connfd,listenfd, buff,ticks);
        break;

      default: 
        printf("errore: scelta errrata\n");
        break;
    }

  }

}

