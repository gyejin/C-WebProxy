#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

//./check_metadata input.txt
int main(int argc, char *argv[]) {      //인자 개수 argc, 인자 인덱스 번호 argv
    if (argc != 2) {
        fprintf(stderr, "사용법: %s <파일명>\n", argv[0]);
        exit(1);
    }

    struct stat st;     //stat 구조체
    if (stat(argv[1], &st) < 0) {       //실패하면 -1 반환
        perror("stat");
        exit(1);
    }

    // 파일 타입 확인
    if (S_ISREG(st.st_mode))
        printf("파일 타입: 일반 파일\n");
    else if (S_ISDIR(st.st_mode))
        printf("파일 타입: 디렉토리\n");
    else
        printf("파일 타입: 기타\n");

    // 파일 크기
    printf("파일 크기: %ld 바이트\n", st.st_size);

    // 읽기 권한 확인 (소유자 기준)
    if (st.st_mode & S_IRUSR)       //파일 권한 확인, 타입 있고, 권한있어야 발동
        printf("소유자 읽기 권한 있음\n");
    else
        printf("소유자 읽기 권한 없음\n");

    return 0;
}
