//
// Created by zwy on 17-5-5.
//

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>

#include "server.h"
#include "socket.h"
#include "error.h"
#include "parse.h"

typedef struct EchoFdSet{
    fd_set readSet;
    fd_set writeSet;
    char **buf;
    int *n;
    int fdMax;
} EchoFdSet;

void initEchoFd(EchoFdSet *echoSet, int initFd){
    FD_ZERO(&echoSet->readSet);
    FD_ZERO(&echoSet->writeSet);
    FD_SET(initFd, &echoSet->readSet);
    echoSet->fdMax = initFd;
    echoSet->buf = (char **)malloc(sizeof(char *) * FD_SETSIZE);
    echoSet->n = (int *)malloc(sizeof(int) * FD_SETSIZE);
}

void addEchoFd(EchoFdSet *echoSet, int fd){
    FD_SET(fd, &echoSet->readSet);
    FD_SET(fd, &echoSet->writeSet);
    int oldFdMax = echoSet->fdMax;
    echoSet->fdMax = oldFdMax < fd ? fd : oldFdMax;
    echoSet->buf[fd] = (char *)malloc(sizeof(char) * SERVER_BUFF_SIZE);
    echoSet->n[fd] = 0;
}

void removeEchoFd(EchoFdSet *echoSet, int fd){
    FD_CLR(fd, &echoSet->readSet);
    FD_CLR(fd, &echoSet->writeSet);
    free(echoSet->buf[fd]);
    echoSet->n[fd] = 0;
}

void freeEchoFd(EchoFdSet *echoSet){
    free(echoSet->buf);
    free(echoSet->n);
}

void server(u_short port){
    int serverSock;
    int clientSock;

    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    int n = 0;
    int left = 0;
    char url[URL_LEN];

    timeval timeout;
    fd_set readSet;
    fd_set writeSet;
    EchoFdSet echoFdSet;
    int fd;
    int fdMax;
    int fdReady;

    serverSock = startup(&port);

    //fdset initialization
    initEchoFd(&echoFdSet, serverSock);

    while(1){
        readSet = echoFdSet.readSet;
        writeSet = echoFdSet.writeSet;
        fdMax = echoFdSet.fdMax;
        timeout.tv_usec = 0;
        timeout.tv_sec = 0;

        if((fdReady = select(fdMax + 1, &readSet, &writeSet, NULL, &timeout)) == -1){
            error(ERR_SELECT);
        }
        if(fdReady == 0){ continue; }
        for(fd = 0; fd <= fdMax; fd++){

            if(FD_ISSET(fd, &readSet)){
                if(fd == serverSock){
                    //handle accept
                    clientSock = accept(serverSock, (struct sockaddr *)&clientAddr, &clientAddrLen);
                    if(clientSock == -1){
                        close(serverSock);
                        error(ERR_ACCEPT);
                    }
                    addEchoFd(&echoFdSet, clientSock);
                    printf("Connected to client: %d.\n", clientSock);
                }else{
                    n = read(fd, echoFdSet.buf[fd], SERVER_BUFF_SIZE);
                    if(n == 0){
                        removeEchoFd(&echoFdSet, fd);
                        close(fd);
                        printf("Client %d disconnected.\n", fd);
                    }
                    echoFdSet.n[fd] = n;
                }
            }
            if(FD_ISSET(fd, &writeSet)){
                left = echoFdSet.n[fd];
                if(left > 0){
                    getUrl(echoFdSet.buf[fd], left, url);
                    index(fd, url);
                    removeEchoFd(&echoFdSet, fd);
                    close(fd);
                }
            }
        }
    }
    close(serverSock);
    freeEchoFd(&echoFdSet);
}