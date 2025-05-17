#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

//匹配User表的ORM类
// ORM: Object Relational Mapping 对象关系映射
// 1. 数据库表和类之间的映射关系
// 2. 数据库表的字段和类的成员变量之间的映射关系
// 3. 数据库表的记录和类的对象之间的映射关系
class User
{
    public:
        User(int id = -1, string name = "", string password = "", string state = "offline")
            : id(id), name(name), password(password), state(state) {}

        int getId() const { return id; }
        string getName() const { return name; }
        string getPassword() const { return password; }
        string getState() const { return state; }

        void setId(int id) { this->id = id; }
        void setName(string name) { this->name = name; }
        void setPassword(string password) { this->password = password; }
        void setState(string state) { this->state = state; }
    private:
        int id;
        string name;
        string password;
        string state;
        
};

#endif