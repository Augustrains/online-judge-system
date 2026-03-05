//TODO

#ifndef COMPILER_ONLINE
#include"header.cpp"
#endif 

void TEST1()
{   
   std::vector<int> v={1,2,3};
   std::vector<int> ret=Solution().plusOne(v);
   if(ret[0]==1&&ret[1]==2&&ret[2]==4)
   {
    std::cout<<"通过用例1"<<std::endl;
   }
   else{
    std::cout<<"没有通过用例1"<<std::endl;
   }
}

void TEST2()
{   
    std::vector<int> v={4,3,2,1};
   std::vector<int> ret=Solution().plusOne(v);
   if(ret[0]==4&&ret[1]==3&&ret[2]==3&&ret[3]==2)
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