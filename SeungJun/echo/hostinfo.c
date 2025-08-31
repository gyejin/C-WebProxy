#include "../csapp.h"

int main(int argc, char **argv) {
  struct addrinfo *p, *listp, hints;
  // 입력용이면 지역변수, 출력용이면 힙으로 할당, p는 순회용
  char buf[MAXLINE];  // 8kb 버퍼
  int rc, flags;

  if (argc != 2) {
    printf("인자 한개만 넣어주세용");
    exit(0);
  }

  memset(&hints, 0, sizeof(struct addrinfo));
  // 메모리초기화 쓰레기값 방지
  hints.ai_family = AF_INET;        // IPv4
  hints.ai_socktype = SOCK_STREAM;  // 오직연결

  Getaddrinfo(argv[1], NULL, &hints, &listp);  // 에러처리자동

  flags = NI_NUMERICHOST;  // 도메인이름말고 ip주소리턴
  for (p = listp; p; p = p->ai_next) {
    Getnameinfo(p->ai_addr, p->ai_addrlen, buf, MAXLINE, NULL, 0, flags);
    // buf는 매번 덮어써진다. 내부로직으로 쓰레기값 반환안함
    printf("%s\n", buf);
  }

  Freeaddrinfo(listp);

  exit(0);
}