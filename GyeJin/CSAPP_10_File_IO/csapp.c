#include "csapp.h"

/* 에러 출력 후 종료 */
void unix_error(char *msg) {
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/* Open 래퍼 */
int Open(const char *pathname, int flags, mode_t mode) {
    int rc = open(pathname, flags, mode);
    if (rc < 0)
        unix_error("Open error");
    return rc;
}

/* Close 래퍼 */
int Close(int fd) {
    int rc = close(fd);
    if (rc < 0)
        unix_error("Close error");
    return rc;
}

/* Read 래퍼 */
ssize_t Read(int fd, void *buf, size_t count) {
    ssize_t rc = read(fd, buf, count);
    if (rc < 0)
        unix_error("Read error");
    return rc;
}

/* Write 래퍼 */
ssize_t Write(int fd, const void *buf, size_t count) {
    ssize_t rc = write(fd, buf, count);
    if (rc < 0)
        unix_error("Write error");
    return rc;
}

/********************* Robust I/O (RIO) implementation *******************/

/* rio_readn: n 바이트를 버퍼 usrbuf로 모두 읽도록 시도 */
ssize_t rio_readn(int fd, void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while (nleft > 0) {
        if ((nread = read(fd, bufp, nleft)) < 0) {
            if (errno == EINTR) /* 시그널에 의한 중단 */
                nread = 0;
            else
                return -1;
        } else if (nread == 0) /* EOF */
            break;
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);
}

/* rio_writen: n 바이트를 버퍼 usrbuf에서 모두 쓰도록 시도 */
ssize_t rio_writen(int fd, void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while (nleft > 0) {
        if ((nwritten = write(fd, bufp, nleft)) <= 0) {
            if (errno == EINTR) /* 시그널에 의한 중단 */
                nwritten = 0;
            else
                return -1;
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}

/* rio_readinitb: rio_t 구조체 초기화 */
void rio_readinitb(rio_t *rp, int fd) {
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

/* 내부 rio_read 함수 */
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n) {
    int cnt;

    while (rp->rio_cnt <= 0) {  /* 버퍼가 비었으면 refill */
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        if (rp->rio_cnt < 0) {
            if (errno != EINTR)
                return -1;
        } else if (rp->rio_cnt == 0) /* EOF */
            return 0;
        else
            rp->rio_bufptr = rp->rio_buf; /* 버퍼 포인터 리셋 */
    }

    cnt = n;
    if (rp->rio_cnt < n)
        cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}

/* rio_readlineb: 버퍼에서 최대 maxlen 바이트 한 줄씩 읽기 */
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) {
    int n, rc;
    char c, *bufp = usrbuf;

    for (n = 1; n < maxlen; n++) {
        if ((rc = rio_read(rp, &c, 1)) == 1) {
            *bufp++ = c;
            if (c == '\n') {
                n++;
                break;
            }
        } else if (rc == 0) {
            if (n == 1)
                return 0;  /* EOF, 읽은 데이터 없음 */
            else
                break;     /* EOF, 일부 데이터 읽음 */
        } else
            return -1;     /* 에러 */
    }
    *bufp = 0;
    return n -1;
}

/* rio_readnb: 버퍼에서 최대 n 바이트 읽기 */
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while (nleft > 0) {
        if ((nread = rio_read(rp, bufp, nleft)) < 0)
            return -1;
        else if (nread == 0)
            break; /* EOF */
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);
}
