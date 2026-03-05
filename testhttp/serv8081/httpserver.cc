#include "/root/OnlineJudge/comm/httplib.h"
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <stdexcept>


int main() {
    httplib::Server svr;
    pid_t server_pid = getpid();
    std::cout<<server_pid<<std::endl;
    svr.Get("/get_pid", [server_pid](const httplib::Request&, httplib::Response& res) {
        res.set_content(std::to_string(server_pid), "text/plain");
    });
   
    svr.Get("/beat", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("OK","text/plain");
    });


    svr.listen("0.0.0.0", 8081);
    return 0;
}