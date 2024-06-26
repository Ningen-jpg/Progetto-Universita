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
#include <errno.h>
/*void ricerca_esami(){
    char corso[20];
    printf("Per quale corso vuoi ricercare: ");
    scanf("%s",corso);
    printf("Hai chiesto alla segreteria se ci sono esami per %s",corso);
    printf("\nattendere prego..");
  
}*/

//la close del socket và fatta solo una volta che a studente vengono inviati i dati (le date di esame) che stava cercando
ssize_t FullWrite(int fd, const void *buf, size_t count)
{
	size_t nleft;
	ssize_t nwritten;
	nleft = count;
	while (nleft > 0) {

	/* repeat until no left */
	if ( (nwritten = write(fd, buf, nleft)) < 0) {
		if (errno == EINTR) { /* if interrupted by system call */
		continue;
		/* repeat the loop */
		} else {
		exit(nwritten); /* otherwise exit with error */
		}
	}

	nleft -= nwritten;
	/* set left to write */
	buf +=nwritten;
	/* set pointer */
	}

	return (nleft);
}
int creaSocket(int argc, char ** argv)
{
  int sockfd;
  struct sockaddr_in servaddr;

  if (argc != 2) {
    fprintf(stderr,"Devi inserire anche l'indirizzo: %s <IPaddress>\n",argv[0]);
    exit (1);
  }

  if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr,"errore creazione della socket\n");
    exit (1);
  }
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(1024); //MI STO CONNETTENDO A SEGRETERIA
  if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) < 0) {
    fprintf(stderr,"inet_pton error for %s\n", argv[1]);
    exit (1);
  }
  if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
    fprintf(stderr,"Errore di connessione\n");
    exit(1);
  }
  printf("Connessione avvenuta con la segreteria.\n");

  //ritorna il file descriptor della connessione avvenuta con server
  return sockfd;
  //ora siamo connessi con la segreteria 
}

void sendID(int fd, int argc, char **argv) {
  int id;
  printf("Inserire ID d'esame da cercare:\t");
  scanf("%d", &id);

  // Invia l'ID alla segreteria
  
  if (write(fd, &id, sizeof(id)) != sizeof(id)) {
    perror("write");
    exit(1);
  }
  printf("ID inviato\n");
}
void sendScelta(int fd, int * scelta)
{
  
  if(write(fd, scelta, sizeof(scelta))  != sizeof(scelta))
  {
    perror("Scelta non inviata\n");
    exit(1);
  }
}

void richiesta_prenotazione(){

}


int main(int argc, char **argv){

 // char buffer[INET6_ADDRSTRLEN];


  //switch-case da implementare per la scelta delle operazioni
 
  int scelta= 0;
  while (1)
  {
    printf("Selezionare 1 per ricercare se ci sono esami disponibili.\nInserire 2 per effettuare una prenotazione.\n");
    scanf("%d", &scelta);
    

    switch(scelta)
    {
      case 1:  //ricerca esami
      {
        
        
        int * choice = calloc(1, sizeof(int));
        *choice = scelta;
        int fd = creaSocket(argc, argv);
        sendScelta(fd,choice);
        sendID (fd,argc,argv);
        int num_righe;

        printf("sto per leggere il numero di righe..\n");
        if(read(fd,&num_righe,sizeof(num_righe))<0)
        {
          perror("errore: non è stato ricevuto il numero di righe\n");
          exit(1);
        }

        //alloco la matrice con num_righe
        char **tuple = (char **)calloc(num_righe, sizeof(char *));
        for (int i = 0; i < num_righe; i++)
        {
          tuple[i] = (char *)calloc(1024, sizeof(char));
        }

        printf("==================\n");

        printf("sto per leggere le tuple una ad una...\n");
        for(int i= 0; i < num_righe; i++)
        {
          if(read(fd,&tuple[i],1024)<0)
          {
            perror("errore: non sono state lette le tuple\n");
            exit(1);
          }
        }

        printf("ho letto tuple\n");
        printf("\n ==================\n");

        for(int i = 0; i< num_righe; i++)
        {
          printf("%s\n",tuple[i]);
        }

        
      } break;
      
      case 2: 
        richiesta_prenotazione(); //da implementare
        break;

      default: 
        printf("errore: scelta errrata\n");
        break;
    }
  }
 
 //fine test

}