all:client tcpepoll

client:client.cpp  global.cpp config.cpp
	g++ -g -o client client.cpp global.cpp config.cpp
tcpepoll:tcpepoll.cpp global.cpp config.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp EventLoop.cpp TcpServer.cpp Acceptor.cpp Connection.cpp Buffer.cpp EchoServer.cpp
	g++ -g -std=c++11 -o  tcpepoll tcpepoll.cpp global.cpp config.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp EventLoop.cpp TcpServer.cpp Acceptor.cpp Connection.cpp Buffer.cpp EchoServer.cpp

clean:
	rm -f client tcpepoll
