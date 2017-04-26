//
// Created by zwy on 17-4-25.
//

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<http_parser.h>
#include<sys/socket.h>
#include<netinet/in.h>

#define SERVER_STRING "Server: echo\r\n"
#define URL_LEN 40
#define BUFF_SIZE 1024

int startup(u_short *);
void error(const char *);

int getUrl(char *, int, char *);
void notFound(int);
void index(int, const char *);

void *worker(void *);

//parse callbacks
int onUrlParsed(http_parser *, const char *, size_t);
int onHeadersComplete(http_parser * parser){
    printf("On headers complete.\n");
    return 0;
}

int main(){
    int serverSock;
    int clientSock;
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
    char buf[BUFF_SIZE];
    char url[URL_LEN];

    //TODO refine io
    int n = recv(clientSock, buf, BUFF_SIZE, 0);

    getUrl(buf, n, url);

    index(clientSock, url);

    close(clientSock);

    printf("From client: %d\n", clientSock);
    return NULL;
}

int getUrl(char *buf, int n, char *url){
    http_parser_settings settings;
    http_parser headParser;

    http_parser_settings_init(&settings);
    http_parser_init(&headParser, HTTP_REQUEST);
    settings.on_url = onUrlParsed;
    settings.on_headers_complete = onHeadersComplete;
    headParser.data = url;

    http_parser_execute(&headParser, &settings, buf, n);

    return strlen(url);
}

int onUrlParsed(http_parser * parser, const char *at, size_t len){
    char *url = (char *)parser->data;
    strncpy(url, at, len > URL_LEN ? URL_LEN : len);
    url[len] = '\0';
    return 0;
}

void error(const char *sc)
{
    perror(sc);
    exit(1);
}

void index(int client, const char *echo){
    char buf[1024];

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
    char buf[1024];

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
