#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  uint16_t hex_value;
  int result;

  printf("=== sscanf 반환값 상세 분석 ===\n\n");

  printf("1. 성공 사례들 (반환값 = 1):\n");
  char* success_cases[] = {
      "400",     // 16진수
      "0x400",   // 0x 접두사
      "ABCD",    // 대문자
      "abcd",    // 소문자
      "0",       // 영
      "   123",  // 앞쪽 공백 (무시됨)
      "123abc"   // 뒤쪽 무효 문자는 무시
  };

  for (int i = 0; i < 7; i++) {
    result = sscanf(success_cases[i], "%x", &hex_value);
    printf("  \"%s\" → 반환값: %d, 변환값: %u (0x%04X)\n", success_cases[i],
           result, hex_value, hex_value);
  }

  printf("\n2. 실패 사례들 (반환값 = 0):\n");
  char* failure_cases[] = {
      "hello",   // 완전히 잘못된 형식
      "xyz123",  // 첫 문자부터 잘못됨
      "",        // 빈 문자열
      "   ",     // 공백만
      "G123",    // G는 16진수 문자가 아님
      "0xGGG"    // 잘못된 16진수
  };

  for (int i = 0; i < 6; i++) {
    result = sscanf(failure_cases[i], "%x", &hex_value);
    printf("  \"%s\" → 반환값: %d\n", failure_cases[i], result);
  }

  printf("\n3. 여러 항목 변환 테스트:\n");
  int a, b;

  // 두 개 정수 변환 시도
  printf("두 개 10진수 변환:\n");
  result = sscanf("123 456", "%d %d", &a, &b);
  printf("  \"123 456\" → 반환값: %d, a=%d, b=%d\n", result, a, b);

  result = sscanf("123 abc", "%d %d", &a, &b);
  printf("  \"123 abc\" → 반환값: %d, a=%d (b는 변환 실패)\n", result, a);

  result = sscanf("abc 456", "%d %d", &a, &b);
  printf("  \"abc 456\" → 반환값: %d (첫 번째부터 실패)\n", result);

  printf("\n4. 다양한 형식 지정자:\n");
  int int_val;
  float float_val;
  char char_val;

  // %d (10진수)
  result = sscanf("123", "%d", &int_val);
  printf("  \"123\" with %%d → 반환값: %d, 값: %d\n", result, int_val);

  result = sscanf("0x123", "%d", &int_val);
  printf("  \"0x123\" with %%d → 반환값: %d\n", result);

  // %x (16진수)
  result = sscanf("123", "%x", &int_val);
  printf("  \"123\" with %%x → 반환값: %d, 값: %d (0x%X)\n", result, int_val,
         int_val);

  // %f (실수)
  result = sscanf("3.14", "%f", &float_val);
  printf("  \"3.14\" with %%f → 반환값: %d, 값: %.2f\n", result, float_val);

  result = sscanf("abc", "%f", &float_val);
  printf("  \"abc\" with %%f → 반환값: %d\n", result);

  printf("\n=== 핵심 정리 ===\n");
  printf("- 반환값 = 성공적으로 변환된 항목의 개수\n");
  printf("- 형식에 맞지 않으면 → 0 반환\n");
  printf("- 부분 성공도 가능 (예: 2개 중 1개만 성공 → 1 반환)\n");
  printf("- EOF(-1)는 매우 드문 경우 (입력 스트림 에러)\n");

  return 0;
}