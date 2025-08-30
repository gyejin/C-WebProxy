#include "../csapp.h"

void echo(int connfd);

int main(int argc, char **argv) {
  int listenfd, connfd;
  // listenfd는 포트에 대한 디스크립터
  // connfd는 클라이언트와 1:1 매핑된 디스크립터
  socklen_t clientlen;
  // Accept함수에 필요한 구조체의 크기
  struct sockaddr_storage clientaddr;
  // 클라이언트의 정보를 저장하는 구조체. ip와 port등
  char client_hostname[MAXLINE], client_port[MAXLINE];

  if (argc != 2) {
    fprintf(stderr, "포트 입력");
    exit(0);
  }

  listenfd = Open_listenfd(argv[1]);
  // 저 여기있어요~
  while (1) {
    clientlen = sizeof(struct sockaddr_storage);
    // 저장할수 있는 메모리
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    // 클라이언트 연결 대기 → 연결되면 정보 저장 + 새 소켓 반환
    Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE,
                client_port, MAXLINE, 0);
    // 빼온 정보에서 ip주소 포트 빼옴
    printf("연결됨 (%s, %s)\n", client_hostname, client_port);
    echo(connfd);
    Close(connfd);
  }

  exit(0);
}