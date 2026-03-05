//TODO

#ifndef COMPILER_ONLINE
#include"header.cpp"
#endif 

void TEST1()
{   
    //定义临时对象来完成对应的方法的调用
    bool ret=Solution().isPalindrome(121);
    if(ret){
          std::cout<<"通过用例1"<<std::endl;
    }
    else{
        std::cout<<"没有通过用例1"<<std::endl;
    }
}

void TEST2()
{   
    //定义临时对象来完成对应的方法的调用
    bool ret=Solution().isPalindrome(-10);
    if(!ret){
          std::cout<<"通过用例2"<<std::endl;
    }
    else{
        std::cout<<"没有通过用例2"<<std::endl;
    }
}

int main()
{
    TEST1();
    TEST2();

    return 0;
}