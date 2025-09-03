#include "../csapp.h"
void echo(int connfd);
void *thread(void *vargp);

void sigchild_handler(int sig)
{ // 왜 매개변수 안받음?
    while (waitpid(-1, 0, WNOHANG) > 0)
    { // 실제로 좀비 프로세스가 정리되었다는 뜻
    }
    return;
}

int main(int argc, char **argv)
{
    int listenfd, *connfdp;
    socklen_t clientlen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    pthread_t tid;

    if (argc != 2)
    {
        fprintf(stderr, "포트 입력");
        exit(0);
    }

    Signal(SIGCHLD, sigchild_handler); // 한번해놓으면 자식이죽을때마다 실행
    listenfd = Open_listenfd(argv[1]);

    while (1)
    {
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Pthread_create(&tid, NULL, thread, connfdp);
    }
}

void *thread(void *vargp)
{
    int connfd = *((int *)vargp);
    Pthread_detach(pthread_self());
    Free(vargp);
    echo(connfd);
    Close(connfd);
    return NULL;
}