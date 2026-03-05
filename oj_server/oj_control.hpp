#pragma once
#include <iostream>
#include <string>
#include <mutex>
#include "../comm/util.hpp"
#include "../comm/log.hpp"
#include "../comm/httplib.h"
#include "oj_model2.hpp" //改成2切换成MySQL版本
// #include"oj_model.hpp"
#include <vector>
#include "oj_view.hpp"
#include <cassert>
#include <fstream>
#include <json/json.h>
#include <fstream>
#include <algorithm>
#include"/root/OnlineJudge/history/data.hpp"
#include <cstdlib>
#include <ctime>


namespace ns_control
{
    using namespace std;
    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_model;
    using namespace ns_view;
    using namespace httplib;

    // 提供服务的主机
    class Machine
    {
    public:
        std::string ip;  // 编译服务的ip
        int port;        // 编译服务的接口
        uint64_t load;   // 编译服务的负载
        std::mutex *mtx; // mutex禁止拷贝的，所以使用指针
    public:
        Machine() : ip(""), port(0), load(0), mtx(nullptr)
        {
        }

        ~Machine() {}

    public:
        // 提升主机负载
        void Incconf()
        {
            if (mtx)
                mtx->lock();
            ++load;
            if (mtx)
                mtx->unlock();
        }
        // 负载清零
        void ResetLoad()
        {
            if (mtx)
                mtx->lock();
            load = 0;
            if (mtx)
                mtx->unlock();
        }
        // 减少主机负载
        void Desconf()
        {
            if (mtx)
                mtx->lock();
            --load;
            if (mtx)
                mtx->unlock();
        }

        uint64_t Load()
        {
            uint64_t _load = 0;
            if (mtx)
                mtx->lock();
            _load = load;
            if (mtx)
                mtx->unlock();
            return _load;
        }
    };

    const std::string service_machine = "./conf/service_machine.conf";

    // 负载均衡的模块
    class LoadBalance
    {
    private:
        // 可以提供编译服务的所有主机
        // 每一台主机都有自己的下标，利用下标充当当前主机的id
        std::vector<Machine> machines;
        // 所有在线的主机id
        std::vector<int> online;
        // 所有离线的主机id
        std::vector<int> offline;
        // 保证LoadBlance它的数据安全
        std::mutex mtx;
        //获取每个主机的端口号
        std::vector<pid_t>ppid; 
    

    public:
    std::vector<int>TaskNum;//任务数目
        LoadBalance()
        {
            bool flag = LoadConf(service_machine);
            assert(flag);
            // LOG(INFO)<<"可用主机数目："<<online.size()<<endl;
            LOG(INFO) << "加载 " << service_machine << " 成功" << "\n";

            for(int i=0;i<machines.size();i++)
            {
                httplib::Server svr;
                httplib::Client cli(machines[i].ip, machines[i].port);
                auto res = cli.Get("/get_pid");
                pid_t pid_server;
                if(res==nullptr)
                {
                    LOG(ERROR)<<"获取主机进程号失败!"<<"\n";
                    pid_server=-1;
                }
                else
                {  
                    pid_server=static_cast<pid_t>(std::stoi(res->body));
                    LOG(INFO)<<"获取进程号成功,进程号是:"<<pid_server<<"\n";
                }
                ppid.push_back(pid_server);
                TaskNum.push_back(0);
            }
            
        }
        ~LoadBalance() {}

    public: 
        
