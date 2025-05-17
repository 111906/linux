#include"../../include/server/offlinemessagemodel.hpp"
#include "../../include/server/db/db.h"




        //存储离线消息
        void OfflineMsgModel::insert(int userid, string msg){
            char sql[1024] = {0};
            sprintf(sql, "insert into offlinemessage(userid, message) values(%d, '%s')", userid, msg.c_str());
            MySQL mysql;
            if (mysql.connect())
            {
                mysql.update(sql);
            }
        }

        //删除离线消息
        void OfflineMsgModel::remove(int userid){
            char sql[1024] = {0};
            sprintf(sql, "delete from offlinemessage where userid=%d", userid);
            MySQL mysql;
            if (mysql.connect())
            {
                mysql.update(sql);
            }
        }

        //查询离线消息
        vector<string> OfflineMsgModel::query(int userid){
            char sql[1024] = {0};
            sprintf(sql, "select message from offlinemessage where userid=%d", userid);
            MySQL mysql;
            vector<string> messages;
            if (mysql.connect())
            {
                MYSQL_RES* res = mysql.query(sql);
                if (res != nullptr)
                {
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(res)) != nullptr)
                    {
                        messages.push_back(row[0]);
                    }
                    mysql_free_result(res);
                }
            }
            return messages;
        }