#include "csapp.h"
#include <stdio.h>
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000 // 약 1MB
#define MAX_OBJECT_SIZE 102400 // 100kb

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
/*
서버가 User-Agent를 보고 하는 일들

호환성 체크: "IE 구버전이면 다른 CSS 보내자"
모바일 감지: "모바일 브라우저면 모바일 페이지 보내자"
봇 차단: "크롤러면 접근 제한하자"
통계 수집: "Chrome 사용자가 몇 %인지 집계하자"

여기서 헤더를 고정시킨 이유는
같은 헤더환경에서 테스트하기 위함
*/
void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
int connect_server(char *host, char *port);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        // line:netp:tiny:accept
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        doit(connfd);  // line:netp:tiny:doit
        Close(connfd); // line:netp:tiny:close
    }
}

void doit(int fd)
{
    // 클라이언트 요청 읽기
    // 요청 파싱 및 검증
    // 원격 서버 연결
    // 요청 전달 및 응답 중계
    // 연결 정리
    rio_t rio;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE]; // HTTP 요청을 파싱할 버퍼들

    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    printf("Request headers:\n");
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    // GET http://www.cmu.edu/hub/index.html HTTP/1.1

    if (strcasecmp(method, "GET"))
    {
        clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
        return;
    }

    read_requesthdrs(&rio);
}
void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];
    ssize_t n;

    n = Rio_readlineb(rp, buf, MAXLINE);
    while (strcmp(buf, "\r\n"))
    {
        n = Rio_readlineb(rp, buf, MAXLINE);
        if (n <= 0)
        {
            printf("Read failed or EOF\n");
            break;
        }

        printf("%s", buf);
    }
    return;
}

int connect_server(char *host, char *port)
{
    int clientfd; // 생성할 fd
    char buf[MAXLINE];

    clientfd = Open_clientfd(host, port); // 해당주소와포트를 위한 fd생성

    return clientfd;
}