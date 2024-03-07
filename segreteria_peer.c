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

 int manage_exams(int connfd,int listenfd) //RICEVE CHIAVE
{
    int buff;
  if ((read(connfd, &buff, 1024)) < 0) // legge la chiave da cercare (mandata da studente)
  {
      perror("errore read");
      exit(1);
  }
  // la socket va chiusa SOLO dopo aver mandato indietro (a studente), le informazioni richieste(lista date esami)

  printf(" mi trovo su manage exams\n");
  printf("chiave = %d\n", buff); // test
  return buff;
}


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

int main(int argc, char **argv){
    int listenFD, connectFD, socketClientFD;
    int pid;
    struct sockaddr_in server_addr, client_addr;
    char readBuf[1024], writeBuf[1024];
    //da decommentare 
   // if(argc<3){fprintf(stderr, "Si utilizza cosi': ./peer <porta su cui vuoi offrire il servizio> <porta su cui vuoi eventualmente chiedere il servizio>\n\nES: ./peer 4000 2500\n"); exit(-1);}


    //--------- CREO LA SOCKET per far fungere come SERVER (per connettermi con studente)
    if( (listenFD=socket(AF_INET, SOCK_STREAM, 0))<0){perror("socket server"); exit(-1);}

    //SETTO LA SOCKET DI RICEZIONE
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons(1024);

    setsockopt(listenFD, SOL_SOCKET, SO_REUSEPORT, &(int){ 1 }, sizeof(int));
    //BIND
    if(bind(listenFD, (struct sockaddr*)&server_addr, sizeof(server_addr))<0 ){perror("bind"); exit(-1);}
    //--------- SERVER


    //--------- CREO LA SOCKET CLIENT
    //if( (socketClientFD=socket(AF_INET, SOCK_STREAM, 0))<0){perror("socket client"); exit(-1);}

    //SETTO LA SOCKET 
    client_addr.sin_family=AF_INET;
    client_addr.sin_port=htons(1025); //PORTA SERVER UNIVERSITARIO
    
    //da modificare (POSSIBILMENTE)
    if(inet_pton(AF_INET, "127.0.0.1", &client_addr.sin_addr)<0){perror("inetpton"); exit(-1);}
    //--------- CLIENT


    /*ORA MI METTO IN ASCOLTO DA SERVER, E DA CLIENT VOLTA PER VOLTA
    DECIDO DI RICHIEDERE UN SERVIZIO SE NE NECESSITO ASCOLTANDO
    LO STDIN*/
    if(listen(listenFD, 1000) < 0){perror("listen"); exit(-1);}
    printf("--- ASCOLTO ---\n      ...\n");


    //VARIABILI PER IL MULTIPLEXING
    fd_set readSet, writeSet;
    int fd_disponibili, fd_connectedClient[FD_SETSIZE]={0}, i;
    int maxfd=listenFD;
    
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    //-----


    while(1){

        FD_SET(listenFD,&readSet);//SETTO IL READSET SUL FD DELLA SOCKET IN ASCOLTO
    
        //Mi metto in ascolto di STDIN se voglio diventare client
        FD_SET(STDIN_FILENO, &readSet);

        //Opero su tutti i client connessi controllando l'apposito array
        for(i=0; i<=maxfd; i++){
            if(fd_connectedClient[i]){
                FD_SET(i, &writeSet);//SETTO IL WRITESET PER I CLIENT CONNESSI E A CUI POSSO "SCRIVERE"
            }
        }

        printf("maxfd attuale=%d\n", maxfd);
        fd_disponibili=select(maxfd+1, &readSet, &writeSet, NULL, NULL);
        printf("FD Disponibili=%d\n", fd_disponibili);
        sleep(1);

    
    //SE HO UN NUOVO CLIENT CONNESSO (PARTE SERVER)
        if(FD_ISSET(listenFD, &readSet)){
            printf("---|Nuovo client connesso");
            //siamo connessi con il cliente
            connectFD=accept(listenFD, NULL, NULL);

            if((pid= fork())<0){
                perror (" fork error ");
                exit ( -1);
            }

            if(pid==0){ //SE SONO IL FIGLIO GESTISCO IL SERVZIO	
                int choice;

                read(connectFD,&choice, 1024); //connectFD è fd della connessione con studente
                //ho letto la scelta, ora va fatto switch case

                switch (choice)
                {
                    case 1: //invia 
                    {
                        char **tuple = (char **)calloc(10, sizeof(char *));
                        for (i = 0; i < 10; i++)
                        {
                            tuple[i] = (char *)calloc(1024, sizeof(char));
                        }
                          //STEP:
                        //connettiamoci con server 
                        //manda id
                        //prendi buffer di tuple esami
                        //restituisci a studente co FullWrite
                        int chiave = manage_exams(connectFD,listenFD);
                        
                        //ora si connette con server
                        if((socketClientFD=socket(AF_INET, SOCK_STREAM, 0))<0)
                        {
                            perror("socket client"); 
                            exit(-1);
                        }
                        if (connect(socketClientFD, (struct sockaddr *) &client_addr, sizeof(client_addr)) < 0) 
                        {
                            fprintf(stderr,"Errore di connessione\n");
                            exit(1);
                        }
                        printf("Connessione avvenuta con il Server Universitario.\n");
                        //ho inviato la chiave
                        write(socketClientFD,&chiave,sizeof(chiave));
                        read(socketClientFD,tuple,sizeof(tuple));
                        write(socketClientFD,tuple,sizeof(tuple));
                       

                    }
                }
              
                close(connectFD);
                exit (-1);
            }
            else 
            { //SE SONO IL PADRE
                close (connectFD);
            }


            printf(" sul FD %d\n", connectFD);
            
            fd_connectedClient[connectFD]=1;//LO METTO NELLA LISTA DEI CLIENT CONNESSI

            if(maxfd<connectFD) maxfd=connectFD;//RICONTROLLO IL MAX

            fd_disponibili--;
        }

        i=0;

        //SE PREMO INVIO DIVENTO CLIENT
        if(FD_ISSET(STDIN_FILENO, &readSet)){
            
            read(STDIN_FILENO, readBuf, 10);//LEGGO LO STDIN PERCHE' IL SEMPLICE FLUSH DEL BUFFER NON VA PURTROPPO...

            fd_disponibili--;

            printf("Sono client\n");
            if( (socketClientFD=socket(AF_INET, SOCK_STREAM, 0))<0){perror("socket client"); exit(-1);}
            if(connect(socketClientFD, (struct sockaddr*)&client_addr, sizeof(client_addr))< 0){perror("connect"); exit(-1);}//MI CONNETTO
            printf("Connesso ad un altro peer\n");
            
	        if(read(socketClientFD, readBuf, sizeof(readBuf))>0){//LEGGO
                fputs(readBuf, stdout);
            }
            
            shutdown(socketClientFD, 2);
            close(socketClientFD);//CHIUDO IL CANALE DI COMUNICAZIONE
            
            fflush(stdin);//PULISCO IL BUFFER

            printf("fine client\n");
        }

        

        //SERVO TUTTI I CLIENT ADESSO FIN QUANDO VE NE SONO DISPONIBILI (PARTE SERVER)
        while(fd_disponibili>0 && i<FD_SETSIZE){
            i++; //INCREMENTO i PER CONTINUARE A CONTROLLARE

            if(fd_connectedClient[i]==0){//SE E' ZERO, VAI AL PROSSIMO CICLO SENZA PROSEGUIRE
                continue;
            }

            //ALTRIMENTI SERVI IL CLIENT
            if(FD_ISSET(i, &writeSet)){
                printf("---|Servo il client connesso sulla fd %d\n...\n", i);
                fd_disponibili--;
                

                //OFFRO IL SERVIZIO

                //----
                close(i);//CHIUDO IL CANALE COL CLIENT APPENA SERVITO


                fd_connectedClient[i]=0;
                printf("---|Client connesso sulla fd %d servito.\n", i);

                if(maxfd == i){ //Se ho raggiunto (e servito) il maxfd devo trovare il nuovo maxfd procedendo a ritroso
                    while(fd_connectedClient[--i] ==0){
                        maxfd=i;
                        break;
                    }
                }

            }

        }//WHILE SERVER CHE OFFRE SERVIZIO AI CLIENT GIA CONNESSI

    }//------
    
    exit(1);
}