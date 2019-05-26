
#include <pthread.h>
#define MAX_TASKS 1000

typedef void *Value;

typedef struct Node *Node;

// Value is the parameter passed to a worker
struct Node
{
  Value value;
  struct Node *next;
} node;

typedef struct Queue *Queue;
//count: number of unused nodes
//divider: divide the queue into used and unused parts
struct Queue
{
  Node first;
  Node divider;
  Node last;
  int count;
  pthread_mutex_t task_lock;
} queue;

typedef Queue tq;
// Create a node to initialize the queue, but this node would not be used.
Queue newQueue()
{
  Queue res = (Queue)malloc(sizeof queue);
  Node fst = (Node)malloc(sizeof(node));
  Node lst;
  Node dvd;
  fst->value = NULL;
  lst = fst;
  dvd = fst;
  res->first = fst;
  res->last = lst;
  res->divider = dvd;
  res->count = 0;
  pthread_mutex_init(&res->task_lock, NULL);
  return res;
}

tq newtq()
{
  Queue tq = newQueue();
  return (tq);
}

// Move the divider to get next unused node
void *gettask(tq tq)
{
  void *res = NULL;
  pthread_mutex_lock(&tq->task_lock);
  if (tq->divider != tq->last)
  {
    tq->count--;
    res = tq->divider->next->value;
    tq->divider = tq->divider->next;
  }
  pthread_mutex_unlock(&tq->task_lock);
  return res;
}

// Put the new node after the current last node then set it as the new last node
void puttask(tq tq, void *v)
{
  Node newNode = (Node)malloc(sizeof(node));
  newNode->value = v;
  pthread_mutex_lock(&tq->task_lock);
  tq->count++;
  tq->last->next = newNode;
  tq->last = tq->last->next;
  pthread_mutex_unlock(&tq->task_lock);
}

typedef struct WorkerNode *WorkerNode;
// Worker is the worker function of the node, tq is the queue of parameters
struct WorkerNode
{
  void *(*worker)(void *i);
  tq tq;
  struct WorkerNode *next;
  int count;
  int maxthreads;
} workernode;

// Node_count is the number of stages, count is the number of workers in each stage
typedef struct WorkerQueue *WorkerQueue;
struct WorkerQueue
{
  WorkerNode first;
  WorkerNode divider;
  WorkerNode last;
  int node_count;
  int count;
  void * initpara;
  pthread_mutex_t task_lock;
} workerqueue;

typedef WorkerQueue wq;


WorkerNode getworker(wq wq)
{
  void *res = NULL;
  pthread_mutex_lock(&wq->task_lock);
  if (wq->divider != wq->last)
  {
    wq->node_count--;
    res = wq->divider->next;
    wq->divider = wq->divider->next;
  }
  pthread_mutex_unlock(&wq->task_lock);
  return res;
}

void putworker(wq wq, void *(*Worker)(void *i), int maxthread)
{
  WorkerNode newNode = (WorkerNode)malloc(sizeof(workernode));
  newNode->worker = Worker;
  newNode->count = wq->count;
  newNode->maxthreads = maxthread;
  pthread_mutex_lock(&wq->task_lock);
  wq->node_count++;
  wq->last->next = newNode;
  wq->last = wq->last->next;
  pthread_mutex_unlock(&wq->task_lock);
}

// Set the input set of the first stage
void putinput(wq wq, tq tq){
  wq->first->next->count = tq->count;
  wq->first->next->tq = tq;
  wq->count = tq->count;
}


// Add a stage to an empty worker queue of parameters passed by task queues
void putfirstworker_queue(wq wq, void *(*Worker)(void *i), int maxthread, tq tq){
  putworker(wq, Worker, maxthread);
  putinput(wq, tq);
}

// Add a stage to an empty worker queue of parameters passed by arrays
void putfirstworker_array(wq wq, void *(*Worker)(void *i), int maxthread, int count){
  putworker(wq, Worker, maxthread);
  wq->first->next->count = count;
  wq->count = count;
}

WorkerQueue newWorkerQueue()
{
  WorkerQueue res = (WorkerQueue)malloc(sizeof(workerqueue));
  WorkerNode fst = (WorkerNode)malloc(sizeof(workernode));
  WorkerNode lst;
  WorkerNode dvd;
  fst->count = 0;
  lst = fst;
  dvd = fst;
  res->first = fst;
  res->last = lst;
  res->divider = dvd;
  res->count = 0;
  res->node_count = 0;
  pthread_mutex_init(&res->task_lock, NULL);
  return res;
}

// Create a new worker queue whose parameters are passed by queues;
wq newwq(void *(*Worker)(void *i), int maxthread, void * p, int count)
{
  WorkerQueue wq = newWorkerQueue();
  putworker(wq, Worker,maxthread);
  wq->initpara = p;
  wq->first->next->count = count;
  wq->count = count;
  return (wq);
}

// Create a new worker queue whose parameters are passed by queues;
wq newwq_queue(void *(*Worker)(void *i), int maxthread, tq tq)
{
  WorkerQueue wq = newWorkerQueue();
  putfirstworker_queue(wq, Worker, maxthread,tq);
  return (wq);
}

// Create a new worker queue whose parameters are passed by arrays;
wq newwq_array(void *(*Worker)(void *i), int maxthread, int count)
{
  WorkerQueue wq = newWorkerQueue();
  putfirstworker_array(wq, Worker, maxthread,count);
  return (wq);
}