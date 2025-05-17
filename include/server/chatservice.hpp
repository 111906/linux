#ifndef CHARSERVICE_H
#define CHARSERVICE_H

#include<muduo/net/TcpServer.h>
#include<unordered_map>
#include<functional>
#include"../../thirdparty/json.hpp"
#include "../../include/server/usermodel.hpp"
#include"../../include/server/offlinemessagemodel.hpp"
#include"../../include/server/friendmodel.hpp"
#include"../../include/server/groupmodel.hpp"
#include"../../include/server/kafka.hpp"

#include"usermodel.hpp"
#include<mutex>

using namespace std;
using namespace muduo;
using namespace muduo::net;
using json=nlohmann::json;

//处理消息的回调函数类型
using MsgHandler=std::function<void(const TcpConnectionPtr& conn, json& js, Timestamp time)>;

//聊天服务器业务类
class ChatService{
    public:
        //获取单例对象的接口函数
        static ChatService* instance();
        //处理登录业务
        void login(const TcpConnectionPtr& conn, json& js, Timestamp time);
        //处理注册业务
        void reg(const TcpConnectionPtr& conn, json& js, Timestamp time);
        //处理一对一聊天业务
        void oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time);
        //添加好友
        void addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time);
        //创建群组
        void createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);
        //加入群组
        void addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);
        //群组聊天
        void groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time);
        // 获取消息对应的处理函数
        MsgHandler getHandler(int msgid);
        //处理客户端异常退出
        void clientCloseException(const TcpConnectionPtr& conn);
        //服务器异常退出,业务重置方法
        void reset();
        //处理注销登录业务
        void loginout(const TcpConnectionPtr& conn, json& js, Timestamp time);
        //kafka消息处理
        void handleKafkaMessage(const string& topic, const string& message);
        //单例模式
        ChatService();
        //存储消息id和对应的业务处理函数
        unordered_map<int, MsgHandler> _msgHandlerMap;
        //存储在线用户的连接
        unordered_map<int, TcpConnectionPtr> _userConnMap;

        //定义互斥锁，保护userConnMap的线程安全
        mutex _connMutex;

        //数据操作类对象
        UserModel _usermodel;
        //离线消息操作类对象
        OfflineMsgModel _offlineMsgModel;
        //存储好友关系的操作类对象
        FriendModel _friendModel;
        //存储群组操作类对象
        GroupModel _groupModel;
        // Kafka 操作类对象
        Kafka _kafka;
};

#endif