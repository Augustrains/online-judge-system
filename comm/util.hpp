#pragma once
#include<iostream>
#include<string>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/time.h>
#include<atomic>
#include<fstream>
#include<boost/algorithm/string.hpp>
#include<vector>

namespace ns_util
{     
    class TimeUtil
    {
        public:
        static std::string GetTimeStamp()
        {   
            struct timeval _time;
            gettimeofday(&_time,nullptr);
            return std::to_string(_time.tv_sec);
        }

        static std::string GetTimeMs()
        {
            //获得毫秒时间戳
            
            struct  timeval  _time;
    
            
            gettimeofday(&_time,nullptr);
            return std::to_string( _time.tv_sec*1000+_time.tv_usec/1000);  

        }
    };
    const std::string temp_path="./temp/";  
    class PathUtil
    {   
        static std::string Addsuffix(const std::string &file_name,const std::string &suxxif)
        {
            std::string path_name=temp_path;
            path_name+=file_name;
            path_name+=suxxif;
            return path_name;
        }
        public:
         //构建源文件路径+后缀的完整文件名
         //1234->./temp/1234.cpp
       
         //编译时的临时文件

        static std::string Src(const std::string &file_name)
        {
            return Addsuffix(file_name,".cpp");
        }
        //构建可执行程序的完整路径+后缀名
        static std::string Exe(const std::string &file_name)
        {
            return Addsuffix(file_name,".exe");
        }

        static std::string CompilerError(const std::string &file_name)
        {
            return Addsuffix(file_name,".compile_error");
        }
        
        /////////////////////////////////////////////////////////////////
        //运行时的临时文件
        static std::string Stdin(const std::string &file_name)
        {
            return Addsuffix(file_name,".stdin");
        }

        static std::string Stdout(const std::string &file_name)
        {
            return Addsuffix(file_name,".stdout");
        }

        //构建该程序对应的标准错误完整路径+后缀名
        static std::string Stderr(const std::string &file_name)
        {
            return Addsuffix(file_name,".stderr");
        }

    };

    class FileUtil
    {
      public:
      static bool IsFileExists(const std::string &path_name)
      {
          struct stat st;
          if(stat(path_name.c_str(),&st)==0)
          {
            //获取文件属性成功，文件已经存在
            return true;
          }

          return false;
      }

      static std::string  UniqFileName()
      { 
        //形成唯一文件名
        static std::atomic_uint id(0);//原子性递增
        id++;
        std::string uniq_id=std::to_string(id);
        std::string ms=TimeUtil::GetTimeMs();//时间戳    
        return ms+"_"+uniq_id;
      }

      static bool WriteFile(const std::string & target,const std::string &content)
      {
        //将content写入target
        std::ofstream out(target);
        if(!out.is_open())
        {
            return false;
        }
        
        out.write(content.c_str(),content.size());
        out.close();
        return true;
        
      }
      static bool  ReadFile(const std::string &target,std::string*content,bool keep=false)
      {
        (*content).clear();

        std::ifstream in(target);
        if(!in.is_open()){
          return false;
        }
        std::string line;
        //getline 不保存行分隔符,但有些时候需要保存如\n,用keep判断
        //getline内部重载了强制类型转化
        while(std::getline(in,line))
        {
           (*content)+=line;
           (*content)+=(keep  ? "\n" :"");
        }
        in.close();
        return true;

      }
    };
    
    class StringUtil
    {
     public:
        /*
        str:输入型，目标要切分的字符串
        target:输出型，保存被切分后的各个字符串
        sep:指定的分隔符
        */
        static void SplitString(const std::string&str,std::vector<std::string> *target,std::string sep)
        {
            //boost 来切分
            boost::split((*target),str,boost::is_any_of(sep),boost::algorithm::token_compress_on);

        }
    };
   
}