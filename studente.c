//Chiede alla segreteria se ci siano esami disponibili un corso
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

//la close del socket và fatta solo una volta che a studente vengono inviati i dati (le date di esame) che stava cercando

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

void sendScelta(int fd, int scelta)
{
  if(write(fd, &scelta, sizeof(scelta))  != sizeof(scelta))
  {
    perror("Scelta non inviata\n");
    exit(1);
  }
}

void ricerca_esami(int fd, int scelta, int argc, char **argv)
{
  sendScelta(fd, scelta);
  sendID(fd, argc, argv);
  int num_righe;

  // printf("sto per leggere il numero di righe..\n");
  if (read(fd, &num_righe, sizeof(num_righe)) < 0)
  {
    perror("errore: non è stato ricevuto il numero di righe\n");
    exit(1);
  }
  printf("il num di righe arrivate e' : %d\n", num_righe);
  if (num_righe > 0)
  {
    // matrice per le eventuali tuple trovate

    char tuple[num_righe][1024];
    printf("==================\n");

    // printf("sto per leggere le tuple una ad una...\n");
    for (int i = 0; i < num_righe; i++)
    {
      if (read(fd, &tuple[i], sizeof(tuple[i])) < 0)
      {
        perror("errore: non sono state lette le tuple\n");
        exit(1);
      }
    }

    printf("Tuple trovate:\n");
    int c=1;
    for (int i = 0; i < num_righe; i++)
    {
      printf("%d° %s\n",c, tuple[i]);
      c++;
    }
  }
  else
  {
    printf("Non ci sono esami con questo ID.\n");
  }
}

void richiesta_prenotazione(int fd,int scelta, int argc,char **argv){
  ricerca_esami(fd,scelta,argc,argv);
  printf("\nPer quale data vuoi prenotarti?: ");
  int sceltadata;
  scanf("%d",&sceltadata);
  sendScelta(fd,sceltadata);
  printf("sono dopo sendScelta\n");
  int numero_progressivo;
  if (read(fd, &numero_progressivo, sizeof(numero_progressivo)) < 0)
  {
    perror("errore: non è stato ricevuto il numero progressivo\n");
    exit(1);
  }
  printf("Il tuo numero di prenotazione e' : %d\n", numero_progressivo);
}


int main(int argc, char **argv){
  
  // char buffer[INET6_ADDRSTRLEN];
 
  int scelta= 0;
  int fd;
  while (1)
  {
    printf("1) Ricerca esami disponibili\n2) Effettua una prenotazione\n3) Interrompi la comunicazione\nScegli: ");
    scanf("%d", &scelta);
    fd = creaSocket(argc, argv);
    switch(scelta)
    {
      case 1:  //ricerca esami
      {
        ricerca_esami(fd,scelta,argc,argv);
      } break;
      
      case 2: 
        richiesta_prenotazione(fd,scelta,argc,argv);
        break;
      case 3: 
        //lo studente interrompe la connessione con la segreteria
        sendScelta(fd,scelta);
        close(fd);
        exit(0);
      default: 
        printf("errore: scelta errrata\n");
        break;
    }
  }
 
 //fine test

}