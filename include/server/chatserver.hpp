#ifndef CHATSERVER_H
#define CHATSERVER_H

#include<muduo/net/EventLoop.h>
#include<muduo/net/TcpServer.h>

using namespace muduo;
using namespace muduo::net;

//主类
class ChatServer{

    public:
        //构造函数
        ChatServer(EventLoop* loop, const InetAddress& listenAddr,const string& nameArg);
        //启动服务器 
        void start();
    private:
        EventLoop* loop_;//指向事件循环对象的指针
        TcpServer _server;//实现muduo网络库的服务器的类对象
        //处理用户连接和断开
        void onConnection(const TcpConnectionPtr& conn);
        //处理用户发送的消息
        void onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time);
        
};

#endif