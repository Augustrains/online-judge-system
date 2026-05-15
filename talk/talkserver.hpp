#include"./talkdata.hpp"
#include "../comm/httplib.h"
#include "../oj_server/judgeadmin.hpp"

namespace server
{  

    #define WWWROOT "./www"
    using namespace data;
    using namespace util;
    TableVideo *tb_video=nullptr;
   class Server
   {
    private:
    int _port;
    httplib::Server _svr;
    
    private:
    static void Inserttype(const httplib::Request &req,httplib::Response &rsp)
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

        // 检查帖子标题是否已存在
        std::string type = video["type"].asString();
        if (tb_video->CheckTypeExists(type))
        {
            rsp.status=409;
            rsp.body=R"({"result":false,"reason": "帖子名称已存在，请使用不同的标题"})";
            rsp.set_header("Content-Type","application/json");
            return;
        }

        if(tb_video->Inserttype(video)==false)
        {
            rsp.status=500;
            rsp.body=R"({"result":false,"reason": "新增数据失败，请稍后重试"})";
            rsp.set_header("Content-Type","application/json");
            return;
        }

        rsp.status=200;
        rsp.body=R"({"result":true,"message": "帖子创建成功"})";
        rsp.set_header("Content-Type","application/json");
        return;
    }


        static void Insertone(const httplib::Request &req,httplib::Response &rsp)
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

        if(tb_video->Insertone(video)==false)
        {
            rsp.status=500;
            rsp.body=R"({"result":false,"reason": "新增数据失败"})";
            rsp.set_header("Content-Type","application/json");
            return;
        }

        rsp.status=200;
        rsp.body=R"({"result":true,"message": "评论发布成功"})";
        rsp.set_header("Content-Type","application/json");
        return;
    }


    static void Selectype( const httplib::Request &req,httplib::Response &rsp)
    {
     

       Json::Value inform;
     if(tb_video->SelectAll(inform)==false)
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


    static void Selectone( const httplib::Request &req,httplib::Response &rsp)
    {
       
      // 添加调试输出

       Json::Value inform;
       Json::Value video;
        //反序列化
       if(JsonUtil::UnSerialize(req.body,video)==false)
       {   
          rsp.status=400;
          rsp.body=R"({"result":false,"reason": "新的信息格式解析失败"})";
          rsp.set_header("Content-Type","application/json");
          return;
       };
      int f=tb_video->Selectone(inform,video);
     if(f==0)
    {
     rsp.status=500;
     rsp.body=R"({"result":false,"reason":"查找帖子失败"})";
     rsp.set_header("Content-Type","application/json");
     return;
    }
    //组织响应正文
    JsonUtil::Serialize(inform,rsp.body);
    rsp.set_header("Content-Type","application/json");
    return ;
    } 

    // 更新帖子点击量
static void UpdateView(const httplib::Request &req, httplib::Response &rsp)
{   
    std::cout<<"更新"<<std::endl;
    Json::Value data;
    if (JsonUtil::UnSerialize(req.body, data) == false) {
        rsp.status = 400;
        rsp.body = R"({"result":false,"reason": "请求格式错误"})";
        rsp.set_header("Content-Type", "application/json");
        return;
    }
    
    std::string type = data["type"].asString();
    std::cout<<type<<std::endl;
    if (type.empty()) {
        rsp.status = 400;
        rsp.body = R"({"result":false,"reason": "缺少type参数"})";
        rsp.set_header("Content-Type", "application/json");
        return;
    }
    
    // 更新点击量
    if (tb_video->UpdateViewCount(type) == false) {
        rsp.status = 500;
        rsp.body = R"({"result":false,"reason": "更新点击量失败"})";
        rsp.set_header("Content-Type", "application/json");
        return;
    }
    
    rsp.body = R"({"result":true,"message": "点击量更新成功"})";
    rsp.set_header("Content-Type", "application/json");
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
       _svr.Post("/inserttype",Inserttype);//创建一个帖子
        _svr.Post("/insertone",Insertone);//创建帖子内一个评论
       _svr.Get("/findtype",Selectype);//显示所有帖子
       _svr.Post("/findone",Selectone);//显示某个帖子的所有评论
       _svr.Post("/updateview", UpdateView);


       // 删除帖子（需要管理员权限）
    _svr.Post("/deletetalk", [](const httplib::Request &req, httplib::Response &res) {
        try {
            Json::Value data;
            JsonUtil::UnSerialize(req.body, data);
            std::string type = data["type"].asString();
            std::string name = data.get("name", "").asString();

            // 服务端校验管理员权限
            if (name.empty() || !judgeadmin::ojadmin::GetInstance().IsRoot(name)) {
                res.status = 403;
                res.body = R"({"result":false,"reason": "无权限，仅管理员可删除"})";
                res.set_header("Content-Type", "application/json");
                return;
            }

            if(tb_video->deleteTalk(type)==true)
            {
              res.status = 200;
              res.body = R"({"result":true,"message": "删除成功"})";
              res.set_header("Content-Type", "application/json");
              return ;
            }
            else
            {
                res.status = 400;
                res.body = R"({"result":false,"reason": "删除失败"})";
                res.set_header("Content-Type", "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.body = R"({"result":false,"reason": "请求格式错误"})";
            res.set_header("Content-Type", "application/json");
        }
    });
   

      _svr.Get("/checkadmin", [](const httplib::Request &req, httplib::Response &res) {
      if (!req.has_param("name")) {
            res.status = 400;
            res.set_content("Missing 'name' parameter", "text/plain");
            return;
        }
        
        Json::Value jud;
        std::string username = req.get_param_value("name");
        bool ret=judgeadmin::ojadmin::GetInstance().IsRoot(username);
        jud["isAdmin"]=ret;
       JsonUtil::Serialize(jud,res.body);
       res.status = 200;

      } );
 
       _svr.Post("/deletecomme", [](const httplib::Request &req, httplib::Response &res) {

        Json::Value data;
        if (JsonUtil::UnSerialize(req.body, data) == false)
            {
                res.status = 400;
                res.body = R"({"result":false,"reason": "请求格式错误"})";
                res.set_header("Content-Type", "application/json");
                return;
            }

            // 检查id字段是否存在且为字符串或整数类型
            if (!data.isMember("id") || (!data["id"].isString() && !data["id"].isInt())) {
                res.status = 400;
                res.body = R"({"result":false,"reason": "缺少id字段或类型错误"})";
                res.set_header("Content-Type", "application/json");
                return;
            }

            // 服务端校验管理员权限
            std::string name = data.get("name", "").asString();
            if (name.empty() || !judgeadmin::ojadmin::GetInstance().IsRoot(name)) {
                res.status = 403;
                res.body = R"({"result":false,"reason": "无权限，仅管理员可删除评论"})";
                res.set_header("Content-Type", "application/json");
                return;
            }

            // 兼容 int 和 string 两种 id 类型
            int id;
            if (data["id"].isInt()) {
                id = data["id"].asInt();
            } else {
                id = std::stoi(data["id"].asString());
            }

            bool ret=tb_video->delcomment(id);
           if(ret==false)
           {
                 res.status = 400;
                 res.body = R"({"result":false,"reason": "删除失败"})";
                 res.set_header("Content-Type", "application/json");
                 return;
           }

            res.status = 200;
            res.body = R"({"result":true,"message": "评论删除成功"})";
            res.set_header("Content-Type", "application/json");
      } );


          //3.启动服务器
       _svr.listen("0.0.0.0",_port);
       return true;
    }

   };
}
