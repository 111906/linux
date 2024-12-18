#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<pthread.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <sys/epoll.h>

//实现了一个基于 TCP 的服务器端程序，提供了多线程、select、poll、epoll几种不同的方式来处理客户端连接及数据收发

void *create_phread(void *arg){

    int clientfd=*(int *)arg;
    while(1){
        char buffer[128]={0};
        int count=recv(clientfd,buffer,128,0);
        if(count==0)
        break;
        send(clientfd,buffer,count,0);

        printf("clientfd: %d,count: %d,buffer: %s\n",clientfd,count,buffer);
    }
    close(clientfd);
}


int main(){

    int sockfd=socket(AF_INET,SOCK_STREAM,0);

    struct  sockaddr_in serveraddr;
    memset(&serveraddr,0,sizeof(struct  sockaddr_in));

    serveraddr.sin_family=AF_INET;
    serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
    serveraddr.sin_port=htons(2048);

    if(-1== bind(sockfd,(struct sockaddr *)&serveraddr,sizeof(struct  sockaddr))){
        perror("bind");
        return -1;
    }
    
    listen(sockfd,10);

#if 0
    while(1){
    struct sockaddr_in clientaddr;
    socklen_t len =sizeof(clientaddr);
    int clientfd=accept(sockfd,(struct sockaddr*)&clientaddr,&len);
    
    pthread_t thid;
    pthread_create(&thid,NULL,create_phread,&clientfd);
    }
#elif 0 //select
      //select(maxfd,rset,wset,eset,timeout);
      fd_set rfds,rset;
      FD_ZERO(&rfds);
      FD_SET(sockfd,&rfds);

      int maxfd=sockfd;

      while (1)
      {
        rset=rfds;

        int nready=select(maxfd+1,&rset,NULL,NULL,NULL);

        if(FD_ISSET(sockfd,&rset)){
            struct sockaddr_in clientaddr;
            socklen_t len =sizeof(clientaddr);
            int clientfd=accept(sockfd,(struct sockaddr*)&clientaddr,&len);

            printf("socket\n");

            FD_SET(clientfd,&rfds);
            maxfd=clientfd;
        }

        int i=0;
        for(i=sockfd+1;i<=maxfd;i++){

            if(FD_ISSET(i,&rset)){
                 char buffer[128]={0};
                 int count=recv(i,buffer,128,0);
                 if(count==0){
                     close(i);
                     FD_CLR(i, &rfds);
                     break;
                 }
                   
                 send(i,buffer,count,0);

                 printf("clientfd: %d,count: %d,buffer: %s\n",i,count,buffer);
            }
        }
      }

#elif 0
   struct pollfd fds[1024]={0};
   fds[sockfd].fd=sockfd;
   fds[sockfd].events=POLLIN;
   
   int maxfd=sockfd;

   while(1)
   {
    int nready=poll(fds,maxfd+1,-1);

    if(fds[sockfd].revents & POLLIN)
    {

        struct sockaddr_in clientaddr;
            socklen_t len =sizeof(clientaddr);
            int clientfd=accept(sockfd,(struct sockaddr*)&clientaddr,&len);

            printf("socket:%d\n",clientfd);

            fds[clientfd].fd=clientfd;
            fds[clientfd].events=POLLIN;

            maxfd=clientfd;
    }
   
  
        int i=0;
        for(i=sockfd+1;i<=maxfd;i++){


            if(fds[i].revents & POLLIN)
            {
                 char buffer[128]={0};
                 int count=recv(i,buffer,128,0);
                 if(count==0)
                 {
                     close(i);
                     fds[i].fd = -1;
                     fds[i].events=0;

                     break;
                 }
                   
                 send(i,buffer,count,0);

                 printf("clientfd: %d,count: %d,buffer: %s\n",i,count,buffer);
            }
        }
   }

#else
    
     
    int epollfd = epoll_create(1);

    struct epoll_event event;
    event.data.fd = sockfd;
    event.events = EPOLLIN;

    epoll_ctl(epollfd,EPOLL_CTL_ADD,sockfd,&event);

    struct epoll_event events[1024];


    while(1){
        int nready = epoll_wait(epollfd, events, 1024, -1);

        int i=0;
        for(i=0;i<nready;i++){
            int connfd=events[i].data.fd;

            if (sockfd==connfd) {

                struct sockaddr_in clientaddr;
                socklen_t len = sizeof(clientaddr);
                int clientfd = accept(sockfd, (struct sockaddr *)&clientaddr, &len);

                printf("socket:%d\n", clientfd);

                 event.data.fd = clientfd;
                 event.events = EPOLLIN;
                 epoll_ctl(epollfd,EPOLL_CTL_ADD,clientfd,&event);


            }else if(events[i].events & EPOLLIN){
                char buffer[128]={0};
                 int count=recv(connfd,buffer,128,0);
                 if(count==0)
                 {
                    epoll_ctl(epollfd,EPOLL_CTL_DEL,connfd,NULL);
                    close(connfd);

                     break;
                 }
                   
                 send(connfd,buffer,count,0);

                 printf("clientfd: %d,count: %d,buffer: %s\n",connfd,count,buffer);
            }
        }

    }

#endif
    

    getchar();
   
}