//
// Created by zwy on 17-5-5.
//

#ifndef ECHO_ERROR_H
#define ECHO_ERROR_H

#define ERR_BIND "bind"
#define ERR_SOCKET "socket"
#define ERR_LISTEN "listen"
#define ERR_ACCEPT "accept"
#define ERR_GETSOCKNAME "getsockname"
#define ERR_SETSOCKETOPT "setsocketopt"
#define ERR_EPOLL__WAIT "epoll_wait"

void error(const char *);

#endif //ECHO_ERROR_H
