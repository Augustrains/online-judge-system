#pragma once
#include "/root/my/include/mysql.h" // 使用绝对路径
#include<iostream>
namespace judgeadmin
{
    static MYSQL* MysqlInit()
    {
        // 初始化操作句柄
        MYSQL *mysql = mysql_init(nullptr);

        // 连接服务器
        // 句柄，主机，用户，密码，数据库名称，端口，通道，默认
        if (mysql_real_connect(mysql, HOST, "admin", "eT4eN2X5izwNxC2R", "admin", 0, nullptr, 0) == nullptr)
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
        
        bool IsRoot(std::string name)
        {
            _mutex.lock();
            std::string sql;
            sql.resize(200);//防止简介太长了
            #define rootis  "select * from  admin where name='%s' ;"
            sprintf(&sql[0],rootis,name.c_str());
            bool  ret= MysqlQuery(_mysql,sql);  
            if(ret==false)
            {  
                _mutex.unlock();
                return false;
            }
            MYSQL_RES* res=mysql_store_result(_mysql);
            int num_rows=mysql_num_rows(res);
            if(num_rows==0)
            {
                mysql_free_result(res);
                _mutex.unlock();
                return false;
            }           
            mysql_free_result(res);
            _mutex.unlock();
            return true;
        }
    };
}
