#pragma once
#include "Log.hpp"
#include <iostream>
extern Log lg;
class TASK//构建任务，就是一旦获取到连接后，就将客户端的网络信息存到任务里，让服务器根据这个信息去服务客户端
{
    
public:    
    TASK(int &sockfd,const std::string& ip,uint16_t& port):_sockfd(sockfd),_clientip(ip),_clientport(port)
    {}
    void run()
    {
         char inbuffer[1024];
         while(true)
         {
          ssize_t n=read(_sockfd,inbuffer,sizeof(inbuffer));
          if(n>0)
          {
            inbuffer[n]=0;
            std::cout<<"client say# "<<inbuffer<<std::endl;
            //加工处理一下
            std::string echo_string="tcpserver加工处理数据：";
            echo_string+=inbuffer;

            //将加工处理的数据发送会去
            write(_sockfd,echo_string.c_str(),echo_string.size());
          }
          else if(n==0)//如果没有用户连接了，那么就会读到0.服务器端也就不要再读了
          {
            lg(Info,"%s:%d quit, server close sockfd: %d",_clientip.c_str(),_clientport,_sockfd);
            break;
          }
          else
          {
            lg(Fatal,"read errno: %d, errstring: %s",errno,strerror(errno));
          }
         }
    }
    void operator()()
    {
        run();
    }
    ~TASK()
    {
    }
public:
   int _sockfd;
   std::string _clientip;
   uint16_t _clientport;
};