#pragma once
//根据题目list文件，加载所有的题目信息到内存中
//model:主要用来和数据进行交互，对外提供访问数据的接口
//文件版本
#include<iostream>
#include<string>
#include<unordered_map>
#include<cassert>
#include"../comm/log.hpp"
#include<vector>
#include<fstream>
#include"../comm/util.hpp"
#include<cstdlib>
#include"/root/OnlineJudge/history/data.hpp"

namespace ns_model
{    
    using namespace std;
    using namespace ns_log;
    using namespace ns_util;
    /// @brief 
    struct Question{
        std::string number;//题目编号
        std::string title;//标题
        std::string star;//难度:简单，中等，困难
        int cpu_limit;//题目的时间要求(s)
        int mem_limit;//题目的空间要求(kb)
        std::string desc;//题目的描述
        std::string header;//题目预设给用户在线编辑器的代码
        std::string tail;//题目的测试用例，需要和header拼接，形成完整代码，再提交给后端
    };
    
    const string questions_list="./questions/questions.list";
    const std::string questions_path="./questions/";

    class  Model{
    private:
   //题号:题目细节
         unordered_map<string,Question>questions;
    
    public:
      Model()
      {
         assert(LoadQuestionList(questions_list));
      };

      bool LoadQuestionList(const std::string &question_list)
      {
         //加载配置文件：questions/questions.list+题目编号文件
         ifstream in(question_list);
         if(!in.is_open())
         {
            //大概率文件名错了或者路径不存在
            LOG(FATAL)<<" (load fail)加载题库失败,请检查是否存在题库文件"<<"\n";
            return false;
         }
        
        std::string line;
        while(getline(in,line))
        {
           //一行包括一个编号的所有信息
           vector<string> tokens;
           StringUtil::SplitString(line, &tokens, " ");
           // 1 判断回⽂数 简单 1 30000 
           if(tokens.size() != 5)
           {
           LOG(WARNING) << "加载部分题⽬失败, 请检查⽂件格式" << "\n";
           continue;
           }
          
           Question q;
           q.number=tokens[0];
           q.title=tokens[1];
           q.star=tokens[2];
           q.cpu_limit=atoi(tokens[3].c_str());
           q.mem_limit=atoi(tokens[4].c_str());
           
           std::string path=questions_path;
           path+=q.number;
           path+="/";

           FileUtil::ReadFile(path+"desc.txt",&(q.desc),true);
           FileUtil::ReadFile(path+"header.cpp",&(q.header),true);
           FileUtil::ReadFile(path+"tail.cpp",&(q.tail),true);

           questions.insert({q.number,q});

        }
        LOG(INFO)<<" 加载题库...成功!(success)"<<"\n";
        in.close();
        return  true;
      }
      

      bool GetAllQuestions(vector<Question>*out){
         if(questions.size()==0)
         {
          LOG(ERROR)<<"(user fail)用户获取题库失败"<<"\n";
          return false;
         }
         for(const auto &q:questions)
         {
             out->push_back(q.second);// first:key   second:value
         }
         return true;
      }
      
      bool GetOneQuestion(const std::string &number,Question*q){
         const auto&iter=questions.find(number);
         if(iter==questions.end())
         {
          LOG(ERROR)<<"(user fail)用户获取题库失败,题目编号:"<< number <<"\n";
          return false;
         }
         (*q)=iter->second;
         return true;
      }

       bool AddQuestion(const Question& q) {
    // 1. 检查题目编号是否已存在
    if (questions.find(q.number) != questions.end()) {
        LOG(WARNING) << "添加失败，题目编号 " << q.number << " 已存在\n";
        return false;
    }
    
    std::string q_path = questions_path + q.number + "/";
    if (mkdir(q_path.c_str(), 0775) != 0) {  // Linux目录创建
        LOG(ERROR) << "创建题目目录失败: " << q_path << "\n";
        return false;
    }

    // 3. 写入题目文件
    try {
        FileUtil::WriteFile(q_path + "desc.txt", q.desc);
        FileUtil::WriteFile(q_path + "header.cpp", q.header);
        FileUtil::WriteFile(q_path + "tail.cpp", q.tail);
        
        // 4. 更新questions.list（确保新内容另起一行）
        bool needsNewline = false;
        std::ifstream checkFile(questions_list);
        
        // 检查文件是否存在且不为空
        if (checkFile.good()) {
            // 定位到文件末尾前一个字符
            checkFile.seekg(-1, std::ios_base::end);
            char lastChar;
            checkFile.get(lastChar);
            
            // 如果最后一个字符不是换行符，则需要添加
            needsNewline = (lastChar != '\n');
        }
        checkFile.close();
        
        // 以追加模式打开文件
        std::ofstream list_file(questions_list, std::ios::app);
        
        // 如果需要，先添加一个换行符
        if (needsNewline) {
            list_file << "\n";
        }
        
        // 写入新题目信息
        list_file << q.number << " " << q.title << " " << q.star << " " 
                 << q.cpu_limit << " " << q.mem_limit << "\n";
        list_file.close();
        
        // 5. 更新内存数据
        questions[q.number] = q;
        LOG(INFO) << "成功添加题目: " << q.number << "\n";
        return true;
    } catch (...) {
        LOG(ERROR) << "写入题目文件时发生异常\n";
        return false;
    }
}
    bool DeleteQuestion( std::string& number) {
        // 1. 检查题目是否存在
        auto it = questions.find(number);
        if (it == questions.end()) {
            LOG(WARNING) << "删除失败，题目编号 " << number << " 不存在\n";
            return false;
        }

        // 2. 删除题目目录
        std::string q_path = questions_path + number + "/";
        std::string cmd = "rm -rf " + q_path;  // Linux删除命令
        if (system(cmd.c_str()) != 0) {
            LOG(ERROR) << "删除题目目录失败: " << q_path << "\n";
            return false;
        }
         
        //删除历史记录中的对应题目id
        historydata::TableVideo().deletehistory(atoi(number.c_str()));
         
        // 3. 更新questions.list
        vector<string> new_list;
        ifstream in(questions_list);
        string line;
        while (getline(in, line)) {
            vector<string> tokens;
            StringUtil::SplitString(line, &tokens, " ");
            if (!tokens.empty() && tokens[0] != number) {
                new_list.push_back(line);
            }
        }
        in.close();

        ofstream out(questions_list);
        for (const auto& l : new_list) {
            out << l << "\n";
        }
        out.close();

        // 4. 更新内存数据
        questions.erase(it);
        LOG(INFO) << "成功删除题目: " << number << "\n";
        return true;
    }


      ~Model()
      {};
    };

}
