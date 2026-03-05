#ifndef _MY_DATA_
#define _MY_DATA_
#include "./util.hpp"
#include "/root/my/include/mysql.h" // 使用绝对路径
#include <mutex>
#include <thread>
#include<vector>
#include<cstdlib>
#include<fstream>
#include "/root/OnlineJudge/comm/util.hpp";

using namespace ns_util;
using namespace std;
namespace logindata
{

#define HOST "8.140.17.222"
#define USER "login_system"
#define PASS "p8mfrbfFk3kWSSfC"
#define NAME "login_system"
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

        bool Insert(const Json::Value &video)
        {  
            _mutex.lock();
            // id name password
            std::string sql;
            sql.resize(4096+video["info"].asString().size());//防止简介太长了
            #define INSERT_VIDEO "insert into login values('%s','%s');"
            sprintf(&sql[0],INSERT_VIDEO,video["name"].asCString(),video["password"].asCString());
            _mutex.unlock();
            return MysqlQuery(_mysql,sql);
        }

        bool is_exit(const Json::Value &inform)
        {   
            std::string sql;
            sql.resize(1024);
            _mutex.lock();
            #define SELECTLIKE_VIDEO  "select * from  login where name like '%s';"
            sprintf(&sql[0],SELECTLIKE_VIDEO,inform["name"].asCString());
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
                std::cout<<"用户或密码错误"<<std::endl;
                _mutex.unlock();
                return false;
            }
            _mutex.unlock();

            return true;
        }


        bool Update(const Json::Value &video)
        {   
            _mutex.lock();
            std::string sql;
            sql.resize(1024);
            #define SELECTLIKE_VIDEO  "select * from  login where name like '%s' and password like '%s' ;"
            sprintf(&sql[0],SELECTLIKE_VIDEO,video["username"].asCString(),video["oldPassword"].asCString());
           // std::cout<<sql<<std::endl;
            if(MysqlQuery(_mysql,sql)==false)
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
                std::cout<<"未找到该用户!"<<std::endl;
                _mutex.unlock();
                return false;
            }
            mysql_free_result(res);
            #define UPDATE_VIDEO "update login set password='%s' where name='%s';"
            sprintf(&sql[0],UPDATE_VIDEO ,video["newPassword"].asCString(),video["username"].asCString());
            _mutex.unlock();
            return MysqlQuery(_mysql,sql);
        }
          
        bool create_user(const Json::Value &video)
        {
            _mutex.lock();
            std::string sql;
            sql.resize(1024); 
             #define UPDATE_VIDEO "insert into  login values('%s','%s');"
            sprintf(&sql[0],UPDATE_VIDEO ,video["username"].asCString(),video["password"].asCString());
            _mutex.unlock();
            return MysqlQuery(_mysql,sql);
            
        }
    
        bool SelectLike(const Json::Value &inform,Json::Value &reinform)
        { 
            std::string sql;
            sql.resize(1024);
            _mutex.lock();
            #define SELECTLIKE_VIDEO  "select * from  login where name like '%s' and password like '%s' ;"
            sprintf(&sql[0],SELECTLIKE_VIDEO,inform["name"].asCString(),inform["password"].asCString());
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
                std::cout<<"用户或密码错误"<<std::endl;
                _mutex.unlock();
                return false;
            }
            for(int i=0;i<num_rows;i++)
            {
                MYSQL_ROW row=mysql_fetch_row(res);
                Json::Value video;
                  // id name password
                video["id"]=atoi(row[0]);
                video["name"]=row[1];
                video["password"]=row[2];
                reinform.append(video);
            }
            mysql_free_result(res);
            _mutex.unlock();
            return true;
        }


        bool questionlist(Json::Value &list)
        {
         //加载配置文件：questions/questions.list+题目编号文件
         ifstream in("/root/OnlineJudge/oj_server/questions/questions.list");
         if(!in.is_open())
         {
            //大概率文件名错了或者路径不存在
           std::cout<<" (load fail)加载题库失败,请检查是否存在题库文件"<<"\n";
            return false;
         }
        
        std::string line;
        while(getline(in,line))
        {
           //一行包括一个编号的所有信息
           vector<string> tokens;
           StringUtil::SplitString(line, &tokens, " ");
           // 1 判断回⽂数 简单 1 30000 
           if(tokens.size() != 5)
           {
           std::cout<< "加载部分题⽬失败, 请检查⽂件格式" << "\n";
           continue;
           }
          
           Json::Value  q;
           q["id"]=tokens[0];
           q["title"]=tokens[1];
           q["difficulty"]=tokens[2];
           q["timeComplexity"]=tokens[3];
           q["spaceComplexity"]=tokens[4];
           

           //例如/root/OnlineJudge/oj_server/questions/1
           std::string path="/root/OnlineJudge/oj_server/questions/";
           path+=tokens[0];
           path+="/";
           
          // std::cout<<path<<std::endl;
           std::string describe;
           describe.resize(4048);
           //std::cout<<path+"desc.txt"<<endl;
           FileUtil::ReadFile(path+"desc.txt",&describe,true);
           //std::cout<<describe<<std::endl;
           q["desc"]=describe;
           std::string headers;
           //std::cout<<path<<std::endl;
           FileUtil::ReadFile(path+"header.cpp",&headers,true);
           q["header"]=headers;
           std::string tails;
           FileUtil::ReadFile(path+"tail.cpp",&tails,true);
           q["tail"]=tails;
           list.append(q);
        }
       // LOG(INFO)<<" 加载题库...成功!(success)"<<"\n";
        in.close();
        return  true;
        }
    };
}

#endif