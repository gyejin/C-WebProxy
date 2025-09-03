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
        // 힙에 연결 식별자를 위한 메모리 동적 할당
        // 각 스레드가 고유한 connfd 값을 가져야 하므로 스택이 아닌 힙에 저장
        connfdp = Malloc(sizeof(int));

        // 클라이언트 연결 수락하고 연결 식별자를 힙에 저장
        // Accept()는 블로킹 호출 - 클라이언트 연결을 기다림
        *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        // 새로운 스레드 생성하여 클라이언트 요청을 병렬 처리
        // connfdp를 인자로 전달하여 각 스레드가 자신만의 연결을 처리하도록 함
        Pthread_create(&tid, NULL, thread, connfdp);

        // 메인 스레드는 즉시 다음 클라이언트 연결을 기다리러 돌아감
        // 이전 클라이언트는 별도 스레드에서 처리 중
    }
}

void *thread(void *vargp)
{
    // void 포인터를 int 포인터로 캐스팅하여 연결 식별자 추출
    int connfd = *((int *)vargp);

    // 스레드를 detached 상태로 만들어 종료 시 자동으로 리소스 정리
    // 메인 스레드가 pthread_join()을 호출할 필요 없음
    Pthread_detach(pthread_self());

    // 힙에 할당된 메모리 해제 (메인 스레드에서 Malloc으로 할당했던 것)
    // 메모리 누수 방지
    Free(vargp);

    // 실제 클라이언트 요청 처리 (여기서는 echo, 프록시에서는 doit 호출)
    echo(connfd);

    // 클라이언트와의 연결 종료
    Close(connfd);

    // 스레드 종료 (자동으로 리소스 정리됨 - detached 상태이므로)
    return NULL;
}