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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define ENABLE_HTTP_RESPONST  1



//实现了一个基于 TCP 的服务器端程序，提供了epoll和reactor的方式来处理客户端连接及数据收发
#define BUFFER_LENGTH 1024
typedef int(*RCALLBACK)(int fd,int epollfd);

//listenfd
//listenfd触发EPOLLIN事件的时候执行accept_cb()
int accept_cb(int fd,int epollfd);
//clientfd
//clientfd触发EPOLLIN事件的时候执行recv_cb()
int recv_cb(int fd,int epollfd);
//clietnfd触发EPOLLOUT事件的时候执行send_cb()
int send_cb(int fd,int epollfd);

struct  conn_item
{
    int fd;

    char rbuffer[BUFFER_LENGTH];
    int rlen;
    char wbuffer[BUFFER_LENGTH];
    int wlen;


   union
   {
    RCALLBACK accept_callback;
    RCALLBACK recv_callback;
   }recv_t;
   
    
    RCALLBACK send_callback;
};


struct  conn_item connlist[1048576]={0};

#if ENABLE_HTTP_RESPONST

typedef struct  conn_item connection_t;

int http_response(connection_t *conn){
#if 0
    conn->wlen=sprintf(conn->wbuffer,
    "HTTP/1.1 200 OK\r\n"
    "Accept-Ranges: bytes\r\n"
    "Content-Length: 82\r\n"
    "Content-Type: text/html\r\n"
    "Date: Sat, 06 Aug 2022 13:16:46 GMT\r\n\r\n"
    "<html><head><title>0vioce.king</title></head><body><h1>King</h1></body></html>\r\n\r\n");
#else
     int filefd=open("index.html",O_RDONLY);

     struct stat stat_buf;
     fstat(filefd,&stat_buf);
     
     conn->wlen=sprintf(conn->wbuffer,
    "HTTP/1.1 200 OK\r\n"
    "Accept-Ranges: bytes\r\n"
    "Content-Length: %ld\r\n"
    "Content-Type: text/html\r\n"
    "Date: Sat, 06 Aug 2022 13:16:46 GMT\r\n\r\n",stat_buf.st_size);
    int count=read(filefd,conn->wbuffer+conn->wlen,BUFFER_LENGTH-conn->wlen);
    conn->wlen+=count;


#endif
    return conn->wlen;
}
#endif



int set_event(int fd,int ev,int flag,int epollfd){
    if(flag){
        struct epoll_event event;
        event.data.fd = fd;
        event.events = ev;
        epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    }else{
        struct epoll_event event;
        event.data.fd = fd;
        event.events = ev;
        epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&event);
    }
}

int accept_cb(int fd,int epollfd){

    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    int clientfd = accept(fd, (struct sockaddr *)&clientaddr, &len);
    if(clientfd < 0){
        return -1;
    }

    set_event(clientfd,EPOLLIN,1,epollfd);

    connlist[clientfd].fd=clientfd;
    memset(connlist[clientfd].rbuffer,0,BUFFER_LENGTH);
    connlist[clientfd].rlen=0;

    memset(connlist[clientfd].wbuffer,0,BUFFER_LENGTH);
    connlist[clientfd].wlen=0;


    connlist[clientfd].recv_t.recv_callback=recv_cb;
    connlist[clientfd].send_callback=send_cb;

    if((clientfd%1000)==999){
        printf("clientfd :%d\n",clientfd);
    }

    return clientfd;

}


int recv_cb(int fd,int epollfd){

        char *buffer=connlist[fd].rbuffer;
        int index=connlist[fd].rlen;
        int count=recv(fd,buffer+index,BUFFER_LENGTH-index,0);
        if(count==0)
        {
            epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,NULL);
            close(fd);
            return -1;
        }


        
        connlist[fd].rlen+= count;
#if 1
        memcpy(connlist[fd].wbuffer,connlist[fd].rbuffer,connlist[fd].rlen);
        connlist[fd].wlen=connlist[fd].rlen;
        connlist[fd].rlen-=connlist[fd].rlen;
#else
    //http_request(&connlist[fd]);
    http_response(&connlist[fd]);
#endif

        //设置事件
        set_event(fd,EPOLLOUT,0,epollfd);
        
                        
        //send(fd,buffer,connlist[fd].index,0);
        return count;
}

int send_cb(int fd,int epollfd){
    char *buffer=connlist[fd].wbuffer;
    int index=connlist[fd].wlen;
    int count=send(fd,buffer,index,0);

    set_event(fd,EPOLLIN,0,epollfd);


    return count;
}


int init_server(unsigned short port){
    int sockfd=socket(AF_INET,SOCK_STREAM,0);

    struct  sockaddr_in serveraddr;
    memset(&serveraddr,0,sizeof(struct  sockaddr_in));

    serveraddr.sin_family=AF_INET;
    serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
    serveraddr.sin_port=htons(port);

    if(-1== bind(sockfd,(struct sockaddr *)&serveraddr,sizeof(struct  sockaddr))){
        perror("bind");
        return -1;
    }
    
    listen(sockfd,10);

    return sockfd;

}


int main(){

    int port_count=20;
    unsigned short port=2048;
    int i=0;

    int epollfd = epoll_create(1);

    for(i=0;i<port_count;i++){
        int sockfd=init_server(port+i);
        connlist[sockfd].fd=sockfd;
        connlist[sockfd].recv_t.accept_callback=accept_cb;
        set_event(sockfd,EPOLLIN,1,epollfd);
    }
   
    
    struct epoll_event events[1024];


    while(1){
        int nready = epoll_wait(epollfd, events, 1024, -1);

        int i=0;
        for(i=0;i<nready;i++){
            int connfd=events[i].data.fd;

           
            if(events[i].events & EPOLLIN){
                //int count=recv_cb(connfd,epollfd);
                int count=connlist[connfd].recv_t.recv_callback(connfd,epollfd);

                //printf("count: %d\nrecv-->buffer: %s\n",count,connlist[connfd].rbuffer);
            }else if(events[i].events & EPOLLOUT){

                //int count=send_cb(connfd,epollfd);
                //printf("send-->buffer: %s\n",connlist[connfd].wbuffer);
                int count=connlist[connfd].send_callback(connfd,epollfd);
                

            }
        }

    }

    

    getchar();
   
}