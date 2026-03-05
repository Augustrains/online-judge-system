#ifndef _MY_DATA_
#define _MY_DATA_
#include "./talkutil.hpp"
#include "/root/my/include/mysql.h" // 使用绝对路径
#include <mutex>
#include <thread>
#include<cstdlib>
#include "/root/OnlineJudge/oj_server/judgeadmin.hpp"

namespace data
{

#define HOST "8.140.17.222"
#define USER "talk"
#define PASS "d4jp3LpKW476sht5"
#define NAME "talk"

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
        bool Insertone(const Json::Value &talk)
        {  
            _mutex.lock();
            // name id  time content
            std::string sql;
            sql.resize(1024);
            #define INSERT_VIDEO "insert into talk values('%s','%s',null,'%s','%s');"
            sprintf(&sql[0],INSERT_VIDEO,talk["type"].asCString(),talk["name"].asCString(),talk["time"].asCString(),talk["content"].asCString());
            _mutex.unlock();
            return MysqlQuery(_mysql,sql);
        }


           bool Inserttype(const Json::Value &talk)
        {  
            _mutex.lock();
            // name id  time content
            std::string sql;
            sql.resize(1024);
            #define INSERT_VIDEO "insert into type values('%s','%s','%s',%d);"
            sprintf(&sql[0],INSERT_VIDEO,talk["type"].asCString(),talk["time"].asCString(),talk["name"].asCString(),talk["number"].asInt());
            _mutex.unlock();
            return MysqlQuery(_mysql,sql);
        }
       
         //查看所有帖子
        bool SelectAll(Json::Value &reinform)
        { 
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
            if(num_rows==0)
            {
                std::cout<<"当前没有帖子"<<std::endl;
                _mutex.unlock();
                return false;
            }
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
      
        
        //查看某个帖子
        int Selectone(Json::Value &reinform,const Json::Value talk)
        {    
            std::string sql;
            sql.resize(1024);
            #define SELECTLIKE_VIDEO  "select * from talk where type='%s';"
            sprintf(&sql[0],SELECTLIKE_VIDEO,talk["type"].asCString());
            //std::cout<<sql<<std::endl;
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
            if(num_rows==0)
            {
               // std::cout<<"未查找到帖子"<<std::endl;
                _mutex.unlock();
                return 1 ;
            }
            for(int i=0;i<num_rows;i++)
            {
                MYSQL_ROW row=mysql_fetch_row(res);
                Json::Value video;
                  // id name password
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
            // name id  time content
            std::string sql;
            sql.resize(1024);
            #define INSERT_VIDEO "UPDATE type SET number = number + 1 WHERE type = '%s';"
            std::cout<<sql<<std::endl;
            
            sprintf(&sql[0],INSERT_VIDEO,type.c_str());
           // std::cout<<sql<<std::endl;
            _mutex.unlock();
            return MysqlQuery(_mysql,sql);
         }

         bool deleteTalk( std::string type)
         {
             _mutex.lock();
            std::string sql;
            sql.resize(1024);
            #define del "delete from type WHERE type = '%s';"
            sprintf(&sql[0],del ,type.c_str());
           // std::cout<<sql<<std::endl;
           // std::cout<<sql<<std::endl;
            _mutex.unlock();
            return MysqlQuery(_mysql,sql);
         }

         bool delcomment(int id)
         {
             _mutex.lock();
            std::string sql;
            sql.resize(1024);
            #define del "delete from talk WHERE id = %d;"
            sprintf(&sql[0],del ,id);
            std::cout<<sql<<std::endl;
            _mutex.unlock();
            return MysqlQuery(_mysql,sql);
         }

    };
}

#endif