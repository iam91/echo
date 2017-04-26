//
// Created by zwy on 17-4-25.
//

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<http_parser.h>

int startup(u_short *);
void error(const char *);

void *worker(void *);

int main(){
    int serverSock = -1;
    int clientSock = -1;
    u_short port = 4040;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    pthread_t thread;
    pthread_attr_t workerAttr;

    serverSock = startup(&port);
    printf("Start running: httpd running on port %d\n", port);

    pthread_attr_init(&workerAttr);
    pthread_attr_setdetachstate(&workerAttr, PTHREAD_CREATE_DETACHED);

    while(1){
        clientSock = accept(serverSock,
                             (struct sockaddr *)&clientAddr,
                             &clientAddrLen);
        if (clientSock == -1)
            error("accept");
        if(pthread_create(&thread, &workerAttr, worker, &clientSock)) {
            close(clientSock);
            error("pthread_create");
        }
    }
    close(serverSock);
    return 0;
}

int startup(u_short *port)
{
    int httpd = 0;
    int on = 1;

    struct sockaddr_in name;

    httpd = socket(PF_INET, SOCK_STREAM, 0);
    if (httpd == -1) {
        error("socket");
    }
    memset(&name, 0, sizeof(name)); //set memory of `name` to 0
    name.sin_family = AF_INET;
    name.sin_port = htons(*port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);

    if ((setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0)
    {
        close(httpd);
        error("setsockopt failed");
    }

    if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0){
        close(httpd);
        error("bind");
    }
    if (*port == 0)  /* if dynamically allocating a port */
    {
        socklen_t namelen = sizeof(name);
        //returns the current address to which the socket httpd is bound
        if (getsockname(httpd, (struct sockaddr *)&name, &namelen) == -1) {
            close(httpd);
            error("getsockname");
        }
        *port = ntohs(name.sin_port);
    }
    if (listen(httpd, 5) < 0) {
        close(httpd);
        error("listen");
    }
    return httpd;
}

void *worker(void *args){
    int clientSock = *(int *)args;
    const size_t BUFF_SIZE = 1024;
    char buf[BUFF_SIZE];

    int n = recv(clientSock, buf, BUFF_SIZE, 0);
    send(clientSock, buf, BUFF_SIZE, 0);

    close(clientSock);

    printf("From client: %d\n", clientSock);
    return NULL;
}

void error(const char *sc)
{
    perror(sc);
    exit(1);
}
