#include "compile_run.hpp"
#include"../comm/httplib.h"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sched.h>
#include <unistd.h>

using namespace ns_compile_and_run;
using namespace httplib;


void Usage(std::string proc)
{
    std::cerr << "Usage:" << "\n\t" << proc << " <port> <cpu_id>" << std::endl;
    
}

static bool BindCpu(int cpu_id)
{
    long cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpu_count <= 0)
    {
        std::cerr << "failed to detect online cpu count" << std::endl;
        return false;
    }
    if (cpu_id < 0 || cpu_id >= cpu_count)
    {
        std::cerr << "cpu_id out of range, valid range is [0, " << cpu_count - 1 << "]" << std::endl;
        return false;
    }

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu_id, &mask);
    if (sched_setaffinity(0, sizeof(mask), &mask) != 0)
    {
        std::cerr << "sched_setaffinity failed: " << std::strerror(errno) << std::endl;
        return false;
    }
    return true;
}

//.compile_server port cpu_id
int main(int argc,char *argv[])
{   
    if(argc!=3)
    {
        Usage(argv[0]);
        return 1;
    }

    int port = std::atoi(argv[1]);
    int cpu_id = std::atoi(argv[2]);
    if (port <= 0)
    {
        std::cerr << "invalid port: " << argv[1] << std::endl;
        return 1;
    }
    if (!BindCpu(cpu_id))
    {
        return 1;
    }

    Server svr;
    pid_t server_pid = getpid();
    // 添加日志
   // std::cout << "Starting server at http://localhost:8080" << std::endl;

    svr.Get("/hello", [](const Request &req, Response &resp) {
        std::cout << "Received request at /hello" << std::endl; // 添加日志
        resp.set_content("hello httplib, 你好httplib", "text/plain;charset=utf-8");
    });

    svr.Get("/get_pid", [server_pid](const httplib::Request&, httplib::Response& res) {
        res.set_content(std::to_string(server_pid), "text/plain");
    });

    
    svr.Post("/compile_and_run", [](const Request &req, Response &resp){
        // ⽤⼾请求的服务正⽂是我们想要的json string 
        //LOG(INFO)<<"编译器成功接收到请求"<<"\n";
        std::string in_json = req.body;
        std::string out_json;
        out_json.resize(2048);
        if(!in_json.empty()){

            try {
                ComPileAndRun::Start(in_json, &out_json);
                resp.set_content(out_json, "application/json;charset=utf-8");
                LOG(INFO)<<"-------------请求编译服务成功---------"<<std::endl;
                //std::cout<<out_json<<"\n";
            } catch (const std::exception &e) {
                out_json = R"({"status": -1, "reason": "内部错误"})";
                LOG(ERROR) << "编译异常: " << e.what();
            }       
        }
        });
    //svr.set_base_dir("./wwwroot");
    svr.listen("0.0.0.0", port); // 启动服务

}



    // // 构建一个json测试
    // //
    // std::string in_json;
    // Json::Value in_value;
    // // R"()", raw string ,使得特殊字符保持原貌
    // in_value["code"]=R"(#include<iostream>
    // int main(){
    // std::cout<<"hello world"<<std::endl;
    // return 0;
    // })";
    // //aaaaaaaa
    // //int a=10; a/=0;
    // //while(1);
    // //int *p[1024*1024*1024];
    // //错误用例测试
    // in_value["input"]="";
    // in_value["cpu_limit"]=1;
    // in_value["mem_limit"]=10240*30;//30kb
    
    // Json::FastWriter writer; 
    // in_json=writer.write(in_value);
    // std::cout<<in_json<<std::endl;
    
    // std::string out_json;//将来给客户端返回的json
    // ComPileAndRun::Start(in_json,&out_json);

    // std::cout<<out_json<<std::endl;
    // return 0;
