#include"./data.hpp"
#include"/root/OnlineJudge/comm/httplib.h"
#include<fstream>

namespace historyserver
{  

    #define WWWROOT "./www"
    #define VIDEO_ROOT "/video/"
    #define IMAGE_ROOT "/image/"
    using namespace historydata;
    using namespace historyutil;
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


    static void SelectAll( const httplib::Request &req,httplib::Response &rsp)
    {
     
       
       Json::Value inform;
       std::string city;
       Json::Value weather;
       if(JsonUtil::UnSerialize(req.body,weather)==false)
       {   
          rsp.status=400;
          rsp.body=R"({"result":false,"reason": "新的信息格式解析失败"})";
          rsp.set_header("Content-Type","application/json");
          return;
       };
       city=weather["city"].asCString();
     if(tb_video->SelectAll(city,inform)==false)
    {
     rsp.status=500;
     rsp.body=R"({"result":false,"reason":"查找指定id失败"})";
     rsp.set_header("Content-Type","application/json");
     return;
    }
    
    //组织响应正文
    JsonUtil::Serialize(inform,rsp.body);
    rsp.set_header("Content-Type","application/json");
    return ;
    } 


    static void SelectOne( const httplib::Request &req,httplib::Response &rsp)
    {
     
      Json::Value inform;
      std::string city;
      std::string day;
      Json::Value weather;
      if(JsonUtil::UnSerialize(req.body,weather)==false)
      {   
         rsp.status=400;
         rsp.body=R"({"result":false,"reason": "新的信息格式解析失败"})";
         rsp.set_header("Content-Type","application/json");
         return;
      };
      city=weather["city"].asCString();
      day=weather["day"].asCString();
    if(tb_video->SelectOne(city,day,inform)==false)
   {
    rsp.status=500;
    rsp.body=R"({"result":false,"reason":"查找指定id失败"})";
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
    
    bool writestring(std::string &content)
    {
      std::ofstream outFile("/root/OnlineJudge/oj_server/what.txt");
      if (outFile.is_open()) {
          outFile << content;
          outFile.close();
      } else {
          //写入失败
          return false;
      }
      return true;
    }

    bool readstring(std::string& content) {
      std::ifstream inFile("/root/OnlineJudge/oj_server/answer.txt");
      if (inFile.is_open()) {
          std::string line;
          while (std::getline(inFile, line)) {
              // 将每行内容添加到 content 中，并添加换行符
              content += line + '\n';
          }
          inFile.close();
          return true;
      } else {
          return false;
      }
  }

  bool clearFile(const std::string& filePath) {
    std::ofstream outFile(filePath, std::ios::trunc);
    if (outFile.is_open()) {
        outFile.close();
        return true;
    }
    return false;
}

  
    bool RunModule()
    {
       //1.完成资源初始化-------初始化 数据管理模块，创建指定的目录
        tb_video=new TableVideo();
      //2.添加请求与处理函数映射关系
      _svr.set_mount_point("/",WWWROOT);
       _svr.Post("/insert",Insert);
       _svr.Get("/selectall", SelectAll); // ?: 表示非捕获组，? 表示可选
       _svr.Get("/selectone",SelectOne);

       _svr.Post("/getanswer",[&](const httplib::Request &req,httplib::Response &rsp){ 
         Json::Value video;
          //反序列化
         if(JsonUtil::UnSerialize(req.body,video)==false)
         {   
            rsp.status=400;
            rsp.body=R"({"result":false,"reason": "新的信息格式解析失败"})";
            rsp.set_header("Content-Type","application/json");
            return;
         };
         
        //std::cout<<"测试用自定义字符串进行ai问答"<<std::endl;
       std::string content =video["content"].asCString() ;
        std::cout<<"测试用前端传来的字符串进行ai提问"<<std::endl;
        writestring(content);
        content="";
        Json::Value inform;
        std::string command = "python3 app.py"; 
        // 执行命令
        int result = system(command.c_str());
        if(result==0)
        {
          std::cout<<"调用成功";
        }
        else
        {
          std::cout<<"调用失败";
        }
        readstring(content);
        inform["content"] = content;
        JsonUtil::Serialize(inform, rsp.body);
        rsp.set_header("Content-Type", "application/json");
        rsp.status = 200; // 建议返回成功状态码
        clearFile("/root/OnlineJudge/oj_server/answer.txt");
        clearFile("/root/OnlineJudge/oj_server/what.txt");
       });
          //3.启动服务器
       _svr.listen("0.0.0.0",_port);
       return true;
    }

   };
}
