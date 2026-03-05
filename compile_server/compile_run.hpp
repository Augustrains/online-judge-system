//整合编译和运行
#pragma once

#include"compiler.hpp"
#include"runner.hpp"
#include<json/json.h>
#include<string>
#include"../comm/log.hpp"
#include"../comm/util.hpp"
#include<signal.h>
#include<unistd.h>


namespace ns_compile_and_run
{    
  


     using namespace ns_log;
     using namespace ns_util;
      using namespace ns_compiler;
      using namespace ns_runner;
     class ComPileAndRun{
          public :

          //code>0:进程收到了信号导致异常崩溃
          //code<0:整个过程非运行报错（包括代码为空，编译报错等）
          //code==0:整个过程无误，可以得到结果
          static std::string CodeToDesc(int code,const std::string &file_name) //将数字转化成描述
          {  
            std::string desc; //待完善
            switch (code)
            {
            case 0:
            desc="编译和运行成功";
            break;

            case -1:
            desc="代码为空";
            break;

            case -2:
            desc="未知错误";
            break;

            case -3:
            FileUtil::ReadFile(PathUtil::CompilerError(file_name),&desc,true );//编译错误
            break; 

            case SIGABRT:
            desc="内存超出限制";  //6
            break;

            case SIGXCPU:
            desc="CPU超时"; //24
            break;

            case SIGFPE:
            desc="浮点数溢出"; //8
            break;  

            default:
            desc="未知错误:"+std::to_string(code);
            break;
            }
            
           
            return desc;
          }

          static void  RemoveTempFile(const std::string &file_name)
          {
                 //需要清理的文件数目不确定，但可能的类型是确定的
                 std::string src=PathUtil::Src(file_name);
                 if(FileUtil::IsFileExists(src))    unlink(src.c_str());

                 std::string compiler_error=PathUtil::CompilerError(file_name);
                 if(FileUtil::IsFileExists(compiler_error)) unlink(compiler_error.c_str());

                 std::string exe=PathUtil::Exe(file_name);
                 if(FileUtil::IsFileExists(exe))  unlink(exe.c_str());

                 std::string _stdin=PathUtil::Stdin(file_name);
                 if(FileUtil::IsFileExists(_stdin))  unlink(_stdin.c_str());

                 std::string _stdout=PathUtil::Stdout(file_name);
                 if(FileUtil::IsFileExists(_stdout))unlink(_stdout.c_str());

                 std::string _stderr=PathUtil::Stderr(file_name);
                 if(FileUtil::IsFileExists(_stderr)) unlink(_stderr.c_str());
          }
          /*
          
          输入：
          input:用户提交的代码对应的输入，不做处理
          code:用户提交的代码
          cpu_limit:时间要求
          mem_limit:空间要求 

          输出：
          必填
          status:状态码
          reason:请求结果
          
          选填
          stdout:我的程序运行完的结果
          stderr:我的程序运行完的错误结果

          参数：
          in_json:{"code":"#include....","input":"12345...","cpu_limit":"1s","mem_limit":"10240"} 
          out_json;{"status":"0","reason":"", "stdout":"","stderr":""}
          */
          static void Start(const std::string &in_json, std::string * out_json)
          {
               //将json解析成多个kv
               Json::Value in_value;
               Json::Reader reader;
               reader.parse(in_json,in_value);//后期会有处理差错问题


               std::string code=in_value["code"].asString();
               std::string input=in_value["input"].asString();
               int cpu_limit=in_value["cpu_limit"].asInt();
               int mem_limit=in_value["mem_limit"].asInt();
                

               int  status_code=0;
               Json::Value out_value;
               int run_result;
           
               std::string file_name;
               if(code.size()==0)
               {
                     //差错处理
                    status_code=-1;//代码为空
                    goto END;

               }

               //形成的文件名只具有唯一性，没有目录和后缀
               //思路：毫秒级时间戳+原子性递增唯一值：保证唯一性
               file_name=FileUtil::UniqFileName();

                if(!FileUtil::WriteFile(PathUtil::Src(file_name),code))//形成临时src文件
                {
                   status_code=-2;//未知错误
                   goto END;
                }

               if(!Compiler::Compile(file_name))
               {
                    status_code=-3;//代码编译时发生错误
                   goto END;
               }

               run_result=Runner::Run(file_name,cpu_limit,mem_limit);    

               
               if(run_result<0)
               {
                    status_code=-2;//未知错误
               }
               else if(run_result>0)    
               {
                    status_code=run_result;//程序运行崩溃
                    //序列化
                    
               }
               else
               {
                    //运行成功
                    status_code=0;
                  
                }

           END:
               //status_code
               out_value["status"] = status_code;
               std::string desc=CodeToDesc(status_code, file_name);
               out_value["reason"] = desc;
 
               if(status_code==0)
               {
                    //整个过程全部成功
                    std::string _stdout;
                    FileUtil::ReadFile(PathUtil::Stdout(file_name),&_stdout,true );
                    out_value["stdout"]=_stdout;
                    std::string _stderr;
                    FileUtil::ReadFile(PathUtil::Stderr(file_name),&_stderr,true );
                    out_value["stderr"]=_stderr;
               } 

            
               //序列化
               Json::StreamWriterBuilder writer;
               writer.settings_["indentation"] = "    "; // 设置缩进
               writer.settings_["emitUTF8"] = true;      // 启用 UTF-8 输出，禁用 Unicode 转义
               (*out_json)=Json::writeString(writer, out_value);
               
               RemoveTempFile(file_name);//清空临时文件
          }
    
     };
} // namespace ns_compile_and_run
