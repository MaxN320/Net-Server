all:client tcpepoll

client:client.cpp  global.cpp config.cpp
	g++ -g -o client client.cpp global.cpp config.cpp
tcpepoll:tcpepoll.cpp global.cpp config.cpp InetAddress.cpp Socket.cpp
	g++ -g -std=c++11 -o  tcpepoll tcpepoll.cpp global.cpp config.cpp InetAddress.cpp Socket.cpp

clean:
	rm -f client tcpepoll
