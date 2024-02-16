//Chiede alla segreteria se ci siano esami disponibili per un corso
//Invia una richiesta di prenotazione di un esame alla segreteria

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

void ricerca_esami(){
    char corso[20];
    printf("Per quale corso vuoi ricercare: ");
    scanf("%s",&corso);
    printf("Lo studente chiede alla segreteria se ci sono esami disponibili per ",corso);
    

}

void richiesta_prenotazione(){

}


int main(int argc, char **argv){
    int                sockfd, n;
    char               recvline[1025];
    struct sockaddr_in servaddr;
    char buffer[INET6_ADDRSTRLEN];

    if (argc != 2) {
      fprintf(stderr,"Devi inserire anche l'indirizzo: %s <IPaddress>\n",argv[0]);
      exit (1);
    }
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      fprintf(stderr,"errore creazione della socket\n");
      exit (1);
    }
      servaddr.sin_family = AF_INET;
      servaddr.sin_port   = htons(1024);
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) < 0) {
        fprintf(stderr,"inet_pton error for %s\n", argv[1]);
        exit (1);
      }
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
      fprintf(stderr,"Errore di connessione\n");
      exit(1);
    }

    printf("Connessione avvenuta con la segreteria.");
    //switch-case da implementare per la scelta delle operazioni
    while(1){
        switch(x):
            case1:
                ricerca_esami()
            case2:
                richiesta_prenotazione()

    }
 
}