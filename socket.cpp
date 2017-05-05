//
// Created by zwy on 17-5-5.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "socket.h"
#include "server.h"
#include "error.h"

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

void index(int client, const char *echo){
    char buf[SEND_BUFF_SIZE];

    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<html><title>Echo Back</title>\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<body><p>%s\r\n", echo);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</p></body></html>\r\n");
    send(client, buf, strlen(buf), 0);
}

void notFound(int client){
    char buf[SEND_BUFF_SIZE];

    sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<html><title>Not Found</title>\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<body><p>The server could not fulfill\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "your request because the resource specified\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "is unavailable or nonexistent.\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</p></body></html>\r\n");
    send(client, buf, strlen(buf), 0);
}
