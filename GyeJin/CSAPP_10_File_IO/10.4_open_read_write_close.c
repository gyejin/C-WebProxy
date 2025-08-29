#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE 1024

//output.txt 확인
int main() {
    int fd_in, fd_out;
    ssize_t nread;
    char buffer[BUF_SIZE];

    // 파일 열기 (읽기 전용)
    fd_in = open("input.txt", O_RDONLY);        // 읽기전용 설정
    if (fd_in < 0) {        // 실패 시 -1반환
        perror("open input.txt");
        exit(1);
    }

    // 파일 생성 및 열기 (쓰기 전용, 없으면 생성)
    fd_out = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);        // 쓰기전용 설정 | 파일존재x, 비어있는 새파일 생성 | 파일존재o, 파일 내용 비움, umask설정 0644[?]
    if (fd_out < 0) {       //실패한 경우 -1반환
        perror("open output.txt");
        close(fd_in);
        exit(1);
    }

    // 입력 파일에서 읽고 출력 파일로 쓰기
    while ((nread = read(fd_in, buffer, BUF_SIZE)) > 0) {       // 입력파일 읽고
        if (write(fd_out, buffer, nread) != nread) {        // 출력파일 쓰기, 주소값이 같지않으면 입력풀력 반환값 다름 에러
            perror("write");
            close(fd_in);
            close(fd_out);
            exit(1);
        }
    }
    if (nread < 0) perror("read");      // -1이면 실패한거니까 에러

    // 파일 닫기
    close(fd_in);
    close(fd_out);

    printf("파일 복사 완료\n");     // 다 통과하면 잘된거
    return 0;
}
