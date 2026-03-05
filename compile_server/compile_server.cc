#include "compile_run.hpp"
#include"../comm/httplib.h"

using namespace ns_compile_and_run;
using namespace httplib;


void Usage(std::string proc)
{
    std::cerr<<"Usage:"<<"\n\t"<<proc<<std::endl;
    
}
//.compile_server port
int main(int argc,char *argv[])
{   
    if(argc!=2)
    {
        Usage(argv[0]);
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
    svr.listen("0.0.0.0", atoi(argv[1])); // 启动服务

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