        bool LoadConf(const std::string &machine_conf)
        {
            std::ifstream in(machine_conf);
            if (!in.is_open())
            {
                LOG(FATAL) << " 加载:" << machine_conf << " 失败" << "\n";
                return false;
            }

            std::string line;
            while (getline(in, line))
            {
                std::vector<std::string> tokens;
                StringUtil::SplitString(line, &tokens, ":");
                if (tokens.size() != 2)
                {

                    LOG(WARNING) << "切分 " << line << " 失败" << "\n";
                    continue;
                }

                Machine m;
                m.ip = tokens[0];
                m.port = atoi(tokens[1].c_str());
                m.load = 0;
                m.mtx = new std::mutex();
                // LOG(INFO)<<"主机ip:"<<tokens[0]<<"\n";
                online.push_back(machines.size()); // online里面存的是machines的下标
                machines.push_back(m);
            }
            // LOG(INFO)<<"测试是否通过该函数"<<"\n";
            in.close();
            return true;
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
        
        double WeightedDistribution(int id)
        {
            //获取cpu利用率、内存利用率和当前任务队列的任务数目(就是所谓的当前load)
            double C=get_process_cpu_usage(ppid[id]);
            double M=get_process_memory_utilization(ppid[id]);
            double Q=TaskNum[id];
           // LOG(INFO)<<"主机"<<id<<"的数据信息:"<<"CPU---"<<C<<"|内存---"<<M<<"|任务数目---"<<Q<<"\n";
            return C*0.5+M*0.3+Q*0.2; //权重值
        }

        // id 输出型参数
        // m 输出型餐宿

        //轮询方式单独查看任务队列任务数目
        bool SmartChoice(int *id, Machine **m)
        {
            // 1.使用选择好的主机(更新主机的负载)
            // 2.我们需要可能离线该主机  online---->offline
            mtx.lock();
            // 负载均衡的算法
            // 1.随机数法+hash
            // 2.轮询+hash

            int online_num = online.size();

            if (online_num == 0)
            {
                // 此时没有在线的主机
                LOG(FATAL) << "所有的后端主机全部离线,请尽快修复" << "\n";
                mtx.unlock();
                return false;
            }

            // 通过遍历找负载最小的机器
            *id = online[0];
            *m = &machines[online[0]];
            uint64_t min_load = machines[online[0]].Load();
        

            for (int i = 1; i < online_num; i++)
            {  
                //选择负载最小的主机，此处选择多种办法
                uint64_t temp_load = machines[online[i]].Load();
           
               
                if (min_load > temp_load)
                {
                    min_load = temp_load;
                    *id = online[i];
                    *m = &machines[online[i]];
                }
            }

            mtx.unlock();
            return true;
        }

        //加权方式，查看cpu、内存和任务数目

        //选择主机和id
        bool SmartWeightChoice(int *id, Machine **m)
        {
            // 1.使用选择好的主机(更新主机的负载)
            // 2.我们需要可能离线该主机  online---->offline
            mtx.lock();
            int online_num = online.size();
            if (online_num == 0)
            {
                // 此时没有在线的主机
                LOG(FATAL) << "所有的后端主机全部离线,请尽快修复" << "\n";
                mtx.unlock();
                return false;
            }

            // 通过遍历找负载最小的机器
            *id = online[0];
            *m = &machines[online[0]];
            double min_load = WeightedDistribution(online[0]);
        

            for (int i = 1; i < online_num; i++)
            {  
                //选择负载最小的主机，此处选择多种办法
                double temp_load = WeightedDistribution(online[i]);
                if (min_load > temp_load)
                {
                    min_load = temp_load;
                    *id = online[i];
                    *m = &machines[online[i]];
                }
            }
            TaskNum[*id]++;
            mtx.unlock();
            return true;
        }
          
        //采用随机数的方式选择主机
        bool SmartRandomChoice(int *id, Machine **m)
        {
            // 1.使用选择好的主机(更新主机的负载)
            // 2.我们需要可能离线该主机  online---->offline
            mtx.lock();
            // 负载均衡的算法
            // 1.随机数法+hash
            // 2.轮询+hash

            int online_num = online.size();

            if (online_num == 0)
            {
                // 此时没有在线的主机
                LOG(FATAL) << "所有的后端主机全部离线,请尽快修复" << "\n";
                mtx.unlock();
                return false;
            } 

            srand(time(0));
            //随机选择
            int randomNum = rand() % online.size();
            std::cout<<online.size()<<std::endl;
            *id = online[randomNum];
            *m = &machines[online[randomNum]];
            mtx.unlock();
            return true;
        }
        
           //采用随机数的方式选择主机
           bool oneChoice(int *id, Machine **m)
           {
               // 1.使用选择好的主机(更新主机的负载)
               // 2.我们需要可能离线该主机  online---->offline
               mtx.lock();
               // 负载均衡的算法
               // 1.随机数法+hash
               // 2.轮询+hash
   
               int online_num = online.size();
   
               if (online_num == 0)
               {
                   // 此时没有在线的主机
                   LOG(FATAL) << "所有的后端主机全部离线,请尽快修复" << "\n";
                   mtx.unlock();
                   return false;
               } 
               *id = online[0];
               *m = &machines[online[0]];
               mtx.unlock();
               return true;
           }
   
        void OfflineMachine(int id)
        {
            mtx.lock();
            for (auto iter = online.begin(); iter != online.end(); iter++)
            {
                if (*iter == id)
                {
                    // 找到要离线的主机
                    online.erase(iter);

                    // 将负载清空
                    machines[id].ResetLoad();

                    offline.push_back(id);

                    break;
                }
            }
            mtx.unlock();
        }

        void OnlineMachine()
        {
            // 上线
            mtx.lock();
            // offline--->online
            online.insert(online.end(), offline.begin(), offline.end());
            offline.erase(offline.begin(), offline.end());
            LOG(INFO) << " 所有主机已重新上线!" << "\n";
            mtx.unlock();
        }

        // 查看当前主机
        void ShowMachine()
        {
            mtx.lock();
            LOG(INFO) << "当前在线的主机列表：";
            for (auto &id : online)
            {
                std::cout << id << " ";
            }
            std::cout << std::endl;

            LOG(INFO) << "当前离线的主机列表：";
            for (auto &id : offline)
            {
                std::cout << id << " ";
            }
            std::cout << std::endl;

            mtx.unlock();
        }
    };

