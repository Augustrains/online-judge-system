#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<algorithm>

using namespace std;

class Solution{
  public:
        bool isPalindrome(int x)
        {
             //将你的代码写在下面
             
             return true;
        }
};
//TODO

#ifndef COMPILER_ONLINE
#include"header.cpp"
#endif 

void TEST1()
{   
    //定义临时对象来完成对应的方法的调用
    bool ret=Solution().isPalindrome(121);
    if(ret){
          std::cout<<"通过用例1,测试121通过...ok!"<<std::endl;
    }
    else{
        std::cout<<"没有通过用例1,测试的值是:121"<<std::endl;
    }
}

void TEST2()
{   
    //定义临时对象来完成对应的方法的调用
    bool ret=Solution().isPalindrome(-10);
    if(!ret){
          std::cout<<"通过用例1,测试-10通过...ok!"<<std::endl;
    }
    else{
        std::cout<<"没有通过用例1,测试的值是:-10"<<std::endl;
    }
}

int main()
{
    TEST1();
    TEST2();

    return 0;
}
