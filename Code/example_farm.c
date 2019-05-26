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
void * payload1_test(void * i){
    int a = (int)i;
    a = fib(900090000);
    void * res = (void *)(long)(a);
    send_result(res);
    return NULL;
}


int main()
{   
    //Evaluation of the farm pattern. There are 10 test examples including the provided example,
    // and some parallel examples with different limitation on number of threads, different number of tasks,  
    //and different way to pass parameters.
    time_t a,b,c,d,e,f,g,h,j,k,l,m,n,o,p,q,r,s,t,u, v, w, x, y, z, aa, bb, cc;
    a = time(NULL);
    int i;
    for (i = 0; i < 100; i++)
    {
        int res = payload1(i);
    }
    b = time(NULL);
    printf("Test1:no paralism\nTime:%ld seconds\n", b - a);

    c = time(NULL);
    createfarm(payload1_test, 100, NULL, MAX_THREADS);
    d = time(NULL);
    printf("Test2:pass the same parameter, maximum 16 threads\nTime:%ld seconds\n", d - c);
    
    e = time(NULL);
    createfarm(payload1_test, 100, NULL, MAX_THREADS * 2);
    f = time(NULL);
    printf("Test3:pass the same parameter, maximum 32 threads\nTime:%ld seconds\n", f - e);
    
    g = time(NULL);
    createfarm(payload1_test, 100, NULL, 100);
    h = time(NULL);
    printf("Test4:pass the same parameter, no limitation on number of threads\nTime:%ld seconds\n", h - g);
    
    j = time(NULL);
    tq tq1 = newtq();
    int n1 = 100;
    for(int i = 0; i < n1; i++){
        puttask(tq1, (void *)(long) i);
    }
    createfarm_queue(payload1_test, tq1, MAX_THREADS);
    k = time(NULL);
    printf("Test5:use a queue to pass different parameters for each worker, maximum 16 threads\nTime:%ld seconds\n", k - j);
    
    l = time(NULL);
    tq tq2 = newtq();
    int n2 = 100;
    for(int i = 0; i < n2; i++){
        puttask(tq2, (void *)(long) i);
    }
    createfarm_queue(payload1_test, tq2, MAX_THREADS * 2);
    m = time(NULL);
    printf("Test6:use a queue to pass different parameters for each worker, maximum 32 threads\nTime:%ld seconds\n", m - l);
    
    n = time(NULL);
    tq tq3 = newtq();
    int n3 = 100;
    for(int i = 0; i < n3; i++){
        puttask(tq3, (void *)(long) i);
    }
    createfarm_queue(payload1_test, tq3, 100);
    o = time(NULL);
    printf("Test7:use a queue to pass different parameters for each worker, no limitation on number of threads\nTime:%ld seconds\n", o - n);

    p = time(NULL);
    void * buf[100];
    for(int i = 0; i < 100; i ++)
        buf[i] = (void *)(long)i;
    createfarm_array(payload1_test, 100, buf,  MAX_THREADS);
    q = time(NULL);
    printf("Test8:use an array to pass different parameters for each worker, maximum 16 threads\nTime:%ld seconds\n", q - p);

    r = time(NULL);
    void * buf2[100];
    for(int i = 0; i < 100; i ++)
        buf2[i] = (void *)(long)i;
    createfarm_array(payload1_test, 100, buf2, MAX_THREADS * 2);
    s = time(NULL);
    printf("Test9:use anarray to pass different parameters for each worker, maximum 32 threads\nTime:%ld seconds\n", s - r);

    t = time(NULL);
    void * buf3[100];
    for(int i = 0; i < 100; i ++)
        buf3[i] = (void *)(long)i;
    createfarm_array(payload1_test, 100, buf3, 100);
    u = time(NULL);
    printf("Test10:use an array to pass different parameters for each worker, no limitation on number of threads\nTime:%ld seconds\n", u - t);

    v = time(NULL);
    createfarm(payload1_test, 200, NULL, MAX_THREADS);
    w = time(NULL);
    printf("Test11:pass the same parameter, maximum 16 threads, 200 tasks\nTime:%ld seconds\n", w - v);

    x = time(NULL);
    tq tq4 = newtq();
    int n4 = 200;
    for(int i = 0; i < n4; i++){
        puttask(tq4, (void *)(long) i);
    }
    createfarm_queue(payload1_test, tq4, MAX_THREADS);
    y = time(NULL);
    printf("Test12:use a queue to pass different parameters for each worker, maximum 16 threads, 200 tasks\nTime:%ld seconds\n", y - x);

    z = time(NULL);
    void * buf4[200];
    for(int i = 0; i < 200; i ++)
        buf4[i] = (void *)(long)i;
    createfarm_array(payload1_test, 200, buf4, MAX_THREADS);
    aa = time(NULL);
    printf("Test13:use an array to pass different parameters for each worker, maximum 16 threads, 200 tasks\nTime:%ld seconds\n", aa - z);
    
    bb = time(NULL);
    int i;
    for (i = 0; i < 200; i++)
    {
        int res = payload1(i);
    }
    cc = time(NULL);
    printf("Test14:no paralism, 200 tasks\nTime:%ld seconds\n", cc - bb);
}