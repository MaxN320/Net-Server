all:client echoserver

client:client.cpp
	g++ -g -o client client.cpp
echoserver: InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp EventLoop.cpp TcpServer.cpp Acceptor.cpp Connection.cpp Buffer.cpp EchoServer.cpp ThreadPool.cpp
	g++ -g -o echoserver  InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp EventLoop.cpp TcpServer.cpp Acceptor.cpp Connection.cpp Buffer.cpp EchoServer.cpp ThreadPool.cpp -lpthread

clean:
	rm -f client echoserver
