#include "../csapp.h"

void echo(int connfd) {
  size_t n;
  char buf[MAXLINE];
  rio_t rio;

  Rio_readinitb(&rio, connfd);
  while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
    printf("받은 데이터 %d byte\n", (int)n);
    // rio함수가 size_t를 반환하기때문에 여기서만 형변환
    Rio_writen(connfd, buf, n);
  }
}