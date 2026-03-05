#pragma once

#include<iostream>
#include<string>
#include<ctemplate/template.h>

#include"oj_model2.hpp" //文件版本
//#include"oj_model.hpp"

namespace ns_view
{
    using namespace ns_model;
   /*
   struct Question
   {
        std::string number;//题目编号
        std::string title;//标题
        std::string star;//难度:简单，中等，困难
        int cpu_limiit;//题目的时间要求(s)
        int mem_limit;//题目的空间要求(kb)
        std::string desc;//题目的描述
        std::string header;//题目预设给用户在线编辑器的代码
        std::string tail;//题目的测试用例，需要和header拼接，形成完整代码，再提交给后端
    };
   */

   const std::string template_path="./template.html/"; 

    class View
    {
    public:
        View(){};
        ~View(){};

    public:

        bool GetTalk(std::string *html)
        {
            std::string src_html=template_path+"talk.html";
            ctemplate::TemplateDictionary root("talk");
            ctemplate::Template *tpl=ctemplate::Template::GetTemplate(src_html,ctemplate::DO_NOT_STRIP);
            tpl->Expand(html,&root);
        }

        void AllExpandHtml(vector<struct Question>&questions,std::string *html)
        {
           //题目编号 标题 题目难度 
           //利用表格显示 
           std::string src_html=template_path+"all_questions.html";
            //形成数据字典
           ctemplate::TemplateDictionary root("all_questions");
           
           //cout<<"question的个数是:"<<questions.size()<<endl;
           for(const auto&q:questions)
           {  
             //形成一个子字典
             ctemplate::TemplateDictionary *sub=root.AddSectionDictionary("question_list");
             sub->SetValue("number",q.number);
             sub->SetValue("title",q.title);
             sub->SetValue("star",q.star);
             //cout<<q.number<<" "<<q.title<<" "<<q.star<<endl;
           }

           //获取被渲染的网页
           ctemplate::Template *tpl=ctemplate::Template::GetTemplate(src_html,ctemplate::DO_NOT_STRIP);
           
           //开始完成渲染
           tpl->Expand(html, &root);
        //    cout<<"*******************"<<endl;
        //    cout<<"页面渲染"<<endl;
        //    std::cout<<*html<<std::endl;
        //    cout<<"*******************"<<endl;

        }

        void OneExpandHtml(struct Question &q,std::string *html)
        {

            std::string src_html=template_path+"one_question.html";
            ctemplate::TemplateDictionary root("one_question");
            root.SetValue("number",q.number); 
            root.SetValue("title",q.title); 
            root.SetValue("star",q.star); 
            root.SetValue("desc",q.desc); 
            root.SetValue("pre_code",q.header); 

            ctemplate::Template *tpl=ctemplate::Template::GetTemplate(src_html,ctemplate::DO_NOT_STRIP);
            tpl->Expand(html,&root);
        }

    };

} // namespace 
