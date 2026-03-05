//形成网络服务
#include<iostream>
#include"../comm/httplib.h"
#include "oj_control.hpp"
#include "/root/OnlineJudge/oj_server/judgeadmin.hpp" 
#include "/root/OnlineJudge/oj_server/userfind.hpp"
#include<signal.h>
#include"apiutil.hpp"
#include <cstdlib> // 包含 system 函数
#include<fstream>
#include"/root/video/data.hpp"
#define INPUT_FILE "/root/OnlineJudge/oj_server/what.txt"
#define FILE_PATH  "/root/OnlineJudge/oj_server/answer.txt"


using namespace httplib;
using namespace ns_control;
using namespace apiutil;
using namespace judgeadmin;

Control* ctrlPtr;

// 传统的 C 风格信号处理函数
void signalHandler(int signum) {
    if (ctrlPtr) {
        ctrlPtr->RecoverMachine();
    }
}


void writeToInputFile(const string& content) {
  ofstream file(INPUT_FILE);
  if (!file.is_open()) {
      cerr << "无法打开输入文件: " << INPUT_FILE << endl;
      exit(1);
  }
  file << content;
  file.close();
}

// 从 FILE_PATH 读取内容到 ans
string readFromOutputFile() {
  ifstream file(FILE_PATH);
  if (!file.is_open()) {
      cerr << "无法打开输出文件: " << FILE_PATH << endl;
      exit(1);
  }
  string ans((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
  file.close();
  return ans;
}

bool clearFile(const std::string& filePath) {
  // 以输出模式和截断模式打开文件（文件不存在时会创建，存在时清空内容）
  std::ofstream file(filePath, std::ios::out | std::ios::trunc);
  
  if (!file.is_open()) { // 检查文件是否打开成功
      std::cerr << "无法打开文件: " << filePath << std::endl;
      return false;
  }
  
  // 关闭文件（ofstream 析构时也会自动关闭）
  file.close();
 // std::cout << "文件已清空: " << filePath << std::endl;
  return true;
}




int main()
{    
     Control ctrl;
     ctrlPtr = &ctrl;
     
     //信号处理函数
     signal(SIGQUIT, signalHandler);

     
    //用户请求的服务路由功能
    Server svr;

    //获取所有的题目列表

    //std::cout << "Starting server at http://localhost:8080" << std::endl;

    //加载一些没啥用的页面，如talk
    svr.Get("/talk",[&ctrl](const Request &req,Response &resp){
      std::string html;
      //ctrl.GetTalk(&html);
      //LOG(INFO)<<"测试html:"<<html<<"\n";
      ctrl.GetHtml(&html,"talk.html");
      resp.set_content(html,"text/html;charset=utf-8");
  });  

  // 处理预检请求
  svr.Options("/.*", [](const Request& req, Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    res.status = 204;
});
    
  //api  ,得到的json中只有content，把该内容放进文件中，调用python，然后把结果从文件中读取，改成json，返回出去
  svr.Post("/getanswer",[&ctrl](const Request &req,Response &rsp){
    rsp.set_header("Access-Control-Allow-Origin", "*");
    //1.得到json中的content
    Json::Value con;
    //反序列化
   if(JsonUtil::UnSerialize(req.body,con)==false)
   {   
      rsp.status=400;
      rsp.body=R"({"result":false,"reason": "新的信息格式解析失败"})";
      rsp.set_header("Content-Type","application/json");
      return;
   };
   std::string content=con["content"].asCString();
   std::string ans;
   ans.resize(1024);
    //2.输入到文件中
    writeToInputFile(content);
    //3.调用python

   int result=0;
    if (result == 0) {
        ans=readFromOutputFile();
      //  std::cout<<"测试输出:"<<ans<<std::endl;
        //清空文件
        clearFile(FILE_PATH);
        clearFile(INPUT_FILE);
        Json::Value inform;
        inform["content"]=ans;
        //返回json
        JsonUtil::Serialize(inform,rsp.body);
        rsp.set_header("Content-Type","application/json");
    } else {
        std::cout << "app.py 执行失败，错误码：" << result << std::endl;
        rsp.status=400;
        rsp.body=R"({"result":false,"reason": "无法调用python"})";
        rsp.set_header("Content-Type","application/json");
    }
   
});
  
    //compete
  svr.Get("/compete",[&ctrl](const Request &req,Response &resp){
    std::string html;
    //ctrl.GetTalk(&html);
    //LOG(INFO)<<"测试html:"<<html<<"\n";
    ctrl.GetHtml(&html,"compete.html");
    resp.set_content(html,"text/html;charset=utf-8");
});
   //job
  svr.Get("/job",[&ctrl](const Request &req,Response &resp){
  std::string html;
  //ctrl.GetTalk(&html);
  //LOG(INFO)<<"测试html:"<<html<<"\n";
  ctrl.GetHtml(&html,"job.html");
  resp.set_content(html,"text/html;charset=utf-8");
});

  


    svr.Get("/all_questions",[&ctrl](const Request &req,Response &resp){
        //返回一张包含有全部题目的网页
        std::string html;
        ctrl.AllQuestions(&html);
      //  std::cout << "Received request at /all_questions" << std::endl; // 添加日志
        resp.set_content(html,"text/html;charset=utf-8");
      
     //   cout<<"success to create"<<endl;
    });
    //用户要根据对应的题目编号，获取题目的内容
    //question/100 ->  正则匹配
    //R"()",保持字符串raw string 原貌，不用做相关的转义
    svr.Get(R"(/question/(\d+))" ,[&ctrl](const Request &req,Response &resp){
      std::string number=req.matches[1];   
      std::string html;
      ctrl.Question(number,&html);
      resp.set_content(html,"text/html; charset=utf-8");
    });

    //用户提交代码，使用判题功能 (1.每道题的测试用例，2.compile_and_run)
    svr.Post(R"(/judge/(\d+))",[&ctrl](const Request &req,Response &resp){
        std::string number=req.matches[1];  
        std::string result_json;
         ctrl.Judge(number,req.body,&result_json);
         resp.set_content(result_json,"application/json;charset=uft-8");
       // resp.set_content("指定的题目编号:"+number,"text/plain;charset=utf-8");

    });

   svr.Post("/addquestion",[&ctrl](const Request &req,Response &rsp){
     //生成一个question格式的变量
       //1.拆json
      Json::Value con;
   if(JsonUtil::UnSerialize(req.body,con)==false)
   {   
      rsp.status=400;
      rsp.body=R"({"result":false,"reason": "新的信息格式解析失败"})";
      rsp.set_header("Content-Type","application/json");
      return;
   };
       //2.得到对应question
    ns_model::Question newq;
    newq.number=con["number"].asCString();
    newq.title=con["title"].asCString();
    newq.star=con["star"].asCString();
    newq.cpu_limit=con["cpu_limit"].asInt();
    newq.mem_limit=con["mem_limit"].asInt();
    newq.desc=con["desc"].asCString();
    newq.header=con["header"].asCString();
    newq.tail=con["tail"].asCString();
     //调用对应函数
     bool ret=ctrl.AddQuestion(newq);
     //判断结果

     if(ret==true)
     {
      rsp.status=200;
      rsp.body=R"({"result":false,"reason": "添加成功"})";
      rsp.set_header("Content-Type","application/json");
      return;
     }
     else
     {
      rsp.status=400;
      rsp.body=R"({"result":false,"reason": "当前编号已存在"})";
      rsp.set_header("Content-Type","application/json");
      return;
     }
    
    } );
    

    svr.Post("/delquestion",[&ctrl](const Request &req,Response &rsp){   
      Json::Value con;
   if(JsonUtil::UnSerialize(req.body,con)==false)
   {   
      rsp.status=400;
      rsp.body=R"({"result":false,"reason": "新的信息格式解析失败"})";
      rsp.set_header("Content-Type","application/json");
      return;
   };
      std::string number=con["number"].asCString();
      bool ret=ctrl.DeleteQuestion(number);
        if(ret==true)
     {
      rsp.status=200;
      rsp.body=R"({"result":false,"reason": "删除成功"})";
      rsp.set_header("Content-Type","application/json");
      return;
     }
     else
     {
      rsp.status=400;
      rsp.body=R"({"result":false,"reason": "要删除的编号不存在"})";
      rsp.set_header("Content-Type","application/json");
      return;
     }
     } );
    svr.set_base_dir("./wwwroot");
     
    //判断是不是管理员用户
    svr.Post("/checkadmin", [](const Request &req, Response &rsp) {
    Json::Value body;
    if (!JsonUtil::UnSerialize(req.body, body)) {
        rsp.status = 400;
        rsp.body = R"({"isAdmin":false})";
        return;
    }
    std::string username = body["name"].asString();
    bool isAdmin = ojadmin().IsRoot(username);
    rsp.set_header("Content-Type", "application/json");
     rsp.body = std::string(R"({"isAdmin":)") + (isAdmin ? "true" : "false") + "}";
});



svr.Get("/user.html", [](const Request &req, Response &rsp) {
    std::string userhtml;
    ns_util::FileUtil().ReadFile("./template.html/user.html",&userhtml);
    rsp.set_content(userhtml , "text/html");
});


// 获取用户列表接口
svr.Get("/getusers", [](const Request &req, Response &rsp) {
    Json::Value users;
    std::cout<<"获取用户列表"<<std::endl;
    bool ret =userfind::ojadmin().SelectLike(users);
     JsonUtil::Serialize(users,rsp.body);
     rsp.set_header("Content-Type","application/json");
});



//根据题目名称，返回对应id号，然后根据id号和name组成查看题解视频的url
svr.Post("/getid", [](const Request &req, Response &rsp) {
    Json::Value con;
      Json::Value users;
    if(JsonUtil::UnSerialize(req.body,con)==false)
    {  
      users["code"]=400;
      JsonUtil::Serialize(users,rsp.body);
      rsp.status=400;
      rsp.set_header("Content-Type","application/json");
      return;
    }  
     std::string title = con["title"].asString();  
      int id=  aoddata::TableVideo().getid(title);
      if(id!=0)
      {  
           users["id"]=id;
           users["code"]=200;
        JsonUtil::Serialize(users,rsp.body);
        rsp.set_header("Content-Type","application/json");
      }
      else
      {
        users["code"]=500;
      rsp.status=500;
      JsonUtil::Serialize(users,rsp.body);
      rsp.set_header("Content-Type","application/json");
      return;
      }
     
});


    svr.listen("0.0.0.0",8080); 
    return 0;
}