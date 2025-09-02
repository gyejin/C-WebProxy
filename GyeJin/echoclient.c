#include "csapp.h"

int main(int argc, char **argv)
{
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    printf("Type message: ");
    while (Fgets(buf, MAXLINE, stdin) != NULL) {
        Rio_writen(clientfd, buf, strlen(buf)); //서버로 메시지 전송
        Rio_readlineb(&rio, buf, MAXLINE);     //서버로부터 에코 메시지 수신
        Fputs(buf, stdout);                    //화면에 에코 메시지 출력
        printf("Type message: ");
    }
    Close(clientfd);
    exit(0);
}