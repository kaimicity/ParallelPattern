
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include "queue.h"

// Rename the exit function so that it could be more high-level
void send_result(void *r)
{
  pthread_exit(r);
}

// The parameter of the function of thread-rearrange function of array parameters
typedef struct mypara_array
{
  void *(*Worker)(void *i);
  int n;
  void *mybuf[MAX_TASKS];
} mypara_array;

// The parameter of the function of sub-group functions of queue parameters
typedef struct mypara_queue
{
  void *(*Worker)(void *i);
  int n;
  tq tq1;
} mypara_queue;

// The parameter of the function of sub-group functions of single parameters
typedef struct mypara
{
  void *(*Worker)(void *i);
  int n;
  void *p;
} mypara;

//Create a thread for each worker, pass the same parameter;
void *createfarm_sub(void *(*Worker)(void *i), int n, void *p, tq output)
{
  pthread_t threads[n];
  void *status;
  for (int i = 0; i < n; i++)
  {
    pthread_create(&threads[i], NULL, Worker, p);
  }
  for (int i = 0; i < n; i++)
  {
    pthread_join(threads[i], &status);
    puttask(output, status);
  }
  return NULL;
}

// Run a group of workers linearly, pass the same parameter
void *createthreads(void *myp)
{
  mypara mp = *((mypara *)myp);
  int n = mp.n;
  pthread_t threads[n];
  tq myout = newtq();
  void *p = mp.p;
  for (int i = 0; i < n; i++)
  {
    void *status;
    pthread_create(&threads[i], NULL, mp.Worker, p);
    pthread_join(threads[i], &status);
    puttask(myout, status);
  }
  pthread_exit(myout);
  return NULL;
}

//Create a basic farm, all workers share the same parameter.
//n is the number of workers, p is the parameter
void *createfarm(void *(*Worker)(void *i), int n, void *p, int maxthread)
{
  if(n > MAX_TASKS){
    printf("Error: please do not set over 1000 tasks.\n");
    return NULL;
  }

  tq output = newtq();
  if (n <= maxthread)
    //if number of worker isn't larger the limitation of threads, create one thread for each worker
    createfarm_sub(Worker, n, p, output);
  else
  {
    //if number of worker is larger than the limitation of threads, rearrange workers into max number of threads
    int quotient = n / maxthread;
    int rem = n % maxthread;
    pthread_t threads[maxthread];
    void *status;
    mypara myp[maxthread];
    for (int j = 0; j < rem; j++)
    {
      myp[j].Worker = Worker;
      myp[j].n = quotient + 1;
      myp[j].p = p;
      pthread_create(&threads[j], NULL, createthreads, (void *)&myp[j]);
    }
    for (int j = rem; j < maxthread; j++)
    {
      myp[j].Worker = Worker;
      myp[j].n = quotient;
      myp[j].p = p;
      pthread_create(&threads[j], NULL, createthreads, (void *)&myp[j]);
    }
    for (int j = 0; j < maxthread; j++)
    {
      pthread_join(threads[j], &status);
      while (((tq)status)->count > 0)
      {
        puttask(output, gettask((tq)status));
      }
    }
  }
  return output;
}

void *createfarm_sub_array(void *(*Worker)(void *i), int n, void *buf[], tq out)
{
  pthread_t threads[n];
  void *status;
  for (int i = 0; i < n; i++)
  {
    void *p = buf[i];
    pthread_create(&threads[i], NULL, Worker, p);
  }
  for (int i = 0; i < n; i++)
  {
    pthread_join(threads[i], &status);
    puttask(out, status);
  }
  return NULL;
}

void *createthreads_array(void *myp)
{
  mypara_array mp = *((mypara_array *)myp);
  int n = mp.n;
  pthread_t threads[n];
  tq myout = newtq();
  for (int i = 0; i < n; i++)
  {
    void *p = mp.mybuf[i];
    void *status;
    pthread_create(&threads[i], NULL, mp.Worker, p);
    pthread_join(threads[i], &status);
    puttask(myout, status);
  }
  pthread_exit(myout);
  return NULL;
}

// A refined farm whose workers use parameters passed by an array. The order of out put woulb response 
// to the order of input
void *createfarm_array(void *(*Worker)(void *i), int n, void *buf[], int maxthread)
{
  if(n > MAX_TASKS){
    printf("Error: please do not set over 1000 tasks.\n");
    return NULL;
  }
  tq output = newtq();
  if (n < maxthread)
    createfarm_sub_array(Worker, n, buf, output);
  else
  {
    int quotient = n / maxthread;
    int rem = n % maxthread;
    pthread_t threads[maxthread];
    void *status;
    mypara_array myp[maxthread];
    for (int j = 0; j < rem; j++)
    {
      myp[j].Worker = Worker;
      myp[j].n = quotient + 1;
      for (int k = 0; k < quotient + 1; k++)
      {
        myp[j].mybuf[k] = buf[j * (quotient + 1) + k];
      }
      pthread_create(&threads[j], NULL, createthreads_array, (void *)&myp[j]);
    }
    for (int j = rem; j < maxthread; j++)
    {
      myp[j].Worker = Worker;
      myp[j].n = quotient;
      for (int k = 0; k < quotient; k++)
      {
        myp[j].mybuf[k] = (void *)buf[rem * (quotient + 1) + (j - rem) * quotient + k];
      }
      pthread_create(&threads[j], NULL, createthreads_array, (void *)&myp[j]);
    }
    for (int j = 0; j < maxthread; j++)
    {
      pthread_join(threads[j], &status);
      while (((tq)status)->count > 0)
      {
        puttask(output, gettask((tq)status));
      }
    }
  }
  return output;
}

