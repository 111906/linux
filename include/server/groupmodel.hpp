#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.hpp"
#include <string>
#include <vector>
using namespace std;

//维护族群信息的操作类
class GroupModel
{
public:
    //创建群组
    bool createGroup(Group &group);
    //加入群组
    void addGroup(int userid, int groupid, string role);
    //查询用户所在的群组列表
    vector<Group> queryGroups(int userid);
    //查询群组用户列表
    vector<int> queryGroupUsers(int userid,int groupid);
};

#endif