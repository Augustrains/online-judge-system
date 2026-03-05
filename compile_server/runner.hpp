//运行
#pragma once

#include<iostream>
#include<cstring>
#include<unistd.h>
#include"../comm/log.hpp"
#include"../comm/util.hpp"
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/fcntl.h>
#include<sys/wait.h>
#include<sys/time.h>
#include<sys/resource.h>


namespace ns_runner
{   
    using namespace ns_util;
    using namespace ns_log;

    class Runner
    {
    public:
        Runner(){}
        ~Runner(){}
    public:

       static void SetProclimit( int _cpu_limit,int _mem_limit)
       {
             //提供设置进程占用资源大小的接口

             //设置cpu时长
             struct rlimit cpu_rlimit;
             cpu_rlimit.rlim_max=RLIM_INFINITY;
             cpu_rlimit.rlim_cur=_cpu_limit;
             setrlimit(RLIMIT_CPU,&cpu_rlimit);

             //设置内存大小
             struct rlimit mem_rlimit;
             mem_rlimit.rlim_max=RLIM_INFINITY;
             mem_rlimit.rlim_cur=_mem_limit*1024;
             setrlimit(RLIMIT_AS,&mem_rlimit);
             

       }
        //指明文件名即可，不需要带路径和后缀
        /*************************************** 
       * 返回值 > 0，代表程序异常，返回值就是对应的信号编号
       * 返回值 = 0，代表子进程正常跑完，结果保存在对应文件中
       * 返回值 < 0,代表内部错误
       * cpu_limit:该进程运行时，可以使用的最大cpu资源上限
       * mem_limit:该程序运行时，可以使用的最大内存大小（kb）
       ******************************************** */
        static int Run(const std::string &file_name,int cpu_limit,int mem_limit)
        {   
            //程序运行：1.代码跑完，结果正确。  2.代码跑完，结果错误。 3.代码没跑完，异常了
            //run只考虑代码跑完，不管结果正确与与否，是否正确是由测试用例决定的
            //一个程序在默认启动的时候：
            //1.标准输入
            //2.标准输出
            //3.标准错误
            std::string _execute = PathUtil::Exe(file_name);
            std::string _stdin  = PathUtil::Stdin(file_name);
            std::string _stdout = PathUtil::Stdout(file_name);
            std::string _stderr = PathUtil::Stderr(file_name);
            
            umask(0);
            int _stdin_fd=open(_stdin.c_str(),O_CREAT | O_RDONLY,0644);
            int _stdout_fd=open(_stdout.c_str(),O_CREAT | O_WRONLY,0644);
            int _stderr_fd=open(_stderr.c_str(), O_CREAT | O_WRONLY,0644);
  
            if(_stdin_fd<0 || _stdout_fd<0 || _stderr_fd<0)
            {   
                LOG(ERROR)<<"运行时打开标准文件失败"<<"\n";
                //代表打开文件失败
                return -1;
            } 
            
            
            pid_t pid=fork();
            if(pid<0){
              LOG(ERROR)<<"运行时创建子进程失败"<<"\n";
              close(_stdin_fd);
              close(_stdout_fd);
              close(_stderr_fd);
              
              //代表创建子进程失败
              return -2;
            }
            else if(pid==0){
            //形成的临时数据
               dup2(_stdin_fd,0);
               dup2(_stdout_fd,1);
               dup2(_stderr_fd,2);
               //重定向，使得结果最后在对应文件中

               SetProclimit(cpu_limit,mem_limit);
               execl(_execute.c_str()/*代表执行谁*/,_execute.c_str()/*在命令行上如何执行该程序*/,nullptr);
               exit(1);
            }
            else{
                close(_stdin_fd);
                close(_stdout_fd);
                close(_stderr_fd);
                
                int status=0;
                waitpid(pid,&status,0);
                //程序运行异常一定是因为受到了信号
                LOG(INFO)<<"运行完毕,info:"<<(status & 0x7F)<<"\n";
                return status & 0x7F; //有异常返回非零

            }
        }

    };
}