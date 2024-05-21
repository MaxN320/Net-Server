all:client tcpepoll

client:client.cpp
	g++ -g -o client client.cpp
tcpepoll:tcpepoll.cpp global.cpp config.cpp InetAddress.cpp Socket.cpp
	g++ -g -std=c++11 -o  tcpepoll tcpepoll.cpp global.cpp config.cpp InetAddress.cpp Socket.cpp

clean:
	rm -f client tcpepoll
