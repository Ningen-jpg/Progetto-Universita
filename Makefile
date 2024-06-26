CC = clang
all: server segreteria studente 


server: server.c
	clang server.c -o server

segreteria: segreteria_peer.c
	clang segreteria_peer.c -o peer

studente: studente.c
	clang studente.c -o studente

serverExe: server
	./server

peerExe: peer
	./peer

studenteExe: studente
	./studente 127.0.0.1

clean: 
	@echo "Sto pulendo..."
	rm studente peer server