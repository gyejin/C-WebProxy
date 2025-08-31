/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
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
        connfd = Accept(listenfd, (SA *)&clientaddr,
                        &clientlen); // line:netp:tiny:accept
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        doit(connfd);  // line:netp:tiny:doit
        Close(connfd); // line:netp:tiny:close
    }
}

void doit(int fd) // 클라이언트와 연결된 파일 디스크립터를 받아 HTTP 트랜잭션을 처리
{
    int is_static;    // 정적 컨텐츠인지 동적 컨텐츠인지를 나타내는 플래그 변수
    struct stat sbuf; // 파일의 메타데이터(크기, 권한 등)를 저장할 구조체
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE]; // HTTP 요청을 파싱할 버퍼들
    char filename[MAXLINE], cgiargs[MAXLINE];                           // 파일명과 CGI 인자들을 저장할 버퍼
    rio_t rio;                                                          // Robust I/O를 위한 구조체

    /* Read request line and headers */
    Rio_readinitb(&rio, fd);                       // 파일 디스크립터를 rio 구조체와 연결하여 버퍼링된 입력 초기화
    Rio_readlineb(&rio, buf, MAXLINE);             // 클라이언트로부터 첫 번째 요청 라인을 읽음
    printf("Request headers:\n");                  // 디버깅용: 요청 헤더 출력 시작을 알림
    printf("%s", buf);                             // 읽어온 요청 라인을 서버 콘솔에 출력
    sscanf(buf, "%s %s %s", method, uri, version); // 요청 라인을 method, uri, version으로 파싱
    if (strcasecmp(method, "GET"))
    { // 요청 메소드가 GET이 아닌 경우 (대소문자 구분 없이 비교)
        clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
        // 501 에러 응답 전송 Tiny 서버는 GET만 지원한다는 메시지
        return; // 함수 종료
    }
    read_requesthdrs(&rio); // 나머지 HTTP 요청 헤더들을 모두 읽어서 처리

    /* Parse URI from GET request */
    is_static = parse_uri(uri, filename, cgiargs); // URI를 파싱하여 정적/동적 구분, 파일명과 CGI 인자 추출
    if (stat(filename, &sbuf) < 0)
    { // 요청된 파일이 존재하는지 확인 (stat 시스템 콜 사용)
        clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
        // 파일이 없으면 404 에러 응답 전송. 파일을 찾을 수 없다는 메시지
        return; // 함수 종료
    }

    if (is_static)
    { /* Serve static content */ // 정적 컨텐츠 요청인 경우
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
        { // 일반 파일이 아니거나 읽기 권한이 없는 경우
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
            // 403 에러 응답 전송 파일을 읽을 수 없다는 메시지
            return; // 함수 종료
        }
        serve_static(fd, filename, sbuf.st_size); // 정적 파일을 클라이언트에게 전송
    }
    else
    { /* Serve dynamic content */ // 동적 컨텐츠 요청인 경우
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
        { // 일반 파일이 아니거나 실행 권한이 없는 경우
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
            // 403 에러 응답 전송 CGI 프로그램을 실행할 수 없다는 메시지

            return; // 함수 종료
        }
        serve_dynamic(fd, filename, cgiargs); // CGI 프로그램을 실행하여 동적 컨텐츠 생성 및 전송
    }
}

/*
 * clienterror - 클라이언트에게 HTTP 에러 응답을 전송하는 함수
 * 매개변수:
 *   fd: 클라이언트와 연결된 소켓 파일 디스크립터
 *   cause: 에러의 원인이 된 요소 (파일명, 메소드명 등)
 *   errnum: HTTP 상태 코드 (예: "404", "501", "403")
 *   shortmsg: 간단한 에러 메시지 (예: "Not found", "Forbidden")
 *   longmsg: 자세한 에러 설명 (예: "Tiny couldn't find this file")
 */
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

/*
 * read_requesthdrs - HTTP 요청의 나머지 헤더들을 모두 읽어서 처리하는 함수
 * 매개변수:
 *   rp: Rio 버퍼 포인터 (클라이언트와 연결된 소켓에서 데이터를 읽기 위한 구조체)
 *
 * 기능: HTTP 요청 라인 다음에 오는 모든 헤더를 읽고 출력하지만 실제로는 무시함
 *       (Tiny 서버는 단순해서 대부분의 헤더를 처리하지 않음)
 */
