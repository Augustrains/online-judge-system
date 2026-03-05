
#include"./server.hpp"


using namespace historyutil;
using namespace historydata;
using namespace historyserver;


/* Json{
    "id":      ,
    "code":    ,
    "time":    ,
    "result":  ,
 
}*/
void data_test()
{
    TableVideo tb_video;
    Json::Value video;
    video["idd"]=4;
    video["id"]=3;
    video["name"]="aa";
   // tb_video.Insert(video);
     Json::Value like_inform;
     tb_video.SelectAll(3,"aa",like_inform);
     std::string body;
     JsonUtil::Serialize(like_inform,body);
     std::cout<<body<<std::endl;

}
int main()
{   
     //data_test();
     Server server(9091);
	 server.RunModule();
}