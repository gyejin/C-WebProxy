#include "../csapp.h"

int main(int argc, char **argv) {
  int clientfd;  // 생성할 fd
  char *host, *port, buf[MAXLINE];
  rio_t rio;  // fd로 식별하는 rio객체

  if (argc != 3) {
    fprintf(stderr, "host, port 입력");
    exit(0);
  }

  host = argv[1];
  port = argv[2];

  clientfd = Open_clientfd(host, port);  // 해당주소와포트를 위한 fd생성
  Rio_readinitb(&rio, clientfd);         // rio와, client로 초기화

  while (Fgets(buf, MAXLINE, stdin) != NULL) {
    // stdin으로 저장된 값을 입력
    Rio_writen(clientfd, buf, strlen(buf));
    // clientfd 프로세스에 buf를 저장
    Rio_readlineb(&rio, buf, MAXLINE);
    // 블록킹함수. 데이터가 있어야 읽음
    Fputs(buf, stdout);
    // 화면으로 출력
  }

  Close(clientfd);
  exit(0);
}