#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv) {
  // argc: 인수 개수 (프로그램명 포함)
  // argv: 인수 배열

  uint16_t hex_value;      // 입력받은 16진수 값 (호스트 바이트 순서)
  uint16_t network_value;  // 네트워크 바이트 순서로 변환된 값

  /* 명령행 인자 검사 */
  if (argc != 2) {
    printf("사용법: %s <hex_value>\n", argv[0]);
    printf("예시: %s 0x400\n", argv[0]);
    exit(1);
  }

  /* 16진수 문자열을 정수로 변환 */
  if (sscanf(argv[1], "%hx", &hex_value) != 1) {
    // hex_value에는 값저장 리턴값은 양수면 ok
    fprintf(stderr, "Invalid hex number: %s\n", argv[1]);
    exit(1);
  }

  /* 호스트 바이트 순서를 네트워크 바이트 순서로 변환 */
  network_value = htons(hex_value);

  /* 결과를 십진수로 출력 */
  printf("%u\n", hex_value);
  return 0;
}