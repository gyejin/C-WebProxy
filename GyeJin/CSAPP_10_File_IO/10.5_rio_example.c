#include "csapp.h"

//키보드 입력 하나 받으면 하나 출력
//gcc -o 10.5_rio_example 10.5_rio_example.c csapp.c 
int main() {
    rio_t rio;
    char buf[RIO_BUFSIZE];
    ssize_t n;

    // 표준 입력과 연결된 rio 버퍼 초기화
    rio_readinitb(&rio, STDIN_FILENO);

    // 한 줄씩 읽어 표준 출력에 쓰기
    while ((n = rio_readlineb(&rio, buf, RIO_BUFSIZE)) != 0) {
        rio_writen(STDOUT_FILENO, buf, n);
    }

    return 0;
}
