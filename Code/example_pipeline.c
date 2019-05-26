// gcc example.c -o example

#include <stdio.h>
#include <time.h>
#include "parpat.h"

#define MAX_THREADS 16

int fib(int n)
{
    int i, Fnew, Fold, temp, ans;

    Fnew = 1;
    Fold = 0;
    for (i = 2; i <= n; i++)
    {
        temp = Fnew;
        Fnew = Fnew + Fold;
        Fold = temp;
    }
    ans = Fnew;
    return ans;
}

int payload1(int i)
{
    return (fib(900090000));
}

//Modified payload function to fit the parallel pattern
void *payload1_test(void *i)
{
    int a = (int)i;
    // printf("%d\n", a);
    a = fib(900090000);
    void * res = (void *)(long)(a);
    send_result(res);
    return NULL;
}

int payload2(int j)
{
    return (fib(900090000));
}

//Modified payload function to fit the parallel pattern
void *payload2_test(void *i)
{
    int a = (int)i;
    a = fib(900090000);
    void * res = (void *)(long)(a);
    send_result(res);
    return NULL;
}


int main()
{
    //Evaluation of the pipeline pattern. There are 3 test examples including the provided example,
    // a parallel example passing parameters by queues and a parallel example passing parameters by arrays
    time_t a,b,c,d,e,f;
    a = time(NULL);
    int i;
    for (i = 0; i < 100; i++)
    {
        int res = payload1(i);
        int res2 = payload2(res);
    }
    b = time(NULL);
    printf("Test1:no paralism\nTime:%ld seconds\n", b - a);


    c = time(NULL);
    tq tq1 = newtq();
    for (int i = 0; i < 100; i++)
    {
        puttask(tq1, (void *)(long)i);
    }
    wq wq1 = newwq_queue(payload1_test, MAX_THREADS, tq1);
    putworker(wq1, payload2_test, MAX_THREADS);
    createpipe_queue(wq1);
    d = time(NULL);
    printf("Test2:pipeline with parameters passed by queues, maximum 16 threads \nTime:%ld seconds\n", d - c);


    e = time(NULL);
    void * buf[100];
    for(int i = 0; i < 100; i ++)
        buf[i] = (void *)(long)i;
    wq wq2 = newwq_array(payload1_test, MAX_THREADS, 100);
    putworker(wq2, payload2_test, MAX_THREADS);
    createpipe_array(wq2, buf);
    f = time(NULL);
    printf("Test3:pipeline with parameters passed by arrays, maximum 16 threads \nTime:%ld seconds\n", f - e);


}