void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE]; // 한 줄씩 헤더를 읽어올 버퍼

    Rio_readlineb(rp, buf, MAXLINE); // 첫 번째 헤더 라인을 읽음 (예: "Host: www.example.com")
    while (strcmp(buf, "\r\n"))
    {                                    // 빈 줄("\r\n")을 만날 때까지 반복 (빈 줄은 헤더의 끝을 의미)
        Rio_readlineb(rp, buf, MAXLINE); // 다음 헤더 라인을 읽음
        printf("%s", buf);               // 읽은 헤더를 서버 콘솔에 출력 (디버깅 목적)
    }
    return; // 모든 헤더를 읽었으면 함수 종료
}

/*
 * parse_uri - URI를 파싱하여 정적/동적 컨텐츠를 구분하고 파일명과 CGI 인자를 추출하는 함수
 * 매개변수:
 *   uri: 클라이언트가 요청한 URI 문자열 (예: "/index.html" 또는 "/cgi-bin/adder?100&200")
 *   filename: 파싱된 파일명이 저장될 버퍼 (출력 매개변수)
 *   cgiargs: CGI 인자들이 저장될 버퍼 (출력 매개변수)
 * 반환값:
 *   1: 정적 컨텐츠인 경우
 *   0: 동적 컨텐츠인 경우
 */
int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr; // 문자열 검색을 위한 포인터

    if (!strstr(uri, "cgi-bin"))
    { /* Static content */     // URI에 "cgi-bin" 문자열이 없으면 정적 컨텐츠로 판단
        strcpy(cgiargs, "");   // 정적 컨텐츠이므로 CGI 인자는 빈 문자열로 초기화
        strcpy(filename, "."); // 현재 디렉토리(".")부터 시작하도록 파일명 초기화
        strcat(filename, uri); // URI를 파일명에 연결 (예: "./index.html")
        if (uri[strlen(uri) - 1] == '/')
        {                                  // URI가 '/'로 끝나는 경우 (디렉토리 요청)
            strcat(filename, "home.html"); // 기본 홈페이지 파일명 "home.html"을 추가
        }
        return 1; // 정적 컨텐츠임을 나타내는 1 반환
    }
    else
    { /* Dynamic content */    // "cgi-bin"이 포함되어 있으면 동적 컨텐츠로 판단
        ptr = index(uri, '?'); // URI에서 '?' 문자의 위치를 찾음 (CGI 인자 구분자)
        if (ptr)
        {                             // '?' 문자가 발견된 경우 (CGI 인자가 있음)
            strcpy(cgiargs, ptr + 1); // '?' 다음 부분을 CGI 인자로 복사 (예: "100&200")
            *ptr = '\0';              // '?' 위치에 null 문자를 넣어 URI를 파일명 부분만 남김
        }
        else
        {                        // '?' 문자가 없는 경우 (CGI 인자가 없음)
            strcpy(cgiargs, ""); // CGI 인자를 빈 문자열로 설정
        }
        strcpy(filename, "."); // 현재 디렉토리(".")부터 시작하도록 파일명 초기화
        strcat(filename, uri); // URI를 파일명에 연결 (예: "./cgi-bin/adder")
        return 0;              // 동적 컨텐츠임을 나타내는 0 반환
    }
}

/*
 * serve_static - 정적 컨텐츠를 클라이언트에게 전송하는 함수
 * 매개변수:
 *   fd: 클라이언트와 연결된 소켓 파일 디스크립터
 *   filename: 전송할 파일의 경로명 (예: "./index.html")
 *   filesize: 전송할 파일의 크기 (바이트 단위)
 */
