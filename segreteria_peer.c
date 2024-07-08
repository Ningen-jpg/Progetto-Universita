/*
La segreteria rappresenta il nostro peer,
ovvero un nodo in grado di fungere sia da client che da server verso gli altri nodi della rete.
In sostanza il nostro peer può svolgere diverse operazioni in base alla modalità in cui opera:
1) Modalità client:
- la segreteria può inserire gli esami sul server universitario
2) Modalità Server:
- inoltra la richiesta di prenotazione degli studenti al server universitario.
- fornisce allo studente le date degli esami disponibili per l'esame scelto dallo studente.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

int manage_exams(int connfd, int listenfd) // Riceve l'ID che serve a recuperare l'esame scelto
{
    int buff;
    if ((read(connfd, &buff, sizeof(buff))) < 0) // legge l'ID da cercare (mandato da studente)
    {
        perror("errore read");
        exit(1);
    }
    // la socket verrà chiusa SOLO dopo aver mandato indietro (a studente), le informazioni richieste(lista date esami)
    return buff;
}

void ricerca_esami(int connectFD,int listenFD,int socketClientFD,struct sockaddr_in server_addr, struct sockaddr_in client_addr, int choice)
{
    int chiave = manage_exams(connectFD, listenFD);

    // invio la scelta per lo switch-case
    if (write(socketClientFD, &choice, sizeof(choice)) < 0)
    {
        perror("errore: non e' stata copiata la scelta");
        exit(1);
    }

    // invio l'id dell'esame da cercare
    if (write(socketClientFD, &chiave, sizeof(chiave)) < 0)
    {
        perror("errore: non e' stata copiata la chiave");
        exit(1);
    }

    // prende le tuple
    int righe = 0; 
    if (read(socketClientFD, &righe, sizeof(righe)) < 0)
    {
        perror("read non fatta");
        exit(-1);
    }

    // allochiamo la matrice che conterrà eventualmente le tuple trovate
    char tuple[50][1024];

    // leggiamo le tuple da server

    int read_value = 0; // utilizzata per verificare se venivano letti tutti i byte
    for (int i = 0; i < righe; i++)
    {
        if ((read_value = read(socketClientFD, &tuple[i], sizeof(tuple[i]))) < 0) // leggo una ad una le tuple trovate dal server
        {
            perror("errore: non è stata letta nessuna tupla\n");
            exit(1);
        }
    }

    if (write(connectFD, &righe, sizeof(righe)) < 0) //invio num_righe
    {
        perror("errore: num righe non inviato\n");
        exit(1);
    }

    printf("=========================\n");

    // mando le tuple, riga per riga a studente
    for (int i = 0; i < righe; i++)
    {
        if (write(connectFD, tuple[i], sizeof(tuple[i])) < 0)
        {
            perror("errore: non è stata inviata una tupla\n");
            exit(1);
        }
    }
}

void richiesta_prenotazione(int connectFD,int listenFD,int socketClientFD,struct sockaddr_in server_addr, struct sockaddr_in client_addr,int choice) //richiesta prenotazione lato segreteria
{
    ricerca_esami(connectFD, listenFD, socketClientFD, server_addr, client_addr,choice); //choice è la scelta dello switch
    //leggo la scelta della data
        int sceltaData;
        if(read(connectFD,&sceltaData,sizeof(sceltaData))<0)
        {
            perror("read non andata bene");
            exit(-1);
        }
        
        //mando la scelta della tupla sulla quale prenotarci su server
        int bytescritti;
        sleep(1);
        if ((bytescritti= write(socketClientFD, &sceltaData, sizeof(sceltaData))) < 0)
        {
            printf("byte scritti: %d\n",bytescritti);
            perror("Write non andata bene");
            exit(-1);
        }

        // read del numero progressivo di prenotazioni (è stato già incrementato su server)
        int numeroProgress;
        
        if (read(socketClientFD, &numeroProgress, sizeof(numeroProgress)) < 0)
        {
            perror("read non andata bene");
            exit(-1);
        }

        //mandiamo il numero progressivo a studente
        if (write(connectFD, &numeroProgress, sizeof(numeroProgress)) < 0)
        {
            perror("Write non andata bene");
            exit(-1);
        }
    
}

void aggiunta_esame(int socketClientFD){
    int scelta=3;  // switch-case sul server
    //comunico al server che voglio aggiungere un esame
    if (write(socketClientFD, &scelta, sizeof(scelta)) < 0)
    {
        perror("errore: non e' stata inviata la scelta");
        exit(1);
    }
    char ID[5]; //ID sempre di 4 caratteri
    char nome[50]; //il nome dell'esame
    char data[11]; //la data dell'esame, sempre di 10 caratteri
    char num[4]="0"; //ipotizziamo che il numero di prenotati non superi il centinaio

    char esame[1024]=""; //inizializzo la stringa

    printf("\nInserisci l'ID dell'esame: ");
    fflush(stdin);
    scanf("%4s",ID);
    getchar();
    printf("\nInserisci il nome dell'esame: ");
    fgets(nome, sizeof(nome), stdin);  // Utilizzo la fgets per avere la possibilità di inserire anche gli spazi
    nome[strcspn(nome, "\n")] = 0; // Rimuove il newline alla fine
    printf("\nInserisci la data dell'esame: ");

    scanf("%10s",data);
   // data[strcspn(data, "\n")] = 0;//rimuove il newline dopo la data

    strcat(esame, ID);
    strcat(esame, ",");
    strcat(esame, nome);
    strcat(esame, ",");
    strcat(esame, data);
    strcat(esame, ",");
    strcat(esame,num);

    esame[strcspn(esame,"\n")]= 0;//rimuove il newline in più
    //invio la stringa al server
    if (write(socketClientFD, &esame, sizeof(esame)) < 0)
    {
        perror("errore: non e' stata inviata la stringa concatenata");
        exit(1);
    }
    printf("\nstringa inviata. Attendo conferma..\n");
}

int main(int argc, char **argv){
    int listenFD, connectFD, socketClientFD; // File descriptor da utilizzare per le connessioni
    struct sockaddr_in server_addr, client_addr;
    // Dichiarazione di due variabili di tipo struct sockaddr_in:
    // - server_addr sarà utilizzata per memorizzare l'indirizzo del server.
    // - client_addr sarà utilizzata per memorizzare l'indirizzo del client.
    // Le strutture sockaddr_in contengono informazioni necessarie per la comunicazione di rete, come gli indirizzi IP e le porte.
    
    char readBuf[1024], writeBuf[1024];
    
    // Creo la socket per far fungere come SERVER (per connettermi con studente)
    if ((listenFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket server");
        exit(-1);
    }

    //SETTO LA SOCKET DI RICEZIONE
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons(1024);

    setsockopt(listenFD, SOL_SOCKET, SO_REUSEPORT, &(int){ 1 }, sizeof(int));
    // Imposta l'opzione SO_REUSEPORT sul socket listenFD.
    // SO_REUSEPORT consente di riutilizzare immediatamente un indirizzo/porta locale, anche se è già in uso.
    // &(int){ 1 } è una sintassi C che crea un valore intero temporaneo pari a 1 e ne prende l'indirizzo.
    // sizeof(int) indica la dimensione del dato da passare a setsockopt, che in questo caso è un intero.

    // BIND
    if (bind(listenFD, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) // Associa il socket 'listenFD' all'indirizzo specificato da 'server'; verifica se il binding fallisce
    {
        perror("bind");
        exit(-1);
    }

    //--------- SERVER

    //--------- CREO LA SOCKET CLIENT
    //SETTO LA SOCKET 
    client_addr.sin_family=AF_INET;
    client_addr.sin_port=htons(1025); //PORTA SERVER UNIVERSITARIO
    
    //da modificare (POSSIBILMENTE)
    if (inet_pton(AF_INET, "127.0.0.1", &client_addr.sin_addr) < 0)
    {
        perror("inetpton");
        exit(-1);
    }
    // Converte l'indirizzo IPv4 rappresentato come stringa "127.0.0.1" nel formato binario utilizzato da sockaddr_in.
    // Il risultato viene memorizzato nel campo sin_addr della struttura client_addr.
    // Verifica se la conversione ha avuto successo controllando se il valore restituito da inet_pton è inferiore a 0 in caso di errore.
    
    //--------- CLIENT

    /*ORA MI METTO IN ASCOLTO DA SERVER, E DA CLIENT VOLTA PER VOLTA
    DECIDO DI RICHIEDERE UN SERVIZIO SE NE NECESSITO ASCOLTANDO
    LO STDIN*/
    if (listen(listenFD, 1000) < 0)
    {
        perror("listen");
        exit(-1);
    }
    printf("--- SONO IN ASCOLTO ---\n\n");

    //VARIABILI PER IL MULTIPLEXING
    fd_set readSet, writeSet;
    int fd_disponibili, fd_connectedClient[FD_SETSIZE]={0}, i;
    int maxfd=listenFD;
    
    //-----

    while(1){

        FD_ZERO(&readSet); // Azzera l'insieme di descrittori di file specificato da readSet.
        FD_ZERO(&writeSet); // Azzera l'insieme di descrittori di file specificato da writeSet.
        FD_SET(listenFD,&readSet); //SETTO IL READSET SUL FD DELLA SOCKET IN ASCOLTO

        //Mi metto in ascolto di STDIN se voglio diventare client
        FD_SET(STDIN_FILENO, &readSet);

        // Opero su tutti i client connessi controllando l'apposito array
        for(i=0; i<=maxfd; i++){
            if(fd_connectedClient[i]){
                FD_SET(i, &writeSet); // SETTO IL WRITESET PER I CLIENT CONNESSI E A CUI POSSO "SCRIVERE"
            }
        }

        fd_disponibili=select(maxfd+1, &readSet, &writeSet, NULL, NULL); //maxfx + 2 perchè deve gestire contemporanemante 1 client e 1 server
        fd_disponibili = fd_disponibili+1;

        // SE HO UN NUOVO CLIENT CONNESSO (PARTE SERVER)
        if (FD_ISSET(listenFD, &readSet)) // verifica se un determinato file descriptor è presente in un insieme di descrittori di file
        { 
            printf("---|Nuovo studente connesso");

            // siamo connessi con il client
            connectFD = accept(listenFD, NULL, NULL);
            int choice = 0;
            read(connectFD, &choice, sizeof(choice)); // connectFD è fd della connessione con studente

            // ho letto la scelta, ora va fatto switch case

            // ora si connette con server
            if ((socketClientFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                perror("socket client");
                exit(-1);
            }
            if (connect(socketClientFD, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
            {
                fprintf(stderr, "Errore di connessione con il server\n");
                exit(1);
            }

            printf("\n\n---|Connessione avvenuta con il Server Universitario.\n");

            switch (choice)
            {
                case 1:
                {
                    ricerca_esami(connectFD, listenFD, socketClientFD, server_addr, client_addr, choice);
                }
                break;
                case 2:
                {
                    richiesta_prenotazione(connectFD, listenFD, socketClientFD, server_addr, client_addr, choice);
                }
                break;
            }
            if (choice ==1 || choice == 2)
            {
                fd_connectedClient[connectFD] = 1; // LO METTO NELLA LISTA DEI CLIENT CONNESSI

                if (maxfd < connectFD)
                {
                    maxfd = connectFD; // RICONTROLLO IL MAX
                }
                fd_disponibili--;
            }
        }

        i = 0;

        // SE PREMO INVIO DIVENTO CLIENT
        if(FD_ISSET(STDIN_FILENO, &readSet)){ //rimane in ascolto nel caso il peer voglia diventare client
            fflush(stdin);

            read(STDIN_FILENO, readBuf, 10); //legge qualsiasi input da tastiera

            fd_disponibili--;
            printf("Sono client\n");
            printf("=======================\n");
            
            if((socketClientFD=socket(AF_INET, SOCK_STREAM, 0))<0)
            { 
                perror("socket client"); 
                exit(-1); 
            }

            if(connect(socketClientFD, (struct sockaddr*)&client_addr, sizeof(client_addr))< 0)
            { 
                perror("connect error con il server dentro modalità client\n");
                exit(-1); 
            } //MI CONNETTO

            printf("Connesso con il server\n");

            //Aggiunta esame
            aggiunta_esame(socketClientFD);

	        if(read(socketClientFD, readBuf, sizeof(readBuf))>0) // LEGGO
            { 
                fputs(readBuf, stdout);
            }
            
            shutdown(socketClientFD, 2); // chiude la connessione con il server
            close(socketClientFD); // CHIUDO IL CANALE DI COMUNICAZIONE
            fflush(stdin); // PULISCO IL BUFFER
            printf("fine client\n");
        }

        //SERVO TUTTI I CLIENT ADESSO FIN QUANDO VE NE SONO DISPONIBILI (PARTE SERVER)
        while(fd_disponibili>0 && i<FD_SETSIZE){
            i++; // INCREMENTO i PER CONTINUARE A CONTROLLARE
            if (fd_connectedClient[i] == 0)
            { // SE E' ZERO, VAI AL PROSSIMO CICLO SENZA PROSEGUIRE
                continue;
            }

            // ALTRIMENTI SERVI IL CLIENT
            if (FD_ISSET(i, &writeSet))
            {
                fd_disponibili--;

                // SVOLGO LE OPERAZIONI

                //----
                close(i); // CHIUDO IL CANALE COL CLIENT APPENA SERVITO

                fd_connectedClient[i] = 0;

                if (maxfd == i)
                { // Se ho raggiunto (e servito) il maxfd devo trovare il nuovo maxfd procedendo a ritroso
                    while (fd_connectedClient[--i] == 0)
                    {
                        maxfd = i;
                        break;
                    }
                }
            }
        } //WHILE SERVER CHE OFFRE SERVIZIO AI CLIENT GIA CONNESSI
    }//------
    exit(1);
}