#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

//ls -l file_with_umask.txt
int main() {
    int fd;
    mode_t old_umask;

    // 현재 umask 값 확인하고 0으로 저장
    old_umask = umask(0);
    printf("Old umask: %03o\n", old_umask);

    // 새 umask 설정 (예: 그룹과 다른 사용자 쓰기 권한 제거), 새 파일 생성 시 이 값이 권한에서 제거
    umask(022);
    printf("New umask set to 022\n");

    // 파일 생성: 권한 0666 (rw-rw-rw-)
    fd = open("file_with_umask.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);       //0은 8진수 나타냄, 6은 읽기4 + 쓰기2, 앞에서부터 소유자 | 그룹 | 다른사용자
    if (fd < 0) {
        perror("open");
        exit(1);
    }
    close(fd);

    printf("file_with_umask.txt 파일 생성 완료\n");

    // umask 원래대로 복구
    umask(old_umask);

    return 0;
}
