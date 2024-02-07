#pragma once


#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include "Log.hpp"
#include "TASK.hpp"
#include "ThreadPool.hpp"
Log lg;


const std::string defaultip="0.0.0.0";
const int defaultfd=-1;
int backlog=10;//一般不要设置太大
enum 
{
    SockError=2,
    BindError,
    AcceptError,
};
class Tcpserver;

class ThreadData
{
public:
  ThreadData(int &fd,const std::string& ip,uint16_t &port,Tcpserver* svr):_sockfd(fd),_ip(ip),_port(port),_svr(svr)
   {}
public:  
    int _sockfd;
    std::string _ip;
    uint16_t _port;
    Tcpserver* _svr;
};
class Tcpserver
{
public:
     Tcpserver(const uint16_t &port,const std::string &ip=defaultip):_listensock(-1),_port(port),_ip(ip)
     {}

     void Init()
     {
        //服务器端启动之前创建套接字，绑定。
        //一开始的这个套接字是属于监听套接字
        _listensock=socket(AF_INET,SOCK_STREAM,0);
       if(_listensock<0)
       {
         lg(Fatal,"sock create errno:%d errstring:%s",errno,strerror(errno));
         exit(SockError);
       }
       //创建套接字成功
       lg(Info,"sock create sucess listensock:%d",_listensock);
       //创建成功后就要绑定服务器的网络信息
       struct sockaddr_in local;
       memset(&local,0,sizeof(local));
       //填充信息
       local.sin_family=AF_INET;
       local.sin_port=htons(_port);
       inet_aton(_ip.c_str(),&local.sin_addr);
       //填充完毕，真正绑定
       if((bind(_listensock,(struct sockaddr*)&local,sizeof(local)))<0)
       {
        lg(Fatal,"bind errno:%d errstring:%s",errno,strerror(errno));
        exit(BindError);
       }
       lg(Info,"bind socket success listensock:%d",_listensock);//绑定成功
       //udp中绑定成功后就可以进行通信了，但tcp与udp不同。tcp是面向连接的，在通信之前
       //需要先获取新连接，获取到新连接才能进行通信。没有获取连接那么就要等待连接，等待新连接的过程叫做监听，监听有没有新连接。
       //需要将套接字设置成监听状态
       listen(_listensock,backlog);//用来监听，等待新连接，只有具备监听状态才能识别到连接
       
       
      }
     static void* Routine(void *args)//静态成员函数无法使用成员函数，再封装一个服务器对象
     {
      //子线程要和主线程分离，主线程不需要等待子线程，直接回去重新获取新连接
      pthread_detach(pthread_self());
      ThreadData* td=static_cast<ThreadData*>(args);
      //子线程用来服务客户端
      td->_svr->Service(td->_sockfd,td->_ip,td->_port);
      delete td;
      return nullptr;
     }
     void Run()
     {
      //一启动服务器，就将线程池中的线程创建
      ThreadPool<TASK>::GetInstance()->Start();//单例对象
      //静态函数，通过类域就可以使用
      lg(Info,"tcpserver is running");
       while(true)
       {
        struct sockaddr_in client;
        socklen_t len=sizeof(client);
        //将套接字设置成监听状态后，就可以获取新连接
        int sockfd=accept(_listensock,(struct sockaddr*)&client,&len);
        //获取从监听套接字那里监听到的连接。然后返回一个新套接字，通过这个套接字与连接直接通信，而监听套接字继续去监听。
        if(sockfd<0)
        {
          lg(Fatal,"accept error,errno: %d, errstring: %s",errno,strerror(errno));
          exit(AcceptError);
        }
        //获取新连接成功
        //将客户端端网络信息带出来
        uint16_t clientport=ntohs(client.sin_port);
        char clientip[32];
        inet_ntop(AF_INET,&client.sin_addr,clientip,sizeof(clientip));
        
        //根据新连接进行通信

        lg(Info,"get a new link...sockfd: %d,clientip: %s,clientport: %d",sockfd,clientip,clientport);
        //-----------version1 单进程版本
        //Service(sockfd,clientip,clientport); 
        //close(sockfd);//不用了就关闭
      
        //-----------version2 多进程版本
        // pid_t id=fork();
        // if(id==0)//子进程，用来处理服务，父进程用来获取新连接
        // {
        //   close(_listensock);//子进程不需要该文件关闭
        //  if(fork()>0)exit(0);//再创建一个子进程，然后让该进程退出，让孙子进程执行下面的服务，子进程就退出，父进程就等待成功就会重新获取连接
        //  Service(sockfd,clientip,clientport);
        //  close(sockfd);
        //  exit(0);
        // }
        // //父进程只负责用来获取新连接，获取完毕后就交给子进程，自己是不用的，所以关闭
        // close(sockfd);
        // pid_t rid=waitpid(id,nullptr,0);//阻塞等待
        // (void)rid;
       
       //-----------version3 多线程版本
        //  ThreadData *td=new ThreadData(sockfd,clientip,clientport,this);
        //  pthread_t tid;
        //  pthread_create(&tid,nullptr,Routine,td);//Routine要设置成静态成员函数
        

       //----------version4 线程池版本

       //构建任务
       TASK t(sockfd,clientip,clientport); 
       //将任务放进线程池里,线程就会到线程池里去执行任务。

       ThreadPool<TASK>::GetInstance()->Push(t);

       }
      
     }
  
     void Service(int &sockfd,const std::string &clientip,uint16_t &clientport)
     {
         char inbuffer[1024];
         while(true)
         {
          ssize_t n=read(sockfd,inbuffer,sizeof(inbuffer));
          if(n>0)
          {
            inbuffer[n]=0;
            std::cout<<"client say# "<<inbuffer<<std::endl;
            //加工处理一下
            std::string echo_string="tcpserver加工处理数据：";
            echo_string+=inbuffer;

            //将加工处理的数据发送会去
            write(sockfd,echo_string.c_str(),echo_string.size());
          }
          else if(n==0)//如果没有用户连接了，那么就会读到0.服务器端也就不要再读了
          {
            lg(Info,"%s:%d quit, server close sockfd: %d",clientip.c_str(),clientport,sockfd);
            break;
          }
          else
          {
            lg(Fatal,"read errno: %d, errstring: %s",errno,strerror(errno));
          }
         }
     }
     ~Tcpserver()
     {}



private:
    int _listensock;//监听套接字只有一个，监听套接字用来不断获取新的连接。返回新的套接字
    std::string _ip;
    uint16_t _port;
};