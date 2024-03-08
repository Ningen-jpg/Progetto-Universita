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


int get_key(int * connectFD)
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
}

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

    int key = get_key(&connectFD);
    printf("inizia il while qui\n");

    //inizializziamo matrice
    int i=0;
    int count=0; //per scorrere le righe della matrice
    int chiave; //per ottenere il valore intero con atoi
    char **matrice = (char **)calloc(10,sizeof(char *));
    for (i = 0; i < 10; i++) {
        matrice[i] = (char *)calloc(1024, sizeof(char));
    }
        while (fgets(buffer, sizeof(buffer), esami))
        {
            char buff_temp[1024];
            strcpy(buff_temp,buffer);
            
            chiave = atoi(strtok(buffer, ","));
            if (chiave == key)
            {
                strcpy(matrice[count], buff_temp);
            }
        }

        //test
        printf("Tuple trovate con ID %d:\n", key);
        for (int i = 0; i < count; i++)
        {
            printf("%s", matrice[i]);
        }

        //invia tuple a segreteria
        for (int i = 0; i < count; i++){
            if (write(connectFD, matrice, strlen(matrice[i])+1)< 0) {
                perror("write");
                exit(1);
            }
        }
        for (int i = 0; i < count; i++)
        {
            free(matrice[i]);
        }

        fclose(esami);
        free(matrice);
        printf("\n==============================\n");
}
