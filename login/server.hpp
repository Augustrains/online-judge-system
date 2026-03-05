#include"./data.hpp"
#include"/root/OnlineJudge/comm/httplib.h"
#include"/root/OnlineJudge/oj_server/userfind.hpp"
#include"/root/OnlineJudge/login/gethistory.hpp"
#include<vector>
#include<string>


namespace loginserver
{  

    #define WWWROOT "./www"
    #define VIDEO_ROOT "/video/"
    #define IMAGE_ROOT "/image/"
    using namespace logindata;
    using namespace loginutil;
    using namespace std;

    TableVideo *tb_video=nullptr;
   class Server
   {
    private:
    int _port;
    httplib::Server _svr;
    
    private:
    static void Insert(const httplib::Request &req,httplib::Response &rsp)
    {    
        Json::Value video;
          //反序列化
         if(JsonUtil::UnSerialize(req.body,video)==false)
         {   
            rsp.status=400;
            rsp.body=R"({"result":false,"reason": "新的信息格式解析失败"})";
            rsp.set_header("Content-Type","application/json");
            return;
         };
        
        if(tb_video->is_exit(video)==true)
        {   
           std::cout<<"用户名已经存在"<<std::endl;
           //用户名已经存在
            rsp.status=409;
            rsp.body=R"({"result":false,"reason": "用户名已经存在"})";
            rsp.set_header("Content-Type","application/json");
            return;
        }

        if(tb_video->Insert(video)==false)
        {
            rsp.status=500;
            rsp.body=R"({"result":false,"reason": "新增数据失败"})";
            rsp.set_header("Content-Type","application/json");
            return;
        }
        
       // 提交成功后回到原本的网页
       // rsp.set_redirect("/index.html",303);
         return;
    }

    static void Update(const httplib::Request &req,httplib::Response &rsp)
    {   
        Json::Value video;
          //反序列化
         if(JsonUtil::UnSerialize(req.body,video)==false)
         {   
            rsp.status=400;
            rsp.body=R"({"result":false,"reason": "新的信息格式解析失败"})";
            rsp.set_header("Content-Type","application/json");
            return;
         };

        if(tb_video->Update(video)==false)
        {
            rsp.status=500;
            rsp.body=R"({"result":false,"reason": "修改数据库信息失败"})";
            rsp.set_header("Content-Type","application/json");
            return;
        }
    }


   

    static void SelectAll( const httplib::Request &req,httplib::Response &rsp)
    {
     
        Json::Value video;
        //反序列化
       if(JsonUtil::UnSerialize(req.body,video)==false)
       {   
          rsp.status=400;
          rsp.body=R"({"result":false,"reason": "新的信息格式解析失败"})";
          rsp.set_header("Content-Type","application/json");
          return;
       };


       Json::Value inform;
     if(tb_video->SelectLike(video,inform)==false)
    {
     rsp.status=500;
     rsp.body=R"({"result":false,"reason":"用户名或者密码错误,请重试"})";
     rsp.set_header("Content-Type","application/json");
     return;
    }
    
    //组织响应正文
    JsonUtil::Serialize(inform,rsp.body);
    rsp.set_header("Content-Type","application/json");
    return ;

    } 

    public:
    Server(int port):_port(port){};

    std::string readFile(const std::string& filename) {
      std::ifstream file(filename);
      if (!file.is_open()) {
          std::cerr << "Failed to open file: " << filename << std::endl;
          return "";
      }
      std::stringstream buffer;
      buffer << file.rdbuf();
      return buffer.str();
  }

