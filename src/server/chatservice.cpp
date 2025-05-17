#include"../../include/server/chatservice.hpp"
#include"../../include/public.hpp"
#include<muduo/base/Logging.h>
#include <mutex>
#include<vector>
using namespace std;
using namespace placeholders;
using namespace muduo;


ChatService* ChatService::instance(){
    static ChatService service;
    return &service;
}

// 注册消息以及对应的业务处理函数
ChatService::ChatService(){
    // 用户基本业务的处理函数
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, placeholders::_1, placeholders::_2, placeholders::_3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, placeholders::_1, placeholders::_2, placeholders::_3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, placeholders::_1, placeholders::_2, placeholders::_3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, placeholders::_1, placeholders::_2, placeholders::_3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, placeholders::_1, placeholders::_2, placeholders::_3)});

    // 群组相关的消息处理函数
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, placeholders::_1, placeholders::_2, placeholders::_3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, placeholders::_1, placeholders::_2, placeholders::_3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, placeholders::_1, placeholders::_2, placeholders::_3)}); 

    // 连接 Kafka
    if(_kafka.connect("192.168.247.129:9092")){  // 根据实际情况修改 Kafka 地址
        // 设置回调函数
        _kafka.init_notify_handler(std::bind(&ChatService::handleKafkaMessage, this, placeholders::_1, placeholders::_2));
    } else {
        LOG_ERROR << "Kafka connect failed!";
    }
}

// 获取消息对应的处理函数
MsgHandler ChatService::getHandler(int msgid){
    auto it=_msgHandlerMap.find(msgid);
    if(it==_msgHandlerMap.end()){
        return [=](const TcpConnectionPtr& conn, json& js, Timestamp time){
            LOG_ERROR<<"msgid:"<<msgid<<"can not find handler";
        };
    }
    return _msgHandlerMap[msgid];
}

// 处理登录业务  id  pwd   pwd
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    string pwd = js["password"];

    User user = _usermodel.query(id);
    if (user.getId() == id && user.getPassword() == pwd)
    {
        if (user.getState() == "online")
        {
            // 该用户已经登录，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "this account is using, input another!";
            conn->send(response.dump());
        }
        else
        {
            // 登录成功，记录用户连接信息
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }

            // id 用户登录成功后，向 Kafka 订阅 topic(id)
            bool sub_ok = _kafka.subscribe(std::to_string(id));
            LOG_INFO << "subscribe(" << id << ") => " << sub_ok;

            // 登录成功，更新用户状态信息 state offline=>online
            user.setState("online");
            _usermodel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            // 查询该用户是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                // 读取该用户的离线消息后，把该用户的所有离线消息删除掉
                _offlineMsgModel.remove(id);
            }

            // 查询该用户的好友信息并返回
            vector<User> userVec = _friendModel.query(id);
            if (!userVec.empty())
            {
                vector<string> vec2;
                for (User &user : userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }
            // 查询用户的群组信息
            vector<Group> groupuserVec = _groupModel.queryGroups(id);
            if (!groupuserVec.empty())
            {
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                vector<string> groupV;
                for (Group &group : groupuserVec)
                {
                    json grpjson;
                    grpjson["id"] = group.getId();
                    grpjson["groupname"] = group.getName();
                    grpjson["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for (GroupUser &user : group.getUsers())
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;
                    groupV.push_back(grpjson.dump());
                }

                response["groups"] = groupV;
            }

            conn->send(response.dump());
        }
    }
    else
    {
        // 该用户不存在，用户存在但是密码错误，登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "id or password is invalid!";
        conn->send(response.dump());
    }
}

// 处理注册业务
void ChatService::reg(const TcpConnectionPtr& conn, json& js, Timestamp time){
    
    string name=js["name"];
    string password=js["password"];

    User user;
    user.setName(name);
    user.setPassword(password);    
    bool state=_usermodel.insert(user);
    if(state){
        json response;
        response["msgid"]=REG_MSG_ACK;
        response["errno"]=0;
        response["id"]=user.getId();
        conn->send(response.dump());
    }else{
        json response;
        response["msgid"]=REG_MSG_ACK;
        response["errno"]=1;
        response["errmsg"]="reg failed";
        conn->send(response.dump());
    }
}

