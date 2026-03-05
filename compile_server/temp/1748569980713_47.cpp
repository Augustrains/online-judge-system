#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<algorithm>

using namespace std;
class Solution{
 public:
        int Max(const vector<int> &v)
        {
            return 0;
        }
};
//TODO

#ifndef COMPILER_ONLINE
#include"header.cpp"
#endif 

void TEST1()
{   
   std::vector<int> v={1,2,3,4,5,6};
   int max=Solution().Max(v);
   if(max==6)
   {
    std::cout<<"通过用例1"<<std::endl;
   }
   else{
    std::cout<<"没有通过用例1"<<std::endl;
   }
}

void TEST2()
{   
    std::vector<int> v={-1,-2,-3,-4,-5,-6};
   int max=Solution().Max(v);
   if(max==-1)
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
