//TODO

#ifndef COMPILER_ONLINE
#include"header.cpp"
#endif 

void TEST1()
{   
   std::vector<int> v={2,7,11,15};
    vector<int> ret=Solution().twoSum(v,9);
   if(ret[0]=0&&ret[1]==1)
   {
    std::cout<<"通过用例1"<<std::endl;
   }
   else{
    std::cout<<"没有通过用例1"<<std::endl;
   }
}

void TEST2()
{   
    std::vector<int> v={3,2,4};
    vector<int> ret=Solution().twoSum(v,6);
   if(ret[0]=1&&ret[1]==2)
   {
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