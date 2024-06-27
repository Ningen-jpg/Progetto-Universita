#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

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

//nuova
int get_key(int * connectFD)
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

    int key;
    *connectFD = accept(listenFD, NULL, NULL);
    // printf("ho accettato con accept\n");
    read(*connectFD, &key, sizeof(key));
    //printf("il fd e : %d\n", connectFD);
    close(listenFD);
    return key;
}

//vecchia
/*int get_key(int * connectFD)
{
    int listenFD;
    struct sockaddr_in server;

    //creiamo la socket
    if ( ( listenFD = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ) {
        perror("socket");
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

    int key;
    *connectFD=accept(listenFD, NULL, NULL);
    // printf("ho accettato con accept\n");
    read(*connectFD,&key,sizeof(key));
    //printf("il fd e : %d\n", connectFD);

    return key;
}*/

int main(int argc, char **argv)
{ 
    char buffer[1024];
    int serverfd;
    char * data;
    int connectFD;
    printf("\n==============================\n");
    FILE * esami = fopen("esami.csv", "r");
    if (esami == NULL)
    {
        printf("Error: Could not open the file\n");
        exit(-1);
    }
    printf("File opened\n");

    //leggiamo la prima riga
    fgets(buffer, sizeof(buffer), esami);
    printf("%s\n", buffer);

    while(1){
        int key = get_key(&connectFD);
        printf("inizia il while qui\n");

        //inizializziamo matrice
        int i=0;
        int count=0; //per scorrere le righe della matrice, indica anche il num di tuple trovate
        int chiave; //per ottenere il valore intero con atoi
        // char **matrice = (char **)calloc(10,sizeof(char *));
        char matrice [10][1024];
    
        while (fgets(buffer, sizeof(buffer), esami))
        {
            char buff_temp[1024];
            strcpy(buff_temp,buffer);
                
            chiave = atoi(strtok(buffer, ","));
            if (chiave == key)
            {
                printf("\n ============TEST\n============\n debug buff_temp =%s\n",buff_temp);
                strcpy(matrice[count], buff_temp);
                count++;
            }
        }

        //test
        printf("Tuple trovate con ID %d:\n", key);
        for (int i = 0; i < count; i++)
        {
            printf("%s", matrice[i]);
        }

        printf("sto inviando num righe\n");
        //invia num righe
        if(write(connectFD, &count, sizeof(int))<0)
        {
            perror("errore, non sono state inviate il num di righe\n");
            exit(1);
        }

        printf("sto inviando le tuple..\n");
        //invia tuple a segreteria
        int bytesc =0;
        for(int i=0;i<count;i++)
        {
            if ((bytesc= write(connectFD, matrice[i], sizeof(matrice[i])))< 0) {
                perror("write");
                exit(1);
            }
        }

        //printf("la prima tupla è: %s\n", matrice[0]);
        //test per vedere se invia anche una sola riga
        //printf("size della prima tupla: %lu\n", strlen(matrice[0]));

        // matrice[0][strlen(matrice[0])+1] = '\0';
        /*if ((bytesc= write(connectFD, matrice[i], 1025))< 0) {
            perror("write");
            exit(1);
        }*/

        printf("byte scritti: %d\n", bytesc);

        // test
        // check del size di matrice[i]
        /*
        for (int i = 0; i < count; i++){
            printf("sto per scrivere la seguente riga: %s\n",matrice[i]);
            matrice[i][strlen(matrice[i])+1]= '\0';
            if (write(connectFD, matrice[i], strlen(matrice[i])+1)< 0) {
                perror("write");
                exit(1);
            }
            printf("il size di matrice[i]+1 e: %lu\n",strlen(matrice[i]+1));

        }
        */

        printf("FORSE ho inviato le tuple\n");

        close(connectFD);
    }
    fclose(esami);
    printf("\n==============================\n");
}
