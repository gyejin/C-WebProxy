#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

//output2.txt 생성 확인
int main() {
    int fd;

    // output.txt 파일을 쓰기 전용으로 엶(없으면 생성, 있으면 덮어씀)
    fd = open("output2.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    // 표준 출력(1)을 output.txt 파일 디스크립터(fd)로 재지정
    if (dup2(fd, STDOUT_FILENO) < 0) {
        perror("dup2");
        close(fd);
        exit(1);
    }

    // fd는 필요 없으니 닫기
    close(fd);

    // 이제부터 printf는 output.txt 에 기록됨
    printf("Hello, this output is redirected to a file!\n");

    return 0;
}
