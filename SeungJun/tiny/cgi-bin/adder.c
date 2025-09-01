/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "../csapp.h"

int main(void)
{
    char *buf, *p, *n11, *n22;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1 = 0, n2 = 0;

    /* Extract the two arguments */
    if ((buf = getenv("QUERY_STRING")) != NULL) // 환경변수에서 데이터
    {
        p = strchr(buf, '&');
        *p = '\0';
        strcpy(arg1, buf);
        strcpy(arg2, p + 1);
        n11 = strchr(arg1, '=');
        n22 = strchr(arg2, '=');

        strcpy(arg1, n11 + 1);
        strcpy(arg2, n22 + 1);

        n1 = atoi(arg1);
        n2 = atoi(arg2);
    }

    /* Make the response body */
    sprintf(content, "QUERY_STRING=%s\r\n<p>", buf);
    sprintf(content + strlen(content), "Welcome to add.com: ");
    sprintf(content + strlen(content), "THE Internet addition portal.\r\n<p>");
    sprintf(content + strlen(content), "The answer is: %d + %d = %d\r\n<p>", n1, n2, n1 + n2);
    sprintf(content + strlen(content), "Thanks for visiting!\r\n");

    /* Generate the HTTP response */
    printf("Content-type: text/html\r\n");
    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("\r\n");
    printf("%s", content);
    fflush(stdout);

    exit(0);
}
/* $end adder */
