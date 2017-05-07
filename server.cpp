//
// Created by zwy on 17-5-5.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#include "server.h"
#include "socket.h"
#include "error.h"
#include "parse.h"

#define MAX_EVENTS 1024
#define EPOLL_SIZE_PLACEHOLDER 1

typedef struct EchoSet{
    char **buf;
    int *n;
} EchoSet;

void initEchoSet(EchoSet *echoSet){
    echoSet->buf = (char **)malloc(sizeof(char *) * MAX_EVENTS);
    echoSet->n = (int *)malloc(sizeof(int) * MAX_EVENTS);
}

void addEchoSet(EchoSet *echoSet, int fd){
    echoSet->buf[fd] = (char *)malloc(sizeof(char *) * SERVER_BUFF_SIZE);
    echoSet->n[fd] = 0;
}

void removeEchoSet(EchoSet *echoSet, int fd){
    free(echoSet->buf[fd]);
    echoSet->n[fd] = 0;
}

void freeEchoSet(EchoSet *echoSet){
    free(echoSet->buf);
    free(echoSet->n);
}

void setEventParam(epoll_event *ev, uint32_t events, int fd){
    memset(ev, 0, sizeof(epoll_event));
    ev->events = events;
    ev->data.fd = fd;
}

void server(u_short port){
    int serverSock;
    int clientSock;

    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    int i;
    int n;
    int fd;
    int left;
    char url[URL_LEN];

    int epfd;
    int evCnt;
    uint32_t ev;
    struct epoll_event event;
    struct epoll_event *epollEvents;
    EchoSet echoSet;

    serverSock = startup(&port);

    initEchoSet(&echoSet);
    epollEvents = (epoll_event *)malloc(sizeof(epoll_event) * MAX_EVENTS);
    epfd = epoll_create(EPOLL_SIZE_PLACEHOLDER);

    //add serverSock
    setEventParam(&event, EPOLLIN, serverSock);
    epoll_ctl(epfd, EPOLL_CTL_ADD, serverSock, &event);

    while(1){
        evCnt = epoll_wait(epfd, epollEvents, MAX_EVENTS, -1);
        if(evCnt == -1){ error(ERR_EPOLL__WAIT); }

        for(i = 0; i < evCnt; i++){
            fd = epollEvents[i].data.fd;
            ev = epollEvents[i].events;
            if(fd == serverSock && ev & EPOLLIN){
                clientSock = accept(serverSock, (struct sockaddr *)&clientAddr, &clientAddrLen);
                if(clientSock == -1){
                    close(serverSock);
                    error(ERR_ACCEPT);
                }
                setEventParam(&event, EPOLLIN | EPOLLOUT, clientSock);
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientSock, &event);
                addEchoSet(&echoSet, clientSock);
                printf("Connected to client: %d.\n", clientSock);
            }else{
                if(ev & EPOLLIN){
                    n = read(fd, echoSet.buf[fd], SERVER_BUFF_SIZE);
                    if(n == 0){
                        removeEchoSet(&echoSet, fd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event);

                        close(fd);
                        printf("Client %d disconnected.\n", fd);
                    }
                    echoSet.n[fd] = n;
                }else if(ev & EPOLLOUT){
                    left = echoSet.n[fd];
                    if(left > 0){
                        getUrl(echoSet.buf[fd], left, url);
                        index(fd, url);

                        removeEchoSet(&echoSet, fd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event);

                        close(fd);
                    }
                }
            }
        }
    }
    close(epfd);
    close(serverSock);
    freeEchoSet(&echoSet);
    free(epollEvents);
}