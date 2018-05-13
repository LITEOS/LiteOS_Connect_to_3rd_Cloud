/*
 * Copyright (C) 2018-2019 �����@Ҷ���Ƽ�   ΢�ţ�yefanqiu
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "iot_import.h"
#include "netdb.h"
#include "sockets.h"

#define PLATFORM_RHINOSOCK_LOG(format, ...) \
    do { \
        printf("RHINOSOCK %u %s() | "format"\n", __LINE__, __FUNCTION__, ##__VA_ARGS__);\
        fflush(stdout);\
    }while(0);


#ifndef CONFIG_NO_TCPIP
uintptr_t HAL_TCP_Establish(const char *host, uint16_t port)
{
    struct addrinfo hints;
    struct addrinfo *addrInfoList = NULL;
    struct addrinfo *cur = NULL;
    int fd = 0;
    int rc = -1;
    char service[6];

    memset(&hints, 0, sizeof(hints));
    
    PLATFORM_RHINOSOCK_LOG("establish tcp connection with server(host=%s port=%u)\r\n", host, port);

    hints.ai_family = AF_INET; //only IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    sprintf(service, "%u", port);

    if ((rc = getaddrinfo(host, service, &hints, &addrInfoList)) != 0) {
        PLATFORM_RHINOSOCK_LOG("getaddrinfo error\r\n");
        return rc;
    }

    for (cur = addrInfoList; cur != NULL; cur = cur->ai_next) {
        if (cur->ai_family != AF_INET) {
            PLATFORM_RHINOSOCK_LOG("socket type error\r\n");
            rc = -1;
            continue;
        }

        fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (fd < 0) {
            PLATFORM_RHINOSOCK_LOG("create socket error\r\n");
            rc = -1;
            continue;
        }

        if (connect(fd, cur->ai_addr, cur->ai_addrlen) == 0) {
            PLATFORM_RHINOSOCK_LOG("socket connect success\r\n");
            rc = fd;
            break;
        }

        close(fd);
        PLATFORM_RHINOSOCK_LOG("connect error\r\n");
        rc = -1;
    }
    
    if (-1 == rc){
        PLATFORM_RHINOSOCK_LOG("fail to establish tcp\r\n");
    } else {
        PLATFORM_RHINOSOCK_LOG("success to establish tcp, fd=%d\r\n", rc);
    }
    freeaddrinfo(addrInfoList);
    return (uintptr_t)rc;
}


int32_t HAL_TCP_Destroy(uintptr_t fd)
{
    int rc;

    //Shutdown both send and receive operations.
    rc = shutdown((int) fd, 2);
    if (0 != rc) {
        perror("shutdown error\r\n");
        return -1;
    }

    rc = close((int) fd);
    if (0 != rc) {
        perror("closesocket error\r\n");
        return -1;
    }

    return 0;
}

uint64_t aliot_platform_time_left(uint64_t t_end, uint64_t t_now)
{
    uint64_t t_left;

    if (t_end > t_now) {
        t_left = t_end - t_now;
    } else {
        t_left = 0;
    }

    return t_left;
}

int32_t HAL_TCP_Write(uintptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms)
{
    int ret, err_code;
    uint32_t len_sent;
    uint64_t t_end, t_left;
    fd_set sets;

    t_end = HAL_UptimeMs( ) + timeout_ms;
    len_sent = 0;
    err_code = 0;
    ret = 1; //send one time if timeout_ms is value 0

    do {
        t_left = aliot_platform_time_left(t_end, HAL_UptimeMs( ));

        if (0 != t_left) {
            struct timeval timeout;

            FD_ZERO( &sets );
            FD_SET(fd, &sets);

            timeout.tv_sec = t_left / 1000;
            timeout.tv_usec = (t_left % 1000) * 1000;

            ret = select(fd + 1, NULL, &sets, NULL, &timeout);
            if (ret > 0) {
               if (0 == FD_ISSET(fd, &sets)) {
                    PLATFORM_RHINOSOCK_LOG("Should NOT arrive\r\n");
                    //If timeout in next loop, it will not sent any data
                    ret = 0;
                    continue;
                }
            } else if (0 == ret) {
                    PLATFORM_RHINOSOCK_LOG("select-write timeout %d\r\n", fd);
                break;
            } else {
                if (EINTR == errno) {
                    PLATFORM_RHINOSOCK_LOG("EINTR be caught\r\n");
                    continue;
                }

                err_code = -1;
                PLATFORM_RHINOSOCK_LOG("select-write fail\r\n");
                break;
            }
        }

        if (ret > 0) {
            ret = send(fd, buf + len_sent, len - len_sent, 0);
            if (ret > 0) {
                len_sent += ret;
            } else if (0 == ret) {
                PLATFORM_RHINOSOCK_LOG("No data be sent\r\n");
            } else {
                if (EINTR == errno) {
                    PLATFORM_RHINOSOCK_LOG("EINTR be caught\r\n");
                    continue;
                }

                err_code = -1;
                PLATFORM_RHINOSOCK_LOG("send fail\r\n");
                break;
            }
        }
    } while((len_sent < len) && (aliot_platform_time_left(t_end, HAL_UptimeMs()) > 0));

    return len_sent;
}


int32_t HAL_TCP_Read(uintptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms)
{
    int ret, err_code;
    uint32_t len_recv;
    uint64_t t_end, t_left;
    fd_set sets;
    struct timeval timeout;

    t_end = HAL_UptimeMs( ) + timeout_ms;
    len_recv = 0;
    err_code = 0;

    do {
        t_left = aliot_platform_time_left(t_end, HAL_UptimeMs());
        if (0 == t_left) {
            break;
        }
        FD_ZERO( &sets );
        FD_SET(fd, &sets);

        timeout.tv_sec = t_left / 1000;
        timeout.tv_usec = (t_left % 1000) * 1000;
    
        ret = select(fd + 1, &sets, NULL, NULL, &timeout);
        if (ret > 0) {
            ret = recv(fd, buf + len_recv, len - len_recv, 0);
            if (ret > 0) {
                len_recv += ret;
            } else if (0 == ret) {
                perror("connection is closed\r\n");
                err_code = -1;
                break;
            } else {
                if (EINTR == errno) {
                    PLATFORM_RHINOSOCK_LOG("EINTR be caught\r\n");
                    continue;
                }
                perror("send fail");
                err_code = -2;
                break;
            }
        } else if (0 == ret) {
            break;
        } else {
            perror("select-recv fail");
            err_code = -2;
            break;
        }
    } while ((len_recv < len));

    //priority to return data bytes if any data be received from TCP connection.
    //It will get error code on next calling
    return (0 != len_recv) ? len_recv : err_code;
}
#endif
