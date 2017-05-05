//
// Created by zwy on 17-5-5.
//

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "server.h"
#include "parse.h"

void server(u_short port){
    int serverSock;
    int clientSock;

    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    int n;
    char echoBuf[SERVER_BUFF_SIZE];
    char url[URL_LEN];

    while(1){
        printf("on port: %d\n", port);
    }
}