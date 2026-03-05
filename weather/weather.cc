
#include"./server.hpp"


using namespace historyutil;
using namespace historydata;
using namespace historyserver;


/* Json{
    "id":      ,
    "high":    ,
    "city":    ,
    "low":     ,
    "day":  
 
}*/
void data_test()
{
     TableVideo tb_video;
    // Json::Value video;
    // video["high"]="4°";
    // video["city"]="吉林";
    // video["low"]="-4°";
    // video["day"]="5-21";
    //  tb_video.Insert(video);

      Json::Value like_inform;
      tb_video.SelectAll("吉林",like_inform);
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