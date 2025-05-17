#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "user.hpp"


//群组用户，多一个role角色信息，从User类继承，复用User的其他信息
class GroupUser : public User
{
    public:
        void setRole(string role) { this->role = role; }
        string getRole() const { return role; }
    private:
        string role; //角色信息
};

#endif