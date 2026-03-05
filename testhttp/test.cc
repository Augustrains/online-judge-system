#include<iostream>
#include<json/json.h>
#include<vector>
#include"/root/OnlineJudge/comm/httplib.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <mutex>
#include <cstdlib>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;




// 函数用于测量向 8.140.17.222:8081 发送 POST 请求的网络延迟
double measureNetworkLatency() {
    // 创建一个 httplib 的客户端实例，目标服务器为 8.140.17.222:8081
    httplib::Client cli("8.140.17.222", 8081);
    cli.set_connection_timeout(1);  // 连接建立超时
    cli.set_read_timeout(1);        // 读取响应超时
    cli.set_write_timeout(1);       // 发送请求超时
    // 记录发送请求前的时间戳
    auto startTime = std::chrono::high_resolution_clock::now();

    // 发送 POST 请求，这里请求路径设为根路径，请求体为空
    auto res = cli.Get("/get_pid");

    // 记录收到响应后的时间戳
    auto endTime = std::chrono::high_resolution_clock::now();

    // 计算时间差，得到延迟时间
    std::chrono::duration<double, std::milli> latency = endTime - startTime;

    // 若请求成功，返回延迟时间；若失败，返回 -1 表示出错
    if (res) {
        return latency.count();
    } else {
        //代表超时了，此时返回3000ms
        return 3000;
    }
}

double get_process_cpu_usage(pid_t pid) {
    auto read_proc_stat = [](pid_t pid) -> unsigned long {
        std::ifstream stat_file("/proc/" + std::to_string(pid) + "/stat");
        if (!stat_file) {
            throw std::runtime_error("Failed to open /proc/" + std::to_string(pid) + "/stat");
        }

        std::string line;
        std::getline(stat_file, line);
        std::istringstream iss(line);

        // 跳过前 13 个字段
        std::string unused;
        for (int i = 0; i < 13; ++i) {
            if (!(iss >> unused)) {
                throw std::runtime_error("Failed to parse /proc/[PID]/stat");
            }
        }

        unsigned long utime, stime;
        if (!(iss >> utime >> stime)) {
            throw std::runtime_error("Failed to read utime/stime");
        }

        return utime + stime;
    };

    // 第一次读取
    unsigned long total_time1 = read_proc_stat(pid);

    // 等待 1 秒
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 第二次读取
    unsigned long total_time2 = read_proc_stat(pid);

    // CPU 使用率 = (总时间差 / 1秒) * 100%
    return (total_time2 - total_time1) * 100.0 / sysconf(_SC_CLK_TCK);
}

//获取系统总内存（单位：KB）
long get_total_memory() {
    std::ifstream meminfo("/proc/meminfo");
    if (!meminfo) {
        throw std::runtime_error("Failed to open /proc/meminfo");
    }
    std::string line;
    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal:") != std::string::npos) {
            std::istringstream iss(line);
            std::string label;
            long total_memory;
            iss >> label >> total_memory;
            return total_memory;
        }
    }
    throw std::runtime_error("Failed to find MemTotal in /proc/meminfo");
}

// 获取进程使用的内存（单位：KB）
long get_process_memory_usage(pid_t pid) {
    std::ifstream statm("/proc/" + std::to_string(pid) + "/statm");
    if (!statm) {
        throw std::runtime_error("Failed to open /proc/" + std::to_string(pid) + "/statm");
    }
    long resident_set_size;
    statm >> resident_set_size;
    return resident_set_size * sysconf(_SC_PAGESIZE) / 1024;
}

// 获取进程的内存利用率
double get_process_memory_utilization(pid_t pid) {
    long total_memory = get_total_memory();
    long process_memory = get_process_memory_usage(pid);
    return (double)process_memory / total_memory * 100;
}


void test_HTTP()
{
	httplib::Server svr;
    httplib::Client cli("8.140.17.222", 8081);
    auto res = cli.Get("/get_pid");
    pid_t pid_server=static_cast<pid_t>(std::stoi(res->body));

    svr.Get("/monitor", [&](const httplib::Request& req, httplib::Response& res) {
         
       double  cpuUsage=get_process_cpu_usage(pid_server);
       double memUsage= get_process_memory_utilization(pid_server);
        std::ostringstream oss;
        oss << "CPU Usage: " << cpuUsage << "%" << std::endl;
        oss << "MEM Usage: " << memUsage << "%" << std::endl;
        res.set_content(oss.str(), "text/plain");
    });

    svr.Get("/network",[&](const httplib::Request& req, httplib::Response& res) {
         double networkLatency = measureNetworkLatency();
         std::ostringstream oss;
         oss << "Network Latency: " << networkLatency << " ms" << std::endl;
         res.set_content(oss.str(), "text/plain");
     });


    svr.listen("0.0.0.0", 8080);
}

int main()
{
	//cout<<"hello"<<endl;

	//序列化工作
	//json将结构化数据转换成一个字符串
	//Value 是json的中间类，可以填充kv值
// 	Json::Value root;
// 	root["code"]="mycode";
// 	root["user"]="ygb";
// 	root["age"]="19";
//    // vector<int>arr_={1,2,3,4,5};
// 	root["arr"].append(1);
// 	root["arr"].append(2);
// 	root["arr"].append(3);
// 	root["arr"].append(4);
	
// 	cout<<"-------------------"<<endl;
// 	//Json::StyledWriter writer;
// 	Json::FastWriter writer;
// 	std::string str= writer.write(root);
//     std::cout<<str<<std::endl;
// 	cout<<"-------------------"<<endl;
    
// 	Json::Value newroot;
// 	Json::Reader reader;
// 	reader.parse(str,newroot);
// 	cout<<newroot<<endl;


//测试httplib库创建的服务器得到其服务器的实时 CPU 负载、内存剩余量、当前任务队列长度和网络延迟信息
test_HTTP();


	return 0;
}
