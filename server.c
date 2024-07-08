/*
Il server viene gestito dalla segreteria, attraverso il quale gestiamo gli esami universitari.
Le operazioni che svolge il server sono:
- Aggiunge nuovi esami, salvati in un file csv
- Effettua le prenotazioni degli esami per gli studenti. 
  Ad ogni richiesta di prenotazione viene inviato il relativo numero di prenotazione progressivo.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

//nuova, cambia solo che c'è la setsockopt in più, il resto è identico
void crea_connessione(int * connectFD)
{
    int listenFD; // File descriptor per la connessione
    struct sockaddr_in server; // sarà utilizzata per memorizzare l'indirizzo del server.

    //creiamo la socket
    if ( ( listenFD = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ) {
        perror("socket");
        exit(1);
    }

    // Abilita l'opzione SO_REUSEADDR
    if (setsockopt(listenFD, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
        perror("setsockopt");
        exit(1);
    }
    // Imposta l'opzione SO_REUSEPORT sul socket listenFD.
    // SO_REUSEPORT consente di riutilizzare immediatamente un indirizzo/porta locale, anche se è già in uso.
    // &(int){ 1 } è una sintassi C che crea un valore intero temporaneo pari a 1 e ne prende l'indirizzo.
    // sizeof(int) indica la dimensione del dato da passare a setsockopt, che in questo caso è un intero.

    server.sin_family      = AF_INET;
    server.sin_port        = htons(1025); // 1025 porta server
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    // Imposta l'indirizzo IP nella struttura 'server' per accettare connessioni su tutte le interfacce di rete disponibili.
    // htonl() converte il valore da ordine di host a ordine di rete, se necessario.
    // INADDR_ANY è un valore speciale che indica che il server è disposto ad accettare connessioni su qualsiasi indirizzo IP associato alla macchina.

    if ( bind(listenFD, (struct sockaddr *) &server, sizeof(server)) < 0 ) {
        perror("bind");
        exit(1);
    }

    //connessione con segreteria
    if ( listen(listenFD, 1024) < 0 ) {
        perror("listen");
        exit(1);
    }
    printf("connessione in corso..\n");

    *connectFD = accept(listenFD, NULL, NULL);
    // printf("ho accettato con accept\n");

    close(listenFD);
}

int get_key(int connectFD)
{
    int key;
    if(read(connectFD, &key, sizeof(key))<0)
    {
        perror("get_key andata male");
        exit(-1);
    }
    return key;
}

void ricerca_esami(int connectFD)
{
    char buffer[1024];
    printf("\n==============================\n");
    FILE *esami = fopen("esami.csv", "r");
    if (esami == NULL)
    {
        printf("Errore: il file non è stato aperto correttamente.\n");
        exit(-1);
    }

    // leggiamo la prima riga
    fgets(buffer, sizeof(buffer), esami);
    printf("%s\n", buffer);

    //crea_connessione(&connectFD);
    int key = get_key(connectFD);

    // inizializziamo matrice
    int i = 0;
    int count = 0; // per scorrere le righe della matrice, indica anche il num di tuple trovate
    int chiave;    // per ottenere il valore intero con atoi

    char matrice[50][1024]; // Matrice per le tuple

    while (fgets(buffer, sizeof(buffer), esami))
    {
        char buff_temp[1024];
        strcpy(buff_temp, buffer);

        chiave = atoi(strtok(buffer, ",")); // funzione strtok per la divisione della stringa in token da utilizzare per le operazioni di ricerca
        // atoi converte da char ad integer
        if (chiave == key)
        {
            printf("%d° riga trovata :\n%s\n", count + 1, buff_temp);
            strcpy(matrice[count], buff_temp);
            count++;
        }
    }

    fclose(esami);

    if (count > 0) // verifico se sono state trovate delle tuple
    {
        // invia num righe
        if (write(connectFD, &count, sizeof(int)) < 0)
        {
            perror("errore, non sono state inviate il num di righe\n");
            exit(1);
        }

        // invia tuple a segreteria
        int bytesc = 0; // debug
        for (int i = 0; i < count; i++)
        {
            if ((bytesc = write(connectFD, matrice[i], sizeof(matrice[i]))) < 0)
            {
                perror("write");
                exit(1);
            }
        }
    }
    else
    {
        // invia num righe
        if (write(connectFD, &count, sizeof(int)) < 0)
        {
            perror("errore, non sono state inviate il num di righe\n");
            exit(1);
        }
    }
}

void richiesta_prenotazione(int connectFD){
    char buffer[1024];
    printf("\n==============================\n");
    FILE *esami = fopen("esami.csv", "r");
    if (esami == NULL)
    {
        printf("Error: Could not open the file\n");
        exit(-1);
    }
    printf("File aperto\n");

    // leggiamo la prima riga
    fgets(buffer, sizeof(buffer), esami);
    printf("%s\n", buffer);

    //crea_connessione(&connectFD);
    int key = get_key(connectFD);

    // inizializziamo matrice
    int i = 0;
    int count = 0; // per scorrere le righe della matrice, indica anche il num di tuple trovate
    int chiave;    // per ottenere il valore intero con atoi

    char matrice[50][1024];

    while (fgets(buffer, sizeof(buffer), esami))
    {
        char buff_temp[1024]; //potrebbe dover essere messa fuori dall'while
        strcpy(buff_temp, buffer);
        chiave = atoi(strtok(buffer, ","));
        if (chiave == key)
        {
            printf("%d° riga trovata :\n%s\n", count + 1, buff_temp);
            strcpy(matrice[count], buff_temp);
            count++;
        }
    }
    
    if (count > 0)
    {
        // invia num righe
        if (write(connectFD, &count, sizeof(int)) < 0)
        {
            perror("errore, non sono state inviate il num di righe\n");
            exit(1);
        }

        // invia tuple a segreteria
        int bytesc = 0; //debug
        for (int i = 0; i < count; i++)
        {
            if ((bytesc = write(connectFD, matrice[i], sizeof(matrice[i]))) < 0)
            {
                perror("write");
                exit(1);
            }
        }

        // Il server riceve la scelta della data, che servirà per modificare la tupla corrispondente
        int sceltaData = 0;
        
        int byteletti = 0; //debug

        if ((byteletti= read(connectFD, &sceltaData, sizeof(sceltaData))) < 0)
        {
            perror("non è stata correttamente letta la scelta della data");
            exit(-1);
        }
        fclose(esami);
        // ricevuta la scelta data
        FILE *esami = fopen("esami.csv", "r+");
        if (esami == NULL)
        {
            printf("Error: Could not open the file\n");
            exit(-1);
        }
        printf("File aperto in lettura scrittura\n");
        sceltaData--;

        char *campo = strrchr(matrice[sceltaData],',');
        int prenotazione = atoi(campo + 1);
        prenotazione++;
        sprintf(campo+1,"%d",prenotazione); // modifica fatta al numero prenotati

        char bufferMatrice[1024];
        strcpy(bufferMatrice,matrice[sceltaData]);
        
        char *id;
        id = strtok(bufferMatrice,","); // recuperiamo l'ID
        char *date;
        date = strtok(NULL,",");
        date = strtok(NULL,","); // recuperiamo la data
        
        while (fgets(buffer, sizeof(buffer), esami))
        {
            if((strstr(buffer,id))!= NULL && (strstr(buffer,date))!= NULL)
            {
                fseek(esami,-strlen(buffer),SEEK_CUR);
                fputs(matrice[sceltaData],esami);
                printf("modifica effettuata\n");
            }
        }
        if (write(connectFD, &prenotazione, sizeof(prenotazione)) < 0)
        {
            perror("errore, non è stato inviata la prenotazione\n");
            exit(1);
        }
    }
    else
    {
        // invia num righe
        if (write(connectFD, &count, sizeof(int)) < 0)
        {
            perror("errore, non sono state inviate il num di righe\n");
            exit(1);
        }
    }
    fclose(esami);
}

void aggiunta_esame(int connectFD){
    char stringa[1024] = "";
    if (read(connectFD, &stringa, sizeof(stringa)) < 0)
    {
        perror("errore: non e' stata letta la stringa concatenata");
        exit(1);
    }

    FILE *esami = fopen("esami.csv", "a");
    if (esami == NULL)
    {
        printf("Errore: il file non è stato aperto correttamente.\n");
        exit(-1);
    }
    printf("File aperto\n");
    fseek(esami, 0, SEEK_END);
    fputs("\n", esami);
    fputs(stringa, esami);
    fclose(esami);
    char messaggio[16] = "Esame aggiunto!";
    if (write(connectFD, &messaggio, sizeof(messaggio)) < 0)
    {
        perror("errore: non e' stato inviato il messaggio");
        exit(1);
    }
    printf("\nEsame aggiunto\n");
}

int main(int argc, char **argv)
{ 
    int serverfd;
    char * data;
    int connectFD;
    int scelta = 0;
    
    while(1){
        crea_connessione(&connectFD);
        if(read(connectFD,&scelta, sizeof(scelta))<0)
        {
            perror("read non andata bene");
            exit(-1);
        }

        switch(scelta)
        {
            case 1:
            {
                ricerca_esami(connectFD);
            }
            break;
            case 2:
            {
                richiesta_prenotazione(connectFD);
            }
            break;
            case 3:
            {
                aggiunta_esame(connectFD);
            }
            break;
            default:
                printf("switch non andata bene\n"); 
        }
        close(connectFD);
    }
    printf("\n==============================\n");
}