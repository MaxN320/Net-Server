all:client tcpepoll

client:client.cpp
	g++ -g -o client client.cpp
tcpepoll:tcpepoll.cpp global.cpp config.cpp
	g++ -g -std=c++11 -o  tcpepoll tcpepoll.cpp global.cpp config.cpp

clean:
	rm -f client tcpepoll
