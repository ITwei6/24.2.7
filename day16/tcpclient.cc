#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


void Usage(std::string proc)
{
    std::cout<<"\n\rUsage: "<<proc<<" port[1024+]\n"<<std::endl;
}
//./tcpclient ip port
int main(int args,char* argv[])
{
    if(args!=3)
    {
     Usage(argv[0]);
     exit(1);
    }
    std::string serverip=argv[1];
    uint16_t serverport = std::stoi(argv[2]);
    struct sockaddr_in server;
    socklen_t len=sizeof(server);
    server.sin_family=AF_INET;
    server.sin_port=htons(serverport);
    inet_pton(AF_INET,serverip.c_str(),&server.sin_addr);

    
    //创建套接字
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0)
    {
        std::cout<<"create sockfd err "<<std::endl;
    }
    //创建套接字成功，创建完套接字后该干什么?
    //连接服务器端的套接字，所以客户端用户需要知道服务器端的网络信息的
    int n=connect(sockfd,(struct sockaddr*)&server,len);
    if(n<0)
    {
        std::cout<<"connect sock err..."<<std::endl;
        exit(2);
    }
    //连接成功
    //连接成功后，就可以直接通信了，就可以直接给对方写消息了。
    std::string message;
    while(true)
    {
        std::cout<<"Please enter#";
        getline(std::cin,message);
        //往套接字里写
        write(sockfd,message.c_str(),message.size());

        char outbuffer[1024];
        //接收服务器发送的加工处理消息

        int n=read(sockfd,outbuffer,sizeof(outbuffer));
        if(n>0)
        {
            outbuffer[n]=0;
            std::cout<<outbuffer<<std::endl;
        }
        
    }
    close(sockfd);
    return 0;
 
}