void serve_static(int fd, char *filename, int filesize)
{
    int srcfd;                                  // 정적 파일을 읽기 위한 파일 디스크립터
    char *srcp, filetype[MAXLINE], buf[MAXBUF]; // 메모리 맵 포인터, 파일 타입, HTTP 응답 버퍼

    /* Send response headers to client */
    get_filetype(filename, filetype);    // 파일 확장자를 분석해서 MIME 타입을 결정 (예: ".html" → "text/html")
    sprintf(buf, "HTTP/1.0 200 OK\r\n"); // HTTP 응답 시작 라인 생성 (성공을 나타내는 200 상태 코드)
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);        // 서버 정보를 헤더에 추가
    sprintf(buf, "%sConnection: close\r\n", buf);              // 연결을 닫을 것임을 클라이언트에게 알림
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);   // 전송할 파일의 크기를 바이트 단위로 명시
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype); // MIME 타입 지정하고 헤더 끝을 나타내는 빈 줄 추가
    // 비효율적인 코드맞음. 학습목표
    Rio_writen(fd, buf, strlen(buf)); // 생성된 HTTP 응답 헤더를 클라이언트에게 전송
    printf("Response headers:\n");    // 디버깅용: 서버 콘솔에 응답 헤더 출력 시작을 알림
    printf("%s", buf);                // 생성된 응답 헤더를 서버 콘솔에 출력

    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0);                        // 정적 파일을 읽기 전용으로 열기 파일디스크립터
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); // 파일을 메모리에 맵핑 (추가적인 버퍼를 생성할 필요x)
    Close(srcfd);                   // 파일 디스크립터는 메모리 맵핑 후에는 불필요하므로 즉시 닫기
    Rio_writen(fd, srcp, filesize); // 메모리에 맵핑된 파일 내용을 클라이언트에게 전송
    Munmap(srcp, filesize);         // 메모리 맵핑 해제 (메모리 누수 방지)
}

/*
 * get_filetype - 파일명에서 확장자를 추출하여 MIME 타입을 결정하는 함수
 * 매개변수:
 *   filename: 분석할 파일명 (예: "./images/logo.png")
 *   filetype: MIME 타입이 저장될 버퍼 (출력 매개변수)
 */
void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html"))      // 파일명에 ".html"이 포함되어 있는지 확인
        strcpy(filetype, "text/html");  // HTML 파일의 MIME 타입 설정
    else if (strstr(filename, ".gif"))  // 파일명에 ".gif"가 포함되어 있는지 확인
        strcpy(filetype, "image/gif");  // GIF 이미지의 MIME 타입 설정
    else if (strstr(filename, ".png"))  // 파일명에 ".png"가 포함되어 있는지 확인
        strcpy(filetype, "image/png");  // PNG 이미지의 MIME 타입 설정
    else if (strstr(filename, ".jpg"))  // 파일명에 ".jpg"가 포함되어 있는지 확인
        strcpy(filetype, "image/jpeg"); // JPEG 이미지의 MIME 타입 설정 (표준 MIME 타입은 "jpeg")
    else                                // 위 확장자에 해당하지 않는 모든 파일
        strcpy(filetype, "text/plain"); // 기본값으로 일반 텍스트 파일로 처리
}

/*
 * serve_dynamic - 동적 컨텐츠(CGI 프로그램)를 실행하고 결과를 클라이언트에게 전송하는 함수
 * 매개변수:
 *   fd: 클라이언트와 연결된 소켓 파일 디스크립터
 *   filename: 실행할 CGI 프로그램의 경로 (예: "./cgi-bin/adder")
 *   cgiargs: CGI 프로그램에 전달할 인자 문자열 (예: "100&200")
 */
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = {NULL}; // HTTP 응답 버퍼와 execve용 빈 환경변수 리스트

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");         // HTTP 응답 시작 라인 생성 (성공을 나타내는 200 상태 코드)
    Rio_writen(fd, buf, strlen(buf));            // 응답 시작 라인을 클라이언트에게 전송
    sprintf(buf, "Server: Tiny Web Server\r\n"); // 서버 정보 헤더 생성
    Rio_writen(fd, buf, strlen(buf));            // 서버 정보 헤더를 클라이언트에게 전송

    if (fork() == 0)
    {                                         // 자식 프로세스 생성 (fork() 반환값이 0이면 자식 프로세스)
                                              // 실제 서버라면 모든 CGI 환경변수를 설정해야 함
        setenv("QUERY_STRING", cgiargs, 1);   // CGI 표준: 쿼리 문자열을 QUERY_STRING 환경변수에 설정
        Dup2(fd, STDOUT_FILENO);              // 표준 출력을 클라이언트 소켓으로 리다이렉트
        Execve(filename, emptylist, environ); // CGI 프로그램을 실행 (현재 프로세스 이미지를 교체)
    }
    Wait(NULL); // 부모 프로세스는 자식 프로세스가 끝날 때까지 대기하고 좀비
                // 프로세스 방지
}