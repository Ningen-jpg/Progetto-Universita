all: server segreteria studente 


server: server.c
	gcc server.c -o server

segreteria: segreteria_peer.c
	gcc segreteria_peer.c -o peer

studente: studente.c
	gcc studente.c -o studente

serverExe: server
	./server

peerExe: peer
	./peer

studenteExe: studente
	./studente 127.0.0.1

clean: 
	@echo "Sto pulendo..."
	rm studente