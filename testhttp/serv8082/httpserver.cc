#include "/root/OnlineJudge/comm/httplib.h"
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <stdexcept>

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

int main() {
    httplib::Server svr;
    pid_t server_pid = getpid();
    std::cout<<server_pid<<std::endl;
    svr.Get("/cpuuse", [server_pid](const httplib::Request&, httplib::Response& res) {
        try {
            double cpu_usage = get_process_cpu_usage(server_pid);
            std::cout << "CPU Usage: " << cpu_usage << "%" << std::endl;
            double mem_usage=get_process_memory_utilization(server_pid);
            std::cout << "Mem Usage: " << mem_usage << "%" << std::endl;
            res.set_content(std::to_string(cpu_usage), "text/plain");
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            res.status = 500;
            res.set_content("Error: " + std::string(e.what()), "text/plain");
        }
    });

    svr.Post("/beat", [](const httplib::Request&, httplib::Response& res) {
        for (int i = 0; i < 10240; i++); // 模拟 CPU 计算
        res.set_content("OK", "text/plain");
    });

    svr.listen("0.0.0.0", 8082);
    return 0;
}