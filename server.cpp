//
// Created by zwy on 17-5-5.
//

#include <ev.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "server.h"
#include "socket.h"
#include "error.h"
#include "parse.h"

#define MAX_EVENTS 1024

typedef struct EchoSet{
    char **buf;
    int *n;
} EchoSet;

static EchoSet echoSet;

static void initEchoSet(EchoSet *);
static void addEchoSet(EchoSet *, int);
static void removeEchoSet(EchoSet *, int);
static void freeEchoSet(EchoSet *);

static void accpet_cb(struct ev_loop *, ev_io *, int);
static void write_cb(struct ev_loop *, ev_io *, int);
static void read_cb(struct ev_loop *, ev_io *, int);

void initEchoSet(EchoSet *echoSet){
    echoSet->buf = (char **)malloc(sizeof(char *) * MAX_EVENTS);
    echoSet->n = (int *)malloc(sizeof(int) * MAX_EVENTS);
}

void addEchoSet(EchoSet *echoSet, int fd){
    echoSet->buf[fd] = (char *)malloc(sizeof(char) * SERVER_BUFF_SIZE);
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

static void accept_cb(struct ev_loop *loop, ev_io *w, int revents){
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int fd = w->fd;
    int clientSock;

    ev_io *readWatcher = NULL;
    ev_io *writeWatcher = NULL;

    if(revents & EV_READ){
        clientSock = accept(fd, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if(clientSock == -1){
            ev_break(loop, EVBREAK_ALL); //break loop
            error(ERR_ACCEPT);
        }

        writeWatcher = (ev_io *)malloc(sizeof(ev_io));
        readWatcher = (ev_io *)malloc(sizeof(ev_io));

        ev_io_init(readWatcher, read_cb, clientSock, EV_READ);
        ev_io_init(writeWatcher, write_cb, clientSock, EV_WRITE);
        ev_io_start(loop, readWatcher);
        ev_io_start(loop, writeWatcher);

        addEchoSet(&echoSet, clientSock);
        printf("Connected to client: %d.\n", clientSock);
    }
}

static void read_cb(struct ev_loop *loop, ev_io *w, int revents){
    int fd = w->fd;
    int n;

    if(revents & EV_READ){
        n = recv(fd, echoSet.buf[fd], SERVER_BUFF_SIZE, 0);
        if(n == -1){
            printf("errno: %d.\n", errno);
        }
        if(n == 0){
            close(fd);
            printf("Client %d disconnected.\n", fd);
        }
        echoSet.n[fd] = n;
        ev_io_stop(loop, w);
        free(w);
        printf("Get %d from client %d.\n", n, fd);
    }
}

static void write_cb(struct ev_loop *loop, ev_io *w, int revents){
    int fd = w->fd;
    int n;
    char url[URL_LEN];

    if(revents & EV_WRITE){
        n = echoSet.n[fd];
        if(n > 0){
            getUrl(echoSet.buf[fd], n, url);
            index(fd, url);

            removeEchoSet(&echoSet, fd);
            ev_io_stop(loop, w);
            free(w);
            close(fd);
        }
    }
}

void server(u_short port){
    int serverSock;

    struct ev_loop *loop = EV_DEFAULT;

    serverSock = startup(&port);

    initEchoSet(&echoSet);

    ev_io acceptWatcher;

    ev_io_init(&acceptWatcher, accept_cb, serverSock, EV_READ);
    ev_io_start(loop, &acceptWatcher);

    ev_run(loop, 0);

//    while(1){
//        ev_run(loop, 0);
//    }

    close(serverSock);
    freeEchoSet(&echoSet);
}