    // 核心的控制器
    class Control
    {
    private:
        Model model_;              // 提供后台数据
        View view_;                // 提供网页渲染
        LoadBalance load_balance_; // 负载均衡器

    public:
        Control() {};
        ~Control() {};

    public:
        // 根据题目数据，构建网页
        // html是输出型参数
         
        //增加题目
        bool AddQuestion(Question &q)
        {
            return model_.AddQuestion(q);
        }
         
        //删除题目
        bool DeleteQuestion(std::string number)
        {
            return model_.DeleteQuestion(number);
        }

        void RecoverMachine()
        {
            load_balance_.OnlineMachine();
        }

        bool GetHtml(std::string *html, std::string path)
        {
            // if(!view_.GetTalk(html))
            // {
            //    LOG(ERROR)<<"获取论坛失败"<<"\n";
            //    return false;
            // }
            // return true;

            std::string _path = "./template.html/" + path;
            fstream in(_path);
            if (!in.is_open())
            {
                LOG(ERROR) << "文件打开失败" << "\n";
                return false;
            }
            std::string tmp;
            while (getline(in, tmp))
            {
                *html += tmp;
            }
            in.close();
            return true;
        }

        bool AllQuestions(std::string *html)
        {
            vector<struct Question> all;
            bool ret = true;
            if (model_.GetAllQuestions(&all))
            {
                // 获取题目信息成功,将所有的题目数据构建成网页返回
                //  cout<<"****************"<<endl;
                //  for(auto e:all)
                //  cout<<e.number<<" "<<e.title<<endl;
                //  cout<<"*****************"<<endl;
                sort(all.begin(), all.end(), [](const struct Question &p1, const struct Question &p2)
                     {
                    /* data */
                    return atoi(p1.number.c_str())<atoi(p2.number.c_str()); });
                view_.AllExpandHtml(all, html);
                // cout<<html<<endl;
            }
            else
            {
                *html = "获取题目失败，形成题目列表失败";
                ret = false;
            }
            return ret;
        }

        bool Question(const std::string &number, std::string *html)
        {
            struct Question q;
            bool ret = true;
            if (model_.GetOneQuestion(number, &q))
            {
                // 获取指定题目成功
                view_.OneExpandHtml(q, html);
            }
            else
            {
                *html = "指定题目:" + number + "不存在";
                ret = false;
            }
            return ret;
        }

