#ifndef GROUP_H
#define GROUP_H

#include "groupuser.hpp"
#include <string>
#include <vector>
using namespace std;

class Group
{
    public:
        Group(int id, string name, string desc)
            : id(id), name(name), desc(desc) {}

        void setId(int id) { this->id = id; }
        void setName(string name) { this->name = name; }
        void setDesc(string desc) { this->desc = desc; }

        int getId() const { return id; }
        string getName() const { return name; }
        string getDesc() const { return desc; }
        vector<GroupUser> getUsers() const { return users; }
    private:
        int id;
        string name;
        string desc;
        vector<GroupUser> users;
};


#endif