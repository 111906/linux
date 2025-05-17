#include "../../include/server/chatserver.hpp"
#include"../../include/server/chatservice.hpp"

#include"../../thirdparty/json.hpp"
#include<functional>
using namespace std;
using namespace placeholders;
using json=nlohmann::json;


ChatServer::ChatServer(EventLoop* loop, const InetAddress& listenAddr,const string& nameArg) 
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

        void ChatServer::start()
        {
            _server.start();
        }


         //处理用户连接和断开
         void ChatServer::onConnection(const TcpConnectionPtr& conn)
         {
            //客户端断开连接
             if(!conn->connected())
             {
                ChatService::instance()->clientCloseException(conn);
                conn->shutdown();
             }
             
         }
         //处理用户发送的消息
         void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time)
         {
             string msg = buffer->retrieveAllAsString();
             //数据解码
             json js=json::parse(msg);
             //通过回调或者接口做到一个连接一个处理handler
             auto msghandler=ChatService::instance()->getHandler(js["msgid"].get<int>());
             //回调消息绑定好的事件处理器，来执行相应的业务处理
             msghandler(conn, js, time);
         }
         
         