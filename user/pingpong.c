#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char* argv[])
{
    int p[2];
    pipe(p);
    int pid = fork();
    if(pid==0){
        char buf[10];
        read(p[0], buf, sizeof(buf));
        close(p[0]);
        printf("%d : received ", getpid());
        printf(buf);
        printf("\n");

        write(p[1], "pong", 5);

    }
    else{
        char buf[10];
        write(p[1], "ping", 5);
        close(p[1]);
        wait(0);
        read(p[0], buf, sizeof(buf));
        printf("%d : received ", getpid());
        printf(buf);
        printf("\n");
    }
    // wait(0);
    exit(0);
}