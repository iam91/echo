//
// Created by zwy on 17-4-25.
//

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>

int startup(u_short *);
void error_die(const char *);

int main(){
    int serverSock = -1;
    int clientSock = -1;
    u_short port = 4040;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    serverSock = startup(&port);
    printf("Start running: httpd running on port %d\n", port);

    while(1){
        clientSock = accept(serverSock,
                             (struct sockaddr *)&clientAddr,
                             &clientAddrLen);
        if (clientSock == -1)
            error_die("accept");
    }

    return 0;
}

int startup(u_short *port)
{
    int httpd = 0;
    int on = 1;

    struct sockaddr_in name;

    httpd = socket(PF_INET, SOCK_STREAM, 0);
    if (httpd == -1)
        error_die("socket");
    memset(&name, 0, sizeof(name)); //set memory of `name` to 0
    name.sin_family = AF_INET;
    name.sin_port = htons(*port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);

    if ((setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0)
    {
        error_die("setsockopt failed");
    }

    if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0)
        error_die("bind");
    if (*port == 0)  /* if dynamically allocating a port */
    {
        socklen_t namelen = sizeof(name);
        //returns the current address to which the socket httpd is bound
        if (getsockname(httpd, (struct sockaddr *)&name, &namelen) == -1)
            error_die("getsockname");
        *port = ntohs(name.sin_port);
    }
    if (listen(httpd, 5) < 0)
        error_die("listen");
    return httpd;
}

void error_die(const char *sc)
{
    printf("%s\n", sc);
    perror(sc);
    exit(1);
}
