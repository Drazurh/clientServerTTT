
all: TTT TTT_server

TTT: client.o message.h
	g++ -o TTT -g client.cc message.cc -lncurses

TTT_server: server.o message.h
	g++ -o TTTServer -g server.cc message.cc