        // id 100
        // code #include....
        // input:""
        void Judge(const std::string &number, const std::string in_json, std::string *out_json)
        {    
          
            //获取用于jmeter测试的信息
           // std::cout<<in_json<<std::endl;
            Json::Value history; //获取历史记录，用于增加一条历史记录
            // LOG(DEBUG)<<in_json<<"\nnumber:"<<number<<"\n";

            // 根据number拿到对应题目细节
            struct Question q;
            model_.GetOneQuestion(number, &q);

            // 1. in_json进⾏反序列化，得到题⽬的id，得到⽤⼾提交源代码，input
            Json::Reader reader;
            Json::Value in_value;
            reader.parse(in_json, in_value);
            std::string code = in_value["code"].asString();
            // 2. 重新拼接⽤⼾代码+测试⽤例代码，形成新的代码
            history["time"]=in_value["time"];
            history["name"]=in_value["name"];
           // cout<<"当前提交的时间是:"<<in_value["time"]<<std::endl;
            history["code"]=code;
            history["id"]=std::stoi(number);
            Json::Value compile_value;
            compile_value["input"] = in_value["input"].asString();
            compile_value["code"] = code + q.tail;
            compile_value["cpu_limit"] = q.cpu_limit;
            compile_value["mem_limit"] = q.mem_limit;
            Json::FastWriter writer;
            std::string compile_string = writer.write(compile_value);
            // 3. 选择负载最低的主机(差错处理)
            // 规则: ⼀直选择，直到主机可⽤，否则，就是全部挂掉
            while (true)
            {
                int id = 0;
                Machine *m = nullptr;

                //更换选择的函数

                //轮询

                // if (!load_balance_.SmartChoice(&id, &m))
                // {
                //     break;
                // }
                
                //加权轮询
                if (!load_balance_.SmartWeightChoice(&id, &m))
                {
                    break;
                }
                 
                //随机
                // if (!load_balance_.SmartRandomChoice(&id, &m))
                // {
                //     break;
                // }
                
                // 只用一个
                // if(!load_balance_.oneChoice(&id,&m))
                // {
                //     break;
                // }
                // LOG(INFO) << " 选择主机成功, 主机id: " << id << "详情: " << m->ip
                //           << ":" << m->port << "\n";
                // 4. 然后发起http请求，得到结果
                Client cli(m->ip, m->port);
                cli.set_connection_timeout(3000);  // 连接超时
                cli.set_read_timeout(3000);        // 读取超时
                m->Incconf();
                auto res = cli.Post("/compile_and_run", compile_string, "application/json;charset=utf-8");
               // std::cout<<"编译允许器返回的结果"<<res<<std::endl;
               // int status = res->status;  // 测试status
                // std::cout<<status<<std::endl;
                if (res)
                {   
                    load_balance_.TaskNum[id]--;

                    // 5. 将结果赋值给out_json
                    if (res->status == 200)
                    {   
                        
                        //std::cout<<"编译返回的结果"<<res->body<<std::endl;  

                        std::string result;
                        Json::Reader red;
                        Json::Value getres;
                        red.parse( res->body,getres);
                         result= getres["stdout"].asString();
                        // std::cout<<"输出的结果:"<<result<<std::endl;
                        if(result=="通过用例1\n通过用例2\n")
                        {
                            history["result"]="success";
                        }
                        else
                        {
                            history["result"]="error";
                        }

                        //将history存放到数据库中
                        //std::cout<<history<<std::endl;
                        //测试编译功能时后先注释掉
                         

                        *out_json = res->body;
                        m->Desconf();
                        LOG(INFO) << "主机:"<<id<<"请求服务成功" << "\n";
                         historydata::TableVideo().Insert(history); 
                        //LOG(INFO)<<*out_json<<"\n";
                        break;
                    }
                    else
                    {   
                        LOG(INFO) << "主机:"<<id<<"请求服务失败" << "\n";
                        m->Desconf();
                        break;
                    }
                  
                }
                else
                {
                    // 请求失败
                  //  std::cout<<res->error()<<std::endl;
                //   auto err = res.error();
                //   std::cerr << "Request failed: " << httplib::to_string(err) << std::endl;
                //     LOG(ERROR) << " 当前请求的主机id: " << id << "详情: " << m ->ip << ":" << m->port << " 可能已经离线" << "\n";
                //     load_balance_.OfflineMachine(id);
                //     load_balance_.ShowMachine(); // 仅仅是为了⽤来调试
                    //循环等待
                    continue;
                }
                
            }
        }
    };
} // namespace name
