#pragma once
#include "/root/my/include/mysql.h" // 使用绝对路径
#include<iostream>
namespace userfind
{
    static MYSQL* MysqlInit()
    {
        // 初始化操作句柄
        MYSQL *mysql = mysql_init(nullptr);

        // 连接服务器
        // 句柄，主机，用户，密码，数据库名称，端口，通道，默认
        if (mysql_real_connect(mysql, HOST, "login_system", "p8mfrbfFk3kWSSfC", "login_system", 0, nullptr, 0) == nullptr)
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


    class ojadmin
    {
    private:
        MYSQL *_mysql;
        std::mutex _mutex;
    public:
         ojadmin() 
        {
            _mysql=MysqlInit();
            if(_mysql==nullptr)
            {
                exit(-1);
            }
        }
        ~ojadmin() 
        {
           MysqlDestory(_mysql);
        }
        
       bool SelectLike(Json::Value &reinform)
        { 
            std::string sql;
            sql.resize(1024); 
            _mutex.lock();
            #define SELECTLIKE_VIDEO  "select * from  login ;"
            sprintf(&sql[0],SELECTLIKE_VIDEO);
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
                video["name"]=row[0];
                video["password"]=row[1];
                reinform.append(video);
            }
            mysql_free_result(res);
            _mutex.unlock();
            return true;
        }


        bool  deleteone(std::string names)
        {
            std::string sql;
            sql.resize(1024);//防止简介太长了
             _mutex.lock();
            #define DELETE_VIDEO "delete from login where name='%s'"
            sprintf(&sql[0],DELETE_VIDEO ,names.c_str());
            //std::cout<<sql<<std::endl;
              _mutex.unlock();
            return MysqlQuery(_mysql,sql);
        }
    };
}
