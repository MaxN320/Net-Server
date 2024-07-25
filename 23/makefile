all:client echoServer

client:client.cpp  global.cpp config.cpp
	g++ -g -o client client.cpp global.cpp config.cpp
echoServer:EchoServer.cpp global.cpp config.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp EventLoop.cpp TcpServer.cpp Acceptor.cpp Connection.cpp Buffer.cpp EchoServer.cpp
	g++ -g -std=c++11 -o  EchoServer EchoServer.cpp global.cpp config.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp EventLoop.cpp TcpServer.cpp Acceptor.cpp Connection.cpp Buffer.cpp EchoServer.cpp

clean:
	rm -f client echoServer
