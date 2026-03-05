
#include"./talkserver.hpp"


//using namespace util;
//using namespace data;
//using namespace server;

//包括插入，删除，查看所有，查看指定，修改

/* Json{
    "name":      ,
    "id":    ,
    "time":    ,
    "content":  ,
 
}*/
// void data_test()
// {
//     data::TableVideo tb_video;
//     Json::Value video;
//     video["name"]="name111";
//     video["time"]="2022.1.23";
//     video["content"]="success";
//     tb_video.Insert(video);
//     //  Json::Value like_inform;
//     //  tb_video.SelectOne(4,like_inform);
//     //  std::string body;
//     //  JsonUtil::Serialize(like_inform,body);
//     //  std::cout<<body<<std::endl;

// }
int main()
{   
      //data_test();
    server::Server server(9099);
	server.RunModule();
}