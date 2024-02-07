#include "tcpserver.hpp"
#include <iostream>
#include <memory>
#include <cstdio>
void Usage(std::string proc)
{
    std::cout<<"\n\rUsage: "<<proc<<" port[1024+]\n"<<std::endl;
}
//./tcpserver port
int main(int args,char*argv[])
{
     if(args!=2)
     {
        Usage(argv[0]);
        exit(-1);
     }
    //定义一个服务器对象
    uint16_t port=std::stoi(argv[1]);
    std::unique_ptr<Tcpserver> tcpsvr(new Tcpserver(port));
    tcpsvr->Init();
    tcpsvr->Run();
}