CC = clang
all: server segreteria studente 

clang: serverClang studenteClang peerClang

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

serverClang: server.c
	clang server.c -o server
	
studenteClang: studente.c
	clang studente.c -o studente

peerClang: peer.c
	clang peer.c -o peer

clean: 
	@echo "Sto pulendo..."
	rm studente peer server