// 处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr& conn){
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for(auto it=_userConnMap.begin();it!=_userConnMap.end();++it){
            if(it->second==conn){
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    // 取消订阅该用户的消息
    _kafka.unsubscribe();

    // 更新用户状态
    if(user.getId()!=0){
        user.setState("offline");
        _usermodel.updateState(user);
    }
}

// 服务器异常退出
void ChatService::reset(){
    // 把 online 用户的状态重置为 offline
    _usermodel.resetState();
}

// 处理一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time){
    int toid=js["toid"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it=_userConnMap.find(toid);
        if(it!=_userConnMap.end()){
            // 对方在线，转发消息
            it->second->send(js.dump());
            return;
        }
    }

    // 查询对方是否在线
    User user=_usermodel.query(toid);
    if(user.getState()=="online"){
        // 向 Kafka 发布消息
        bool pub_ok = _kafka.publish(std::to_string(toid), js.dump());
        LOG_INFO << "publish(to="<<toid<<") => "<<pub_ok;
        return;
    }
    // 对方不在线，存储离线消息
    _offlineMsgModel.insert(toid, js.dump());
}

// 添加好友
void ChatService::addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time){
    int userid=js["id"].get<int>();
    int friendid=js["friendid"].get<int>();
    // 添加好友
    _friendModel.insert(userid, friendid);
    // 返回结果
    json response;
    response["msgid"]=ADD_FRIEND_MSG_ACK;
    response["errno"]=0;
    conn->send(response.dump());
}

// 创建群组
void ChatService::createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time){
    int userid=js["id"].get<int>();
    string groupname=js["groupname"];
    string groupdesc=js["groupdesc"];

    // 存储群组信息
    Group group(-1,groupname,groupdesc);
    if(_groupModel.createGroup(group)){
        _groupModel.addGroup(userid,group.getId(),"creator");
    }
}

// 加入群组
void ChatService::addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time){
    int userid=js["id"].get<int>();
    int groupid=js["groupid"].get<int>();

    // 加入群组
    _groupModel.addGroup(userid, groupid, "normal");
}

// 群组聊天
void ChatService::groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time){
    int userid=js["id"].get<int>();
    int groupid=js["groupid"].get<int>();
    vector<int> uservec=_groupModel.queryGroupUsers(userid, groupid);

    // 群组用户列表
    lock_guard<mutex> lock(_connMutex);
    for(int id:uservec){
        auto it=_userConnMap.find(id);
        if(it!=_userConnMap.end()){
            // 对方在线，转发消息
            it->second->send(js.dump());
        }else{
            // 查询对方是否在线
            User user=_usermodel.query(id);
            if(user.getState()=="online"){
                // 向 Kafka 发布消息
                _kafka.publish(to_string(id), js.dump());
                continue;
            }
            // 对方不在线，存储离线消息
            _offlineMsgModel.insert(id, js.dump());
        }
    }
}

// 处理注销登录业务
void ChatService::loginout(const TcpConnectionPtr& conn, json& js, Timestamp time){
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if(it != _userConnMap.end()){
            _userConnMap.erase(it);
        }
    }

    // 注销登录，取消订阅该用户的消息
    _kafka.unsubscribe();

    // 更新用户状态信息
    User user(userid, "", "", "offline");
    _usermodel.updateState(user);
}

void ChatService::handleKafkaMessage(const string& topic, const string& message) {
    int userid = std::stoi(topic);
    LOG_INFO << "handleKafkaMessage topic="<<topic<<" payload="<<message;
    std::lock_guard<std::mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end()) {
        // 真正有这个连接，把消息转给客户端
        it->second->send(message);
        LOG_INFO << "forward to client " << userid;
        return;
    }
    // 备用：存离线
    _offlineMsgModel.insert(userid, message);
}
