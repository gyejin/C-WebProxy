#include "../csapp.h"
void echo(int connfd);

void sigchild_handler(int sig)
{ // 왜 매개변수 안받음?
    while (waitpid(-1, 0, WNOHANG) > 0)
    { // 실제로 좀비 프로세스가 정리되었다는 뜻
    }
    return;
}

int main(int argc, char **argv)
{
    int listenfd, connfd, port;
    socklen_t clientlen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;

    if (argc != 2)
    {
        fprintf(stderr, "포트 입력");
        exit(0);
    }

    Signal(SIGCHLD, sigchild_handler); // 한번해놓으면 자식이죽을때마다 실행
    listenfd = Open_listenfd(argv[1]);

    while (1)
    {
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        if (Fork() == 0) // 자식에서만 처리함
        {
            Close(listenfd); // 부모listenfd 필요없음
            echo(connfd);    // 연결
            Close(connfd);   // connfd 참조삭제
            exit(0);         // 종료
        }
        Close(connfd); // 부모입장에서 connfd 필요없음
    }
}