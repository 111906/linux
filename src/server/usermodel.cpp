#include "../../include/server/usermodel.hpp"
#include "../../include/server/db/db.h"
#include <muduo/base/Logging.h>
#include <string>

bool UserModel::insert(User& user)
{
    char sql[1024] = {0};
    // 1. 拼接sql语句
    sprintf(sql, "insert into user(name,password,state) values('%s', '%s','%s')", user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());
    // 2. 连接数据库
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

//根据id查询用户信息
User UserModel::query(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id=%d", id);
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
        }
    }
    return User();
}

//更新用户状态
bool UserModel::updateState(User& user)
{
    char sql[1024] = {0};
    sprintf(sql, "update user set state='%s' where id=%d", user.getState().c_str(), user.getId());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

//重置用户状态
void UserModel::resetState()
{
    char sql[1024] = "update user set state='offline' where state='online'";
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}