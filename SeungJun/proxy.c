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
void parse_uri(char *uri, char *filename, char *port, char *cgiargs);
int connect_server(char *host, char *port);
void request_and_serve(int proxyfd, int clientfd, char *method, char *path, char *host, char *port);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

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

void doit(int clientfd)
{
    // 클라이언트 요청 읽기
    // 요청 파싱 및 검증
    // 원격 서버 연결
    // 요청 전달 및 응답 중계
    // 연결 정리
    rio_t rio;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE]; // HTTP 요청을 파싱할 버퍼들
    char host[512], port[12], path[2048];                               // 안전마진두배
    int proxyfd;

    Rio_readinitb(&rio, clientfd);
    Rio_readlineb(&rio, buf, MAXLINE);
    printf("[doit]Request headers:\n");
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    // GET http://www.cmu.edu/hub/index.html HTTP/1.1

    if (strcasecmp(method, "GET"))
    {
        clienterror(clientfd, method, "501", "Not implemented", "Tiny does not implement this method");
        return;
    }

    read_requesthdrs(&rio);
    parse_uri(uri, host, port, path);
    proxyfd = connect_server(host, port); // 서버와 프록시연결
    request_and_serve(proxyfd, clientfd, method, path, host, port);
}
void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];
    ssize_t n;
    printf("[read_requesthdrs]: before while");
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
    printf("[read_requesthdrs]: after while");
    return;
}

int connect_server(char *host, char *port)
{
    return Open_clientfd(host, port);
}

void parse_uri(char *uri, char *hostname, char *port, char *path)
{
    char *host_start, *port_start, *path_start; // 포인터
    char uri_copy[MAXLINE];                     // 원본보존

    strcpy(uri_copy, uri);

    host_start = uri_copy + 7; // "http://" 길이만큼 건너뛰기

    path_start = strchr(host_start, '/');

    if (path_start)
    {
        strcpy(path, path_start);
        *path_start = '\0'; // host 부분만 남기기 위해 null 종료
    }
    else
    {
        // path가 없는 경우 (http://localhost:9999)
        strcpy(path, "/"); // 기본 path
    }

    port_start = strchr(host_start, ':');

    if (port_start)
    {
        // 포트가 있는 경우
        *port_start = '\0';           // host와 port 분리
        strcpy(hostname, host_start); // hostname 복사
        strcpy(port, port_start + 1); // port 복사 (':'를 제외하고)
    }
    else
    {
        // 포트가 없는 경우
        strcpy(hostname, host_start); // hostname 복사
        strcpy(port, "80");           // 기본 HTTP 포트
    }
    printf("[parse uri] uri: %s", uri);
    printf("[parse uri] host: %s", hostname);
    printf("[parse uri] port: %s", port);
    printf("[parse uri] path: %s", path);
    // http://localhost:9999/home.html
    // 나는 클라이언트다 나는클라이언트다.
}

void request_and_serve(int proxyfd, int clientfd, char *method, char *path, char *host, char *port)
{
    // 서버에 요청하고
    // 클라이언트로 서브하는 과정
    char buf[MAXLINE];
    int n;

    sprintf(buf, "%s %s HTTP/1.0\r\n", method, path);
    Rio_writen(proxyfd, buf, strlen(buf));
    sprintf(buf, "Host: %s:%s\r\n", host, port);
    Rio_writen(proxyfd, buf, strlen(buf));
    Rio_writen(proxyfd, (void *)user_agent_hdr, strlen(user_agent_hdr));
    sprintf(buf, "Connection: close\r\n");
    Rio_writen(proxyfd, buf, strlen(buf));
    sprintf(buf, "Proxy-Connection: close\r\n\r\n");
    Rio_writen(proxyfd, buf, strlen(buf));
    // 헤더

    while ((n = Rio_readn(proxyfd, buf, MAXLINE)) > 0)
    {
        Rio_writen(clientfd, buf, n);
    }
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{                                    // 문자열로 받기때문에 포인터
    char buf[MAXLINE], body[MAXBUF]; // HTTP 응답 헤더용 버퍼와 HTML 본문용 버퍼

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");                     // HTML 문서 시작과 제목 태그 생성
    sprintf(body, "%s<body bgcolor=\"ffffff\">\r\n", body);               // 흰색 배경의 body 태그 추가
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);                // "404: Not found" 같은 에러 정보 표시
    sprintf(body, "%s<p>%s: %s</p>\r\n", body, longmsg, cause);           // 상세한 에러 설명과 원인 표시 (단락으로)
    sprintf(body, "%s<hr><em>The Tiny Web server</em></html>\r\n", body); // 수평선과 서버 정보를 이탤릭체로 표시

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg); // HTTP 응답 시작 라인 생성 (예: "HTTP/1.0 404 Not found")
    Rio_writen(fd, buf, strlen(buf));                     // 응답 시작 라인을 클라이언트에게 전송
    sprintf(buf, "Content-type: text/html\r\n");          // Content-Type 헤더를 HTML로 설정
    Rio_writen(fd, buf, strlen(buf));                     // Content-Type 헤더를 클라이언트에게 전송
    sprintf(buf, "Content-length: %d\r\n\r\n",
            (int)strlen(body));         // 본문 크기를 바이트 단위로 지정하고 헤더 끝을 나타내는 빈 줄 추가
    Rio_writen(fd, buf, strlen(buf));   // Content-Length 헤더와 빈 줄을 클라이언트에게 전송
    Rio_writen(fd, body, strlen(body)); // HTML 본문 내용을 클라이언트에게 전송

    /*
    여기서 헤더는 한줄씩 보내는 이유는
    1. 직관성
    2. MAXLINE은 MAXBUF만큼 크지않다
    */
}