
#include"./server.hpp"

// void data_test()
// {
//     TableVideo tb_video;
//     Json::Value video;
//     video["name"]="小天";
//     video["password"]="123456";

// //    tb_video.Insert(video);
    
//     //tb_video.Update(video);
    

//      Json::Value like_inform;
//      tb_video.SelectLike(video,like_inform);
//      std::string body;
//      JsonUtil::Serialize(like_inform,body);
//      std::cout<<body<<std::endl;
// }

int main()
{   
    //data_test();
    loginserver::Server server(9092);
	server.RunModule();
}