#pragma once
//根据题目list文件，加载所有的题目信息到内存中
//model:主要用来和数据进行交互，对外提供访问数据的接口
//mysql版本
#include<iostream>
#include<string>
#include<unordered_map>
#include<cassert>
#include"../comm/log.hpp"
#include<vector>
#include<fstream>
#include"../comm/util.hpp"
#include<cstdlib>
#include"../include/mysql.h"


namespace ns_model
{    
    using namespace std;
    using namespace ns_log;
    using namespace ns_util;
    /// @brief 
    struct Question{
        std::string number;//题目编号
        std::string title;//标题
        std::string star;//难度:简单，中等，困难
        int cpu_limit;//题目的时间要求(s)
        int mem_limit;//题目的空间要求(kb)
        std::string desc;//题目的描述
        std::string header;//题目预设给用户在线编辑器的代码
        std::string tail;//题目的测试用例，需要和header拼接，形成完整代码，再提交给后端
    };
   

    class  Model{
    private:
   //题号:题目细节
         unordered_map<string,Question>questions;
         
         std::string oj_questions="oj_questions";
         const std::string host="127.0.0.1";
         const std::string user="oj_client";
         const std::string password="123456Yy"; 
         const std::string db="oj_client";
         const  int  port= 3306;

    public:
        Model(){};
      
      bool QueryMysql(const std::string &sql,vector<struct Question>*out)
      {
            //访问数据库

            //创建mysql指针
            MYSQL *my= mysql_init(nullptr);
             
            //连接数据库
            if(nullptr==mysql_real_connect(my,host.c_str(),user.c_str(),password.c_str(),db.c_str(),port,nullptr,0))
            {
               LOG(FATAL)<<"数据库连接失败！"<<"\n";
               return false;
            } 

            mysql_set_character_set(my, "utf8");
            LOG(SUCCSEE)<<"连接数据库成功"<<"\n";
            
            //执行sql语句
            if(0!=mysql_query(my,sql.c_str()))
            {    
               LOG(WARNING)<<sql<<" 执行失败"<<"\n";
                return false;
            }
            
            //提取结果
            MYSQL_RES *res=mysql_store_result(my);

            //分析结果
            int rows=mysql_num_rows(res);
      

            for(int i=0;i<rows;i++)
            {  
               MYSQL_ROW row=mysql_fetch_row(res);
               struct Question q;

               // number title star desc header tail  cpu mem

               q.number=row[0];
               q.title=row[1];
               q.star=row[2];
               q.desc=row[3];       
               q.header=row[4];
               q.tail=row[5];
               q.cpu_limit=atoi(row[6]);
               q.mem_limit=atoi(row[7]);       

               out->push_back(q);
            }
            

            free(res); //释放结果空间
            
            //关闭链接
            mysql_close(my);

      }



      bool GetAllQuestions(vector<Question>*out){
          std::string sql="select *from ";
         sql+=oj_questions;
         return QueryMysql(sql,out);
      }

      bool GetOneQuestion(const std::string &number,Question*q){
         bool res=false;
          std::string sql="select *from ";
         sql+=oj_questions;
         sql+=" where number=";
         sql+=number;

         vector<Question> result;
         if(QueryMysql(sql,&result))
         {   
            if(result.size()==1)
            {
               *q=result[0];
               res=true;
            }
         }
        return res;
      }

      





      ~Model()
      {};
    };

}
