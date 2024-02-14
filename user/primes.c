#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int first_data(int lp[2], int* dst){
    if(read(lp[0], dst, sizeof(int)) == sizeof(int)){       // 对于dst加&为什么会导致写不进去？
        printf("prime %d\n", *dst);
        return 0;
    }
    return -1;
}

void trans_data(int lp[2], int rp[2], int num){
    int data;
    while(read(lp[0], &data, sizeof(int)) == sizeof(int)){
        if(data%num){
            write(rp[1], &data, sizeof(int));
            // printf(" %d ", data);
        }
    }
    // printf("\n");
    close(lp[0]);
    close(rp[1]);
}

void primes(int lp[2]){
    close(lp[1]);
    int first;
    if(first_data(lp, &first)==0){
        int rp[2];
        pipe(rp);
        trans_data(lp, rp, first);

        if(fork()==0){
            primes(rp);
        }
        else{
            close(rp[0]);
            wait(0);
            return;
        }
    }
    exit(0);
}

int main(){
    int p[2];
    pipe(p);

    for(int i = 2; i <= 35; i++){
            write(p[1], &i, sizeof(int));
    }
    if(fork()==0){
        primes(p);
    }
    else{
        close(p[0]);
        
        close(p[1]);
        wait(0);
    }
    exit(0);
}