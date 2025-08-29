#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>

//./listdir /workspaces/C_System_programming/webproxy_lab_docker/test
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "사용법: %s <디렉토리 경로>\n", argv);
        exit(1);
    }

    DIR *dirp = opendir(argv[1]);       //디렉토리 스트림 열기
    if (dirp == NULL) {     //없으면 NULL 반환
        perror("opendir");
        exit(1);
    }

    struct dirent *entry;       //[10.7장] dirent구조체
    errno = 0;
    while ((entry = readdir(dirp)) != NULL) {       //디렉토리 항목 읽기
        printf("항목 이름: %s\n", entry->d_name);       //디렉토리 내 항목 이름
    }
    if (errno != 0) {   //errno가 설정됐으면 에러아님
        perror("readdir");
        closedir(dirp);
        exit(1);
    }

    closedir(dirp);
    return 0;
}
