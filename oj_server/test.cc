
#pragma once
#include <iostream>
#include <string>
#include "oj_model2.hpp" //改成2切换成MySQL版本


#include <fstream>

using namespace ns_model;


Question init_question()
{   
    // std::string number;//题目编号
    //     std::string title;//标题
    //     std::string star;//难度:简单，中等，困难
    //     int cpu_limit;//题目的时间要求(s)
    //     int mem_limit;//题目的空间要求(kb)
    //     std::string desc;//题目的描述
    //     std::string header;//题目预设给用户在线编辑器的代码
    //     std::string tail;//题目的测试用例，需要和header拼接，形成完整代码，再提交给后端
    Question newq;
    newq.number="21";
    newq.title="测试添加功能";
    newq.star="简单";
    newq.cpu_limit=1;
    newq.mem_limit=2;
    newq.desc="测试题目描述";
    newq.header="test question header";
    return newq;
}

int main()
{
   Question q=init_question();
   Model().AddQuestion(q);
  //Model().DeleteQuestion("20");
}