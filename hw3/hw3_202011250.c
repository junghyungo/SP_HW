// 202011250 고정현

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {

    if (argc != 3) {    // 입력 오류
        fprintf(stderr, "Usage: %s file_name processes_num\n", argv[0]);    // ex) ./hw3_202011250 sample1.txt 3
        return 1;
    }

    char *filename = argv[1];   // 파일명
    int processNum = atoi(argv[2]); // 자식 프로세스 갯수

    int size = 0;

    FILE *fp = fopen(filename, "r");  // 파일 열기
    if (fp == NULL) { // 파일 열기 오류
        perror("Failed to open file");
    }
    fseek(fp, 0, SEEK_END); // 파일 끝으로 이동
    size = ftell(fp); // 파일 포인터의 현재위치 즉 파일의 크기
    fclose(fp);

    int childSize = size / processNum;    // 담당 범위
    int remainSize = size % processNum;    // 균등하게 나누어지지 않은 나머지 부분

    int start = 0;  // 범위
    int end = 0;

    pid_t childPid;
    int pipeFd[2];  // 파이프 파일디스크립터

    if (pipe(pipeFd) == -1) { // 파이프 생성
        perror("Failed to create the pipe");
        return 1;
    }

    for (int i=0 ; i<processNum ; i++) {    // 지정한 자식 프로세스수 만큼
    
        // 범위
        if (i != 0) {
            start = end+1;
        }
        end = start + childSize - 1;
        if (i == processNum-1) {
            end += remainSize;  // 마지막 자식 프로세스의 경우 나머지 범위 추가
        }

        childPid = fork();  // 포크

        if (childPid == -1) {   // 오류
            perror("Failed to fork");
            return 1;
        }

        if (childPid == 0) { // 자식
            int num = 0;    // 알파벳 캐릭터 갯수
            char buf[1];    // 1바이트(1캐릭터)씩 읽기 위해
            int fd = open(filename, O_RDONLY);  // 파일 읽기모드로 열기

            if (fd == -1) { // open() 오류
                perror("Failed to open fp");
                return 1;
            }

            lseek(fd, start, SEEK_SET); // 파일의 시작 부분에서 +start 만큼 위치

            for (int i=start ; i<=end ; i++) {  // 오프셋 범위 만큼
                if (read(fd, buf, 1) == 1) {    // 실제 1 만큼 읽었다면
                    if ((buf[0] >= 'A' && buf[0] <= 'Z') || (buf[0] >= 'a' && buf[0] <= 'z')) { // 읽은 캐릭터가 a-z,A-Z의 알파벳이라면
                        num++;
                    }
                }
                else {
                    break;
                }
            }

            write(pipeFd[1], &num, sizeof(num));    // 파이프에 알파벳 개수 쓰기
            close(fd);
            printf("Process[%d] has found %d alphabet letters in (%d~%d).\n", getpid(), num, start, end);

            return 1;   // 더 이상 반복문 실행(fork)하지 않고 종료
        }
    }

    if (childPid == 0) {
        return 0;
    }
    else {  // 부모
        int s;
        int totalNum = 0;   // 전체 알파벳의 개수 총합
        int childNum;   // 자식의 알파벳 개수
        for (int i=0 ; i<processNum ; i++) {
            read(pipeFd[0], &childNum, sizeof(childNum));   // 파이프로 알파벳 개수 가져오기
            totalNum += childNum;
            wait(&s);
        }

        printf("Process[%d] has found %d alphabet letters in %s\n", getpid(), totalNum, filename);
    }

    return 0;
}