    bool RunModule()
    {
       //1.完成资源初始化-------初始化 数据管理模块，创建指定的目录
        tb_video=new TableVideo();
      //2.添加请求与处理函数映射关系
      _svr.set_mount_point("/",WWWROOT);
       _svr.Post("/insert",Insert);
       _svr.Post("/update",Update);
       _svr.Post("/select",SelectAll);
       
       _svr.Get("/getusers", [](const httplib:: Request &req, httplib::Response &rsp) {
       Json::Value users;
       std::cout<<"获取用户列表"<<std::endl;
       bool ret =userfind::ojadmin().SelectLike(users);
       JsonUtil::Serialize(users,rsp.body);
       rsp.set_header("Content-Type","application/json");});
       _svr.Get(R"(/www/image/.*)", [&](const httplib::Request& req, httplib::Response& res) {
   
          std::string imagePath ="/root/video"+ req.path;
        //  std::cout << "Image Path: " << imagePath << std::endl;
          std::string imageData = readFile(imagePath);
          if (!imageData.empty()) {
              res.set_content(imageData, "image/jpeg");
          } else {
              res.status = 404;
              res.set_content("Image not found", "text/plain");
          }
      });

    _svr.Get("/questions", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            auto usernameIt = req.params.find("username");
            if (usernameIt == req.params.end()) {
                res.status = 400;
                res.set_content("Missing 'username' parameter", "text/plain");
                return;
            }
            std::string username = usernameIt->second;
            Json::Value allhistory;
            bool ret =historydata::TableVideo().SelectAll(username,allhistory);
            JsonUtil::Serialize(allhistory,res.body);
            res.set_header("Content-Type","application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content("Internal Server Error", "text/plain");
        }
    });
   
    
  _svr.Get("/submissions", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            auto usernameIt = req.params.find("username");
            auto userques=req.params.find("questionId");
            if (usernameIt == req.params.end()||userques==req.params.end()) {
                res.status = 400;
                res.set_content("没有对应历史记录", "text/plain");
                return;
            }
            std::string username = usernameIt->second;
            std::string questionid=userques->second;
           
            //获取某人对于某道题的所有记录
            Json::Value allhistory;
            bool ret =historydata::TableVideo().SelectAllhistory(username,questionid,allhistory);
            JsonUtil::Serialize(allhistory,res.body);
            res.set_header("Content-Type","application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            res.set_content("获取历史记录出现错误", "text/plain");
        }
    });
   


   _svr.Post("/delete-users", [&](const httplib::Request& req, httplib::Response& res)
        {
         Json::Value data;
             //反序列化
       if(JsonUtil::UnSerialize(req.body,data)==false)
       {   
          res.status=400;
          res.body=R"({"result":false,"reason": "新的信息格式解析失败"})";
          res.set_header("Content-Type","application/json");
          return;
       };  
        vector<string> usernames;
        for (const auto& username : data["usernames"]) {
            if (username.isString()) {
                usernames.push_back(username.asCString()); 
            }
        }

       for (const auto& name : usernames) {
            std::cout << "用户名: " << name << std::endl; // 输出：user1, user2
           userfind::ojadmin().deleteone(name);
       }
     });  
   

     _svr.Post("/create-user",[&](const httplib::Request& req, httplib::Response& rsp)
        {

       Json::Value video;
          //反序列化
         if(JsonUtil::UnSerialize(req.body,video)==false)
         {   
            rsp.status=400;
            rsp.body=R"({"result":false,"reason": "新的信息格式解析失败"})";
            rsp.set_header("Content-Type","application/json");
            return;
         };

        if(tb_video->create_user(video)==false)
        {
            rsp.status=500;
            rsp.body=R"({"result":false,"reason": "新增信息失败"})";
            rsp.set_header("Content-Type","application/json");
            return;
        }
          
     });  

    
     _svr.Get("/questionslist",[&](const httplib::Request& req, httplib::Response& rsp)
        {

       Json::Value list;
        if(tb_video->questionlist(list)==false)
        {
            rsp.status=500;
            rsp.body=R"({"result":false,"reason": "新增信息失败"})";
            rsp.set_header("Content-Type","application/json");
            return;
        }
         JsonUtil::Serialize(list,rsp.body);
         rsp.set_header("Content-Type","application/json");
          
     });  
         //3.启动服务器
       _svr.listen("0.0.0.0",_port);
       return true;
    }

   };
}