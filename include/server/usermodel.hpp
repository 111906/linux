#ifndef USERMODEL_H
#define USERMODEL_H
#include "user.hpp"

//user表的数据操作类
class UserModel
{
    public:
        // 添加用户
        bool insert(User& user); 
        //根据id查询用户信息
        User query(int id);
        //更新用户状态
        bool updateState(User& user);
        //重置用户状态
        void resetState();
};


#endif