void *createfarm_sub_queue(void *(*Worker)(void *i), int n, tq tq1, tq output)
{
  pthread_t threads[n];
  void *status;
  for (int i = 0; i < n; i++)
  {
    void *p = gettask(tq1);

    pthread_create(&threads[i], NULL, Worker, p);
  }
  for (int i = 0; i < n; i++)
  {
    pthread_join(threads[i], &status);
    puttask(output, status);
  }
  return NULL;
}

void *createthreads_queue(void *myp)
{
  mypara_queue mp = *((mypara_queue *)myp);
  int n = mp.n;
  pthread_t threads[n];
  tq myout = newtq();
  for (int i = 0; i < n; i++)
  {
    void *p = gettask(mp.tq1);
    void *status;
    pthread_create(&threads[i], NULL, mp.Worker, p);
    pthread_join(threads[i], &status);
    puttask(myout, status);
  }
  pthread_exit(myout);
  return NULL;
}

// A refined farm whose workers use parameters passed by a queue. The order of out put would not response 
// to the order of input
void *createfarm_queue(void *(*Worker)(void *i), tq tq1, int maxthread)
{
  int n = tq1->count;
  if(n > MAX_TASKS){
    printf("Error: please do not set over 1000 tasks.\n");
    return NULL;
  }
  tq output = newtq();
  if (n <= maxthread)
  {
    createfarm_sub_queue(Worker, n, tq1, output);
  }
  else
  {
    int quotient = n / maxthread;
    int rem = n % maxthread;
    pthread_t threads[maxthread];
    void *status;
    mypara_queue myp[maxthread];
    for (int j = 0; j < rem; j++)
    {
      myp[j].Worker = Worker;
      myp[j].n = quotient + 1;
      myp[j].tq1 = tq1;
      pthread_create(&threads[j], NULL, createthreads_queue, (void *)&myp[j]);
    }
    for (int j = rem; j < maxthread; j++)
    {
      myp[j].Worker = Worker;
      myp[j].n = quotient;
      myp[j].tq1 = tq1;
      pthread_create(&threads[j], NULL, createthreads_queue, (void *)&myp[j]);
    }
    for (int j = 0; j < maxthread; j++)
    {
      pthread_join(threads[j], &status);
      while (((tq)status)->count > 0)
      {
        puttask(output, gettask((tq)status));
      }
    }
  }
  return output;
}

// The pipline whose workers use same parameters. 
void *createpipe(WorkerQueue wq)
{
  tq final_res;
  // i is used to identify if it is the first stage
  int i = 0;
  while (wq->node_count > 0)
  {
    WorkerNode wn = getworker(wq);
    tq res;
    if(i == 0)
      res = createfarm(wn->worker, wn->count, wq->initpara, wn->maxthreads);
    else{
      res = createfarm_queue(wn->worker, wn->tq, wn->maxthreads);
    }
    
    if (wq->node_count > 0){
      wn->next->tq = res;
    }else
    {
      final_res = res;
    }
    i ++;
  }
  return final_res;
}

// The pipline whose workers use parameters passed by queues. The order of out put would not response 
// to the order of input
void *createpipe_queue(WorkerQueue wq)
{
  tq final_res;
  while (wq->node_count > 0)
  {
    WorkerNode wn = getworker(wq);

    tq res = createfarm_queue(wn->worker, wn->tq, wn->maxthreads);
    if (wq->node_count > 0)
      wn->next->tq = res;
    else
    {
      final_res = res;
    }
  }
  return final_res;
}

// The pipeline whose workers use parameters passed by an array. The order of out put would response 
// to the order of input
void *createpipe_array(WorkerQueue wq, void *buf[])
{
  void *paras[wq->count];
  tq final_res;
  // i is used to identify if it is the first stage
  int i = 0;
  while (wq->node_count > 0)
  {
    WorkerNode wn = getworker(wq);
    tq res;
    if (i == 0)
      res = createfarm_array(wn->worker, wn->count, buf, wn->maxthreads);
    else
      res = createfarm_array(wn->worker, wn->count, paras, wn->maxthreads);

    if (wq->node_count > 0)
    {
      wn->next->tq = res;
      //transform the returned queue to an array as the parameters of next stage
      for (int i = 0; i < wq->count; i++)
      {
        paras[i] = gettask(res);
      }
      i++;
    }
    else
    {
      final_res = res;
    }
  }
  return final_res;
}