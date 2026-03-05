#ifndef _MY_UTIL_
#define _MY_UTIL
#include<iostream>
#include<fstream>
#include<unistd.h>
#include<sys/stat.h>
#include<json/json.h>
#include<memory>
#include<sstream>

namespace loginutil{
   
    class FileUtil
   {
       private:
       std::string _name;//文件名
       public:
       FileUtil(const std::string name):_name(name){};
       ~FileUtil(){};

       bool Exists()
       {
          //判断文件是否存在
          int ret=access(_name.c_str(),F_OK);
          if(ret!=0)
          {  
            std::cout<<"文件不存在"<<std::endl;
            return false;
          }
          return true;
       }
       size_t Size()
       {
        //文件大小
         if(this->Exists()==false)
         {
            return 0;
         }
         
         struct stat st;
         int ret=stat(_name.c_str(),&st);
         if(ret!=0)
         {
             //获取文件失败
            std::cout<<"获取文件失败"<<std::endl;
            return 0;
         }
          
         return st.st_size;
       }
       
       bool GetContent(std::string *body)
       {
          //读文件内容到body中
          std::ifstream ifs;
          ifs.open(_name.c_str(),std::ios::binary);
            
          //判断打开是否成功
          if(ifs.is_open()==false)
          {  
            std::cout<<"打开失败"<<std::endl;
            return false;
          }
          
          size_t flen=this->Size();
          body->resize(flen);
          ifs.read( &(*body)[0] , flen);//从body[0]所处的位置开始写flen个
          if(ifs.good()==false)
          {
            std::cout<<"读取失败"<<std::endl;
            ifs.close();
            return false;
          }
          ifs.close();
          return true;
          
       }

       bool SetContent(const std::string &body)
       {
          //向文件写入数据
          std::ofstream ofs;
          ofs.open(_name.c_str(),std::ios::binary);
          if(ofs.is_open()==false)
          {
            std::cout<<"打开失败"<<std::endl;
            return false;
          }
          ofs.write(body.c_str(),body.size());
          if(ofs.good()==false)
          {
            std::cout<<"写入失败"<<std::endl;
            ofs.close();
            return false;
          }
          ofs.close();
          return true;
       }

       bool CreateDirectory()
       {
         //创建目录
          if(this->Exists())
          { 
            //已经存在，不用创建
            return true;
          }

          mkdir(_name.c_str(),0777);
          return true;
       }
   };

   class JsonUtil{
    public:
    static bool Serialize(const Json::Value &value,std::string &body)
    {
        //序列化
        Json::StyledWriter writer;
        body= writer.write(value);
        // 检查序列化结果是否为空或无效
    if (body.empty() || body == "null")
     {
        return false;  // 序列化失败
    }
        return true;
    }

    static bool UnSerialize(const std::string &body,Json::Value &value)
    {
       Json::Reader reader;
       bool parsingSuccessful = reader.parse(body, value);
       return parsingSuccessful;  // 直接返回 parse() 的结果
    }
   };

  
 
}
#endif