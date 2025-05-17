#include<muduo/net/EventLoop.h>
#include<muduo/net/TcpServer.h>
#include<iostream>
#include<functional>
#include<string>
using namespace std;
using namespace muduo;
using namespace muduo::net;

//基于muduo网络库开发服务器程序
//1，创建TcpServer对象
//2，创建EventLoop对象
//3，tcpserver的构造函数
//4，设置连接回调函数和消息回调函数
//5，设置线程数，muduo库会自动分配i/o线程和工作线程
//6，启动服务器


class ChatServer
{
    public:
        ChatServer(EventLoop* loop, const InetAddress& listenAddr,const string& nameArg) 
            : _server(loop, listenAddr, nameArg)
            , loop_(loop)
        {
            //用户连接创建和断开时的回调函数 
            _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
            /*_server.setConnectionCallback([this](const TcpConnectionPtr& conn) {
              this->onConnection(conn);
            });*/
            //用户发送消息时的回调函数
            _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        
            //设置线程数
            _server.setThreadNum(4);
        }

        void start()
        {
            _server.start();
        }
    private:
        //处理用户连接和断开
        void onConnection(const TcpConnectionPtr& conn)
        {
            if (conn->connected())
            {
                cout << "New connection: " << conn->peerAddress().toIpPort() <<"你爹来啦！"<< endl;
            }
            else
            {
                cout << "Connection closed: " << conn->peerAddress().toIpPort() <<"你爹走啦！"<< endl;
            }
        }
        //处理用户发送的消息
        void onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time)
        {
            string msg = buffer->retrieveAllAsString();
            cout << "Received message: " << msg << endl;
            conn->send(msg);
        }
        TcpServer _server;
        EventLoop *loop_;
    
};

int main()
{
    EventLoop loop;
    InetAddress listenAddr("192.168.247.129",8888);
    ChatServer server(&loop, listenAddr, "ChatServer");
    server.start();//listenfd通过epoll_ctl添加到epollfd中
    loop.loop();//epoll_wait以阻塞的方式等待新用户连接，已连接用户的读写事件发生等
    return 0;
}