#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd1, fd2, fd3;

    fd1 = open("file1.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);    //쓰기전용 설정, 새로 만들기
    if (fd1 < 0) {
        perror("open file1");
        exit(1);
    }
    printf("fd1: %d\n", fd1);       // fd1 descriptor 번호 출력

    fd2 = open("file2.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);    // 쓰기전용 설정, 새로만들기
    if (fd2 < 0) {
        perror("open file2");
        close(fd1);
        exit(1);
    }
    printf("fd2: %d\n", fd2);       // fd2 descriptor 번호 출력

    // fd1 닫기
    if (close(fd1) < 0) {       // 닫아서 fd1 descriptor 번호 사라짐
        perror("close fd1");
        close(fd2);
        exit(1);
    }
    printf("fd1 closed\n");

    // 다시 파일 열기, fd1이 닫혔으므로 동일한 번호 재사용 예상
    fd3 = open("file3.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd3 < 0) {
        perror("open file3");
        close(fd2);
        exit(1);
    }
    printf("fd3: %d\n", fd3);       //fd1꺼 재사용 됨

    // 자원 해제
    close(fd2);
    close(fd3);

    return 0;
}
