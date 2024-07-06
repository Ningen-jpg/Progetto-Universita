/*
Lo studente rappresenta il nostro client in questa architettura.
Ogni volta che uno studente si collega alla segreteria, può scegliere di effettuare due operazioni:
- Può chiedere alla segreteria se ci sono esami disponibili per un corso.
- Inviare una richiesta di prenotazione di un esame alla segreteria.
*/

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

int creaSocket(int argc, char ** argv)
{
  int sockfd; // File descriptor per la creazione della socket
  struct sockaddr_in servaddr; // Dichiarazione della struttura per memorizzare l'indirizzo del server (IPv4)

  if (argc != 2) {
    fprintf(stderr,"Devi inserire anche l'indirizzo: %s <IPaddress>\n",argv[0]); // Controllo se ho inserito l'indirizzo di loopback
    exit (1);
  }

  if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {  // Creazione della socket
    fprintf(stderr,"errore creazione della socket\n");
    exit (1);
  }

  servaddr.sin_family = AF_INET; // Imposta la famiglia di indirizzi su IPv4
  servaddr.sin_port   = htons(1024); //MI STO CONNETTENDO A SEGRETERIA

  if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) < 0) { // Converte l'indirizzo IP da testo a binario e lo assegna a sin_addr; verifica se la conversione fallisce
    fprintf(stderr,"inet_pton error for %s\n", argv[1]);
    exit (1);
  }

  if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) { // Tenta di stabilire una connessione al server specificato (segreteria); verifica se la connessione fallisce
    fprintf(stderr,"Errore di connessione\n");
    exit(1);
  }

  // Ritorna il file descriptor della connessione avvenuta con server
  return sockfd;
  // Ora siamo connessi con la segreteria 
}

void sendID(int fd, int argc, char **argv) { // Funzione utilizzata per l'invio dell'ID dell'esame da cercare
  int id;
  printf("Inserire ID d'esame da cercare:\t");
  scanf("%d", &id);

  // Invia l'ID alla segreteria
  
  if (write(fd, &id, sizeof(id)) != sizeof(id)) { // Scrive i dati contenuti in 'id' nel file descriptor 'fd';
    perror("write");
    exit(1);
  }
  printf("ID inviato\n");
}

void sendScelta(int fd, int scelta) // Funzione utilizzata per l'invio della scelta relativo allo switch-case
{
  if(write(fd, &scelta, sizeof(scelta))  != sizeof(scelta))
  {
    perror("Scelta non inviata\n");
    exit(1);
  }
}

//scelto 1
int ricerca_esami(int fd, int scelta, int argc, char **argv)
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
    int c=1; // Variabile contatore per listare le tuple trovate
    for (int i = 0; i < num_righe; i++)
    {
      printf("%d° %s\n",c, tuple[i]);
      c++;
    }
  }
  else
  {
    printf("\nNon ci sono esami con questo ID.\n");
  }
  return num_righe;
}

//scelgo 2
void richiesta_prenotazione(int fd,int scelta, int argc,char **argv){
  int num_righe = ricerca_esami(fd, scelta, argc, argv);
  if (num_righe > 0)
  {
    printf("\nPer quale data vuoi prenotarti?: ");
    int sceltadata;
    scanf("%d", &sceltadata);
    sendScelta(fd, sceltadata);
    int numero_progressivo;
    if (read(fd, &numero_progressivo, sizeof(numero_progressivo)) < 0)
    {
      perror("errore: non è stato ricevuto il numero progressivo\n");
      exit(1);
    }
    printf("\nPrenotazione effettuata!\n");
    printf("\nIl tuo numero di prenotazione e' : %d\n", numero_progressivo);
  }
  else{
    printf("Non puoi prenotarti!\n\n");
  }
}


int main(int argc, char **argv)
{ 
  int scelta= 0;
  int fd; // File descriptor per la creazione della socket
  while (1)
  {
    printf("1) Ricerca esami disponibili\n2) Effettua una prenotazione\n3) Interrompi la comunicazione\nScegli: ");
    scanf("%d", &scelta);
    fd = creaSocket(argc, argv);
    switch(scelta)
    {
      case 1:  //ricerca esami
      {
        printf("Connessione avvenuta con la segreteria.\n");
        ricerca_esami(fd,scelta,argc,argv);
      }
      break;
      case 2:
        printf("Connessione avvenuta con la segreteria.\n");
        richiesta_prenotazione(fd,scelta,argc,argv);
        break;
      case 3: 
        printf("Lo studente interrompe la connessione con la segreteria.\n");
        sendScelta(fd,scelta);
        close(fd);
        exit(0);
      default: 
        printf("errore: scelta errrata\n");
        break;
    }
  }
}