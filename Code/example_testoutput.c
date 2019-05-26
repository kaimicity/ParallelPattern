
#include <stdio.h>
#include <time.h>
#include "parpat.h"
#define MAX_THREADS 16

// return the doubled parameter passed in
void *doubleI(void *i)
{
    int a = (int)i;
    a = a * 2;
    send_result((void *)(long)a);
    return NULL;
}

//return the value of parameter+1
void *hello(void *i)
{
    int a = (int)i;
    printf("hello, %d\n", a);
    send_result((void *)(long)(a+1));
    return NULL;
}

int main(){

    tq tq1 = newtq();
    for(int i = 0; i < 100; i++){
        puttask(tq1, (void *)(long) i);
    }
    tq res1 = (tq)createfarm_queue(doubleI, tq1, MAX_THREADS);
    for(int i = 0; i < 100; i++){
        printf("FARM: queue parameters: output%d:%d\n", i, (int)gettask(res1));
    }

    void * buf[100];
    for(int i = 0; i < 100; i ++)
        buf[i] = (void *)(long)i;
    tq res2 = (tq)createfarm_array(doubleI, 100, buf, MAX_THREADS);
    for(int i = 0; i < 100; i++){
        printf("FARM: array parameters: output%d:%d\n", i, (int)gettask(res2));
    }

    tq tq2 = newtq();
    for(int i = 0; i < 100; i++){
        puttask(tq2, (void *)(long) i);
    }
    wq wq1 = newwq_queue(doubleI, MAX_THREADS, tq2);
    putworker(wq1, hello, MAX_THREADS);
    tq res3 = (tq)createpipe_queue(wq1);
    for(int i = 0; i < 100; i++){
        printf("PIPELINE: queue parameters: output%d:%d\n", i, (int)gettask(res3));
    }

    wq wq2 = newwq_array(doubleI, MAX_THREADS, 100);
    putworker(wq2, hello, MAX_THREADS);
    tq res4 = (tq)createpipe_array(wq2, buf);
    for(int i = 0; i < 100; i++){
        printf("PIPELINE: array parameters: output%d:%d\n", i, (int)gettask(res4));
    }


}