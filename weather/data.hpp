#ifndef _MY_DATA_
#define _MY_DATA_
#include "./util.hpp"
#include "/root/my/include/mysql.h" // 使用绝对路径
#include <mutex>
#include <thread>
#include<cstdlib>

namespace historydata
{

#define HOST "8.140.17.222"
#define USER "weather"
#define PASS "x36tfGjDaMfcHHkw"
#define NAME "weather"
    static MYSQL* MysqlInit()
    {
        // 初始化操作句柄
        MYSQL *mysql = mysql_init(nullptr);

        // 连接服务器

        // 句柄，主机，用户，密码，数据库名称，端口，通道，默认
        if (mysql_real_connect(mysql, HOST, USER, PASS, NAME, 0, nullptr, 0) == nullptr)
        {
            std::cout << "连接失败！" << std::endl;
            return nullptr;
        }

        // 设置客户端字符集
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

    static bool MysqlQuery(MYSQL *mysql, const std::string &sql)
    {
        int ret = mysql_query(mysql, sql.c_str());
        if (ret != 0)
        {
            // 执行失败
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

        //实现插入记录，查看记录

        bool Insert(const Json::Value &weathers)
        {  
            _mutex.lock();
            // id code  result  time
            std::string sql;
            sql.resize(1024);
            #define INSERT_VIDEO "insert into weather values( null,'%s','%s','%s','%s');"
            sprintf(&sql[0],INSERT_VIDEO,weathers["high"].asCString(),weathers["city"].asCString(),weathers["low"].asCString(),weathers["day"].asCString());
            _mutex.unlock();
            return MysqlQuery(_mysql,sql);
        }
       

        
         
        bool SelectAll(const std::string &city,Json::Value &reinform)
        {    
            std::cout<<city<<std::endl;
            
            std::string sql;
            sql.resize(1024);
            _mutex.lock();
            #define SELECTLIKE_VIDEO  "select * from  weather where city='%s;"
            sprintf(&sql[0],SELECTLIKE_VIDEO,city);
             std::cout<<sql<<std::endl;
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
            if(num_rows==0)
            {
                std::cout<<"未查找到指定题目"<<std::endl;
                _mutex.unlock();
                return false;
            }
            for(int i=0;i<num_rows;i++)
            {
                MYSQL_ROW row=mysql_fetch_row(res);
                Json::Value video;
                  // id name password
                video["id"]=atoi(row[0]);
                video["high"]=row[1];
                video["city"]=row[2];
                video["low"]=row[3];
                video["day"]=row[4];
                reinform.append(video);
            }
            mysql_free_result(res);
            _mutex.unlock();
            return true;
        }


        bool SelectOne(const std::string &city,const std::string &day,Json::Value &reinform)
        { 
            std::string sql;
            sql.resize(1024);
            _mutex.lock();
            #define SELECTLIKE_VIDEO  "select * from  weather where city='%s and day='%s';"
            sprintf(&sql[0],SELECTLIKE_VIDEO,city,day);
            std::cout<<sql<<std::endl;
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
            if(num_rows==0)
            {
                std::cout<<"未查找到指定题目"<<std::endl;
                _mutex.unlock();
                return false;
            }
            for(int i=0;i<num_rows;i++)
            {
                MYSQL_ROW row=mysql_fetch_row(res);
                Json::Value video;
                  // id name password
                  video["id"]=atoi(row[0]);
                  video["high"]=row[1];
                  video["city"]=row[2];
                  video["low"]=row[3];
                  video["day"]=row[4];
                reinform.append(video);
            }
            mysql_free_result(res);
            _mutex.unlock();
            return true;
        }
    };
}

#endif