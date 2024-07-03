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
    int listenFD;
    struct sockaddr_in server;

    //creiamo la socket
    if ( ( listenFD = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ) {
        perror("socket");
        exit(1);
    }

    // Abilita l'opzione SO_REUSEADDR
    int opt = 1;
    if (setsockopt(listenFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    server.sin_family      = AF_INET;
    server.sin_port        = htons(1025); //1025 porta server
    server.sin_addr.s_addr = htonl(INADDR_ANY);

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
    //printf("il fd e : %d\n", connectFD);
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
        printf("Error: Could not open the file\n");
        exit(-1);
    }
    printf("File opened\n");

    // leggiamo la prima riga
    fgets(buffer, sizeof(buffer), esami);
    printf("%s\n", buffer);

    //crea_connessione(&connectFD);
    int key = get_key(connectFD);

    printf("inizia il while qui\n");

    // inizializziamo matrice
    int i = 0;
    int count = 0; // per scorrere le righe della matrice, indica anche il num di tuple trovate
    int chiave;    // per ottenere il valore intero con atoi

    char matrice[10][1024];

    while (fgets(buffer, sizeof(buffer), esami))
    {
        char buff_temp[1024];
        strcpy(buff_temp, buffer);

        chiave = atoi(strtok(buffer, ","));
        if (chiave == key)
        {
            printf("%d° riga trovata :\n%s\n", count + 1, buff_temp);
            strcpy(matrice[count], buff_temp);
            count++;
        }
    }

    fclose(esami);

    if (count > 0)
    {
        printf("sto inviando num righe\n");
        // invia num righe
        if (write(connectFD, &count, sizeof(int)) < 0)
        {
            perror("errore, non sono state inviate il num di righe\n");
            exit(1);
        }

        printf("sto inviando le tuple..\n");
        // invia tuple a segreteria
        int bytesc = 0;
        for (int i = 0; i < count; i++)
        {
            if ((bytesc = write(connectFD, matrice[i], sizeof(matrice[i]))) < 0)
            {
                perror("write");
                exit(1);
            }
        }

        printf("byte scritti: %d\n", bytesc);

        close(connectFD);
    }
    else
    {
        printf("sto inviando num righe\n");
        // invia num righe
        if (write(connectFD, &count, sizeof(int)) < 0)
        {
            perror("errore, non sono state inviate il num di righe\n");
            exit(1);
        }
        close(connectFD);
    }
}

void richiesta_prenotazione(int connectFD){
    char buffer[1024];
    printf("\n==============================\n");
    FILE *esami = fopen("esami.csv", "r+");
    if (esami == NULL)
    {
        printf("Error: Could not open the file\n");
        exit(-1);
    }
    printf("File opened\n");

    // leggiamo la prima riga
    fgets(buffer, sizeof(buffer), esami);
    printf("%s\n", buffer);

    //crea_connessione(&connectFD);
    int key = get_key(connectFD);

    printf("inizia il while qui\n");

    // inizializziamo matrice
    int i = 0;
    int count = 0; // per scorrere le righe della matrice, indica anche il num di tuple trovate
    int chiave;    // per ottenere il valore intero con atoi

    char matrice[10][1024];

    int posizione_tupla = -1;
    int array_di_indici[10];
    while (fgets(buffer, sizeof(buffer), esami))
    {

        char buff_temp[1024];
        strcpy(buff_temp, buffer);

        chiave = atoi(strtok(buffer, ","));
        posizione_tupla++;
        if (chiave == key)
        {
            printf("%d° riga trovata :\n%s\n", count + 1, buff_temp);
            strcpy(matrice[count], buff_temp);
            count++;
            array_di_indici[--count] = posizione_tupla;
        }
    }

    if (count > 0)
    {
        printf("sto inviando num righe\n");
        // invia num righe
        if (write(connectFD, &count, sizeof(int)) < 0)
        {
            perror("errore, non sono state inviate il num di righe\n");
            exit(1);
        }

        printf("sto inviando le tuple..\n");
        // invia tuple a segreteria
        int bytesc = 0;
        for (int i = 0; i < count; i++)
        {
            if ((bytesc = write(connectFD, matrice[i], sizeof(matrice[i]))) < 0)
            {
                perror("write");
                exit(1);
            }
        }

        // va fatta qui la parte in cui lo studente sceglie la data tramite una semplice "scelta"
        int sceltaData;
        if (read(connectFD, &sceltaData, sizeof(sceltaData)) < 0)
        {
            perror("sceltaData andata male");
            exit(-1);
        }

        // ricevuta la scelta data
        char buffer_tupla[1024]; // per poter ricopiare la tupla aggiornata nel file
        sceltaData--;
        //debug
        printf("%s",matrice[sceltaData]);
        char *campo = strrchr(matrice[sceltaData], ',');
        int prenotazione = atoi(campo + 1);
        prenotazione++;
        sprintf(campo + 1, "%d\n", prenotazione); // aggiorniamo la prenotazione
        //debug
        printf("%s",matrice[sceltaData]);
        // Aggiorna il file con la nuova prenotazione
        fseek(esami, array_di_indici[sceltaData], SEEK_SET);
        fputs(matrice[sceltaData], esami);

        fclose(esami);
        close(connectFD);
    }
    else
    {
        printf("sto inviando num righe\n");
        // invia num righe
        if (write(connectFD, &count, sizeof(int)) < 0)
        {
            perror("errore, non sono state inviate il num di righe\n");
            exit(1);
        }
        close(connectFD);
    }
}

int main(int argc, char **argv)
{ 
    int serverfd;
    char * data;
    int connectFD;
    int scelta;
    
    while(1){
        crea_connessione(&connectFD);
        if(read(connectFD,&scelta, sizeof(scelta))<0)
        {
            perror("read non andata bene");
            exit(-1);
        }

        // debug
        printf("scelta per lo switch:%d", scelta);

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
                //gestire aggiunta esami sotto richiesta di segreteria
            }
            break;
        }
    }
    printf("\n==============================\n");
}