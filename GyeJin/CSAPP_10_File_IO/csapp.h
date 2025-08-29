#ifndef __CSAPP_H__
#define __CSAPP_H__

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

// 오류 처리 함수 (간단한 오류 출력 후 종료)
void unix_error(char *msg);

// Open, Close, Read, Write 함수 래퍼 (오류 검사 포함)
int Open(const char *pathname, int flags, mode_t mode);
int Close(int fd);
ssize_t Read(int fd, void *buf, size_t count);
ssize_t Write(int fd, const void *buf, size_t count);

// RIO (Robust I/O) 패키지 타입과 함수 선언

// 버퍼 사이즈
#define RIO_BUFSIZE 8192

typedef struct {
    int rio_fd;                // 파일 디스크립터
    int rio_cnt;               // 버퍼에 남은 읽지 않은 바이트 수
    char *rio_bufptr;          // 다음 읽을 바이트 위치 포인터
    char rio_buf[RIO_BUFSIZE]; // 내부 버퍼
} rio_t;

// RIO 함수
void rio_readinitb(rio_t *rp, int fd);
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);
ssize_t rio_readn(int fd, void *usrbuf, size_t n);
ssize_t rio_writen(int fd, void *usrbuf, size_t n);

#endif
