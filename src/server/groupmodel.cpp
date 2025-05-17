#include"../../include/server/groupmodel.hpp"
#include "../../include/server/db/db.h"



    //创建群组
    bool GroupModel::createGroup(Group &group){
        char sql[1024]={0};
        sprintf(sql, "insert into allgroup(groupname,groupdesc) values('%s', '%s')", group.getName().c_str(), group.getDesc().c_str());
        MySQL mysql;
        if(mysql.connect()){
            if(mysql.update(sql)){
                return true;
            }
        }
        return false;
    }

    //加入群组
    void GroupModel::addGroup(int userid, int groupid, string role){
        char sql[1024]={0};
        sprintf(sql, "insert into groupuser values(%d, %d, '%s')", userid, groupid, role.c_str());
        MySQL mysql;
        if(mysql.connect()){
            mysql.update(sql);
        }
    }

    //查询用户所在的群组列表
    vector<Group> GroupModel::queryGroups(int userid){
        char sql[1024]={0};
        sprintf(sql, "select g.id, g.groupname, g.groupdesc from groupuser gu inner join allgroup g on gu.groupid=g.id where gu.userid=%d", userid);
        MySQL mysql;
        vector<Group> groups;
        if(mysql.connect()){
            MYSQL_RES* res=mysql.query(sql);
            if(res!=nullptr){
                MYSQL_ROW row;
                while((row=mysql_fetch_row(res))!=nullptr){
                    Group group(atoi(row[0]), row[1], row[2]);
                    groups.push_back(group);
                }
                mysql_free_result(res);
            }
        }
        //查询群组用户列表
        for(auto& group:groups){
            sprintf(sql, "select a.id,a.name,a.state,b.grouprole from user a inner join groupuser b on b.userid=a.id where b.groupid=%d", group.getId());
            MYSQL_RES* res=mysql.query(sql);
            if(res!=nullptr){
                MYSQL_ROW row;
                while((row=mysql_fetch_row(res))!=nullptr){
                    GroupUser user;
                    user.setId(atoi(row[0]));
                    user.setName(row[1]);
                    user.setState(row[2]);
                    user.setRole(row[3]);
                    group.getUsers().push_back(user);
                }
                mysql_free_result(res);
            }
        }
        return groups;
    }

    //查询群组用户列表
    vector<int> GroupModel::queryGroupUsers(int userid,int groupid){
        char sql[1024]={0};
        sprintf(sql, "select userid from groupuser where groupid=%d and userid!=%d", groupid,userid);
        MySQL mysql;
        vector<int> users;
        if(mysql.connect()){
            MYSQL_RES* res=mysql.query(sql);
            if(res!=nullptr){
                MYSQL_ROW row;
                while((row=mysql_fetch_row(res))!=nullptr){
                    users.push_back(atoi(row[0]));
                }
                mysql_free_result(res);
            }
        }
        return users;
    }