#ifndef _MY_DATA_
#define _MY_DATA_
#include "./talkutil.hpp"
#include <mariadb/mysql.h>
#include <mutex>
#include <thread>
#include<cstdlib>
#include "../oj_server/judgeadmin.hpp"

namespace data
{

#define HOST "127.0.0.1"
#define USER "talk"
#define PASS "d4jp3LpKW476sht5"
#define NAME "talk"

    static MYSQL* MysqlInit()
    {
        MYSQL *mysql = mysql_init(nullptr);

        if (mysql_real_connect(mysql, HOST, USER, PASS, NAME, 0, nullptr, 0) == nullptr)
        {
            std::cout << "连接失败！" << std::endl;
            return nullptr;
        }

        mysql_set_character_set(mysql, "utf8");
        return mysql;
    }


    static void MysqlDestory(MYSQL *mysql)
    {
        if (mysql != nullptr)
        {
            mysql_close(mysql);
        }
        return;
    }

    static bool MysqlQuery(MYSQL *&mysql, const std::string &sql)
    {
        if (mysql_ping(mysql) != 0) {
            std::cout << "MySQL连接已断开，正在重连..." << std::endl;
            mysql_close(mysql);
            mysql = mysql_init(nullptr);
            if (mysql_real_connect(mysql, HOST, USER, PASS, NAME, 0, nullptr, 0) == nullptr) {
                std::cout << "重连失败！" << std::endl;
                return false;
            }
            mysql_set_character_set(mysql, "utf8");
            std::cout << "重连成功" << std::endl;
        }
        int ret = mysql_query(mysql, sql.c_str());
        if (ret != 0)
        {
            std::cout << "mysql的query函数失败,原因是:" << mysql_error(mysql) << std::endl;
            return false;
        }
        return true;
    }

    class TableVideo
    {
    private:
        MYSQL *_mysql;
        std::mutex _mutex;

        // 使用 mysql_real_escape_string 转义字符串，防止 SQL 注入
        std::string Escape(const std::string &str)
        {
            if (str.empty()) return "";
            std::vector<char> escaped(str.size() * 2 + 1);
            mysql_real_escape_string(_mysql, escaped.data(), str.c_str(), str.size());
            return std::string(escaped.data());
        }

    public:
         TableVideo()
        {
            _mysql=MysqlInit();
            if(_mysql==nullptr)
            {
                exit(-1);
            }
        }
        ~TableVideo()
        {
           MysqlDestory(_mysql);
        }

        bool Insertone(const Json::Value &talk)
        {
            _mutex.lock();
            std::string sql;
            sql.resize(2048);
            sprintf(&sql[0],"insert into talk values('%s','%s',null,'%s','%s');",
                Escape(talk["type"].asString()).c_str(),
                Escape(talk["name"].asString()).c_str(),
                Escape(talk["time"].asString()).c_str(),
                Escape(talk["content"].asString()).c_str());
            _mutex.unlock();
            return MysqlQuery(_mysql,sql);
        }


           bool CheckTypeExists(const std::string &type)
        {
            _mutex.lock();
            std::string sql;
            sql.resize(512);
            sprintf(&sql[0], "select count(*) from type where type='%s';",
                Escape(type).c_str());
            bool ret = MysqlQuery(_mysql, sql);
            if (!ret) {
                _mutex.unlock();
                return false;
            }
            MYSQL_RES* res = mysql_store_result(_mysql);
            if (res == nullptr) {
                _mutex.unlock();
                return false;
            }
            MYSQL_ROW row = mysql_fetch_row(res);
            bool exists = (row && atoi(row[0]) > 0);
            mysql_free_result(res);
            _mutex.unlock();
            return exists;
        }

        bool Inserttype(const Json::Value &talk)
        {
            _mutex.lock();
            std::string sql;
            sql.resize(2048);
            sprintf(&sql[0],"insert into type values('%s','%s','%s',%d);",
                Escape(talk["type"].asString()).c_str(),
                Escape(talk["time"].asString()).c_str(),
                Escape(talk["name"].asString()).c_str(),
                talk["number"].asInt());
            _mutex.unlock();
            return MysqlQuery(_mysql,sql);
        }

        bool SelectAll(Json::Value &reinform)
        {
            _mutex.lock();
            std::string sql= "select * from  type  ;";
            bool ret= MysqlQuery(_mysql,sql);
            if(ret==false)
            {
                _mutex.unlock();
                return false;
            }

            MYSQL_RES* res=mysql_store_result(_mysql);
            if(res==nullptr)
            {
                std::cout<<"保存到本地失败，原因是:"<<mysql_error(_mysql)<<std::endl;
                _mutex.unlock();
                return false;
            }
            int num_rows=mysql_num_rows(res);
            for(int i=0;i<num_rows;i++)
            {
                MYSQL_ROW row=mysql_fetch_row(res);
                Json::Value video;
                video["type"]=row[0];
                video["time"]=row[1];
                video["name"]=row[2];
                video["number"]=atoi(row[3]);
                reinform.append(video);
            }
            mysql_free_result(res);
            _mutex.unlock();
            return true;
        }


        int Selectone(Json::Value &reinform,const Json::Value talk)
        {
            _mutex.lock();
            std::string sql;
            sql.resize(2048);
            sprintf(&sql[0],"select * from talk where type='%s';",
                Escape(talk["type"].asString()).c_str());
            bool ret= MysqlQuery(_mysql,sql);
            if(ret==false)
            {
                _mutex.unlock();
                return 0;
            }

            MYSQL_RES* res=mysql_store_result(_mysql);
            if(res==nullptr)
            {
                std::cout<<"保存到本地失败，原因是:"<<mysql_error(_mysql)<<std::endl;
                _mutex.unlock();
                return 0;
            }
            int num_rows=mysql_num_rows(res);
            for(int i=0;i<num_rows;i++)
            {
                MYSQL_ROW row=mysql_fetch_row(res);
                Json::Value video;
                video["type"]=row[0];
                video["name"]=row[1];
                video["id"]=atoi(row[2]);
                video["time"]=row[3];
                video["content"]=row[4];
                reinform.append(video);
            }
            mysql_free_result(res);
            _mutex.unlock();
            return 2;
        }


        bool UpdateViewCount(std::string type)
        {
            _mutex.lock();
            std::string sql;
            sql.resize(2048);
            sprintf(&sql[0],"UPDATE type SET number = number + 1 WHERE type = '%s';",
                Escape(type).c_str());
            _mutex.unlock();
            return MysqlQuery(_mysql,sql);
         }

         bool deleteTalk( std::string type)
         {
             _mutex.lock();
            std::string sql;
            sql.resize(2048);
            sprintf(&sql[0],"delete from type WHERE type = '%s';",
                Escape(type).c_str());
            _mutex.unlock();
            return MysqlQuery(_mysql,sql);
         }

         bool delcomment(int id)
         {
             _mutex.lock();
            std::string sql;
            sql.resize(1024);
            sprintf(&sql[0],"delete from talk WHERE id = %d;",id);
            std::cout<<sql<<std::endl;
            _mutex.unlock();
            return MysqlQuery(_mysql,sql);
         }

    };
}

#endif
