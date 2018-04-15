#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
/*
Priority Readers and Writers
    readers priority over writers: writers can only write when there are no readers.
    Multiple readers/writers must be supported (5 of each is fine)
    Readers must read the shared variable X number of times
    Writers must write the shared variable X number of times
    Readers must print: The value read;The number of readers present when value is read
    Writers must print: The written value (readers should be 0)
    Before a reader/writer attempts to access the shared variable it should wait some random amount of time
*/
#define READERS_COUNT 5
#define WRITERS_COUNT 5
#define X 10

// This mutex will protect the variable readersCount
pthread_mutex_t mutex;
pthread_cond_t readerCond;
pthread_cond_t writerCond;

// Multiple readers/writers must be supported (5 of each is fine)
// If there's a writer writing, readersCount = -1
int readersCount = 0;
char sharedVariable = 0;

void randomWait(){
  struct timespec ts;
  int ms = (rand() % (WRITERS_COUNT + READERS_COUNT)) * 100;
  ts.tv_nsec = ms * 1000000L;
  nanosleep(&ts, NULL);
}

/**
 * Readers must read the shared variable X number of times
 * Readers must print: The value read;The number of readers present when value is read
 */
void * reader (void* args) {
  for(int i = 0; i < X; i++) {
    randomWait();
    
    pthread_mutex_lock(&mutex);
      readersCount++;  
      int readersCountWhenStarting = readersCount;
    pthread_mutex_unlock(&mutex);

      printf("Read %c, Readers:%d\n", sharedVariable, readersCountWhenStarting);

    pthread_mutex_lock(&mutex);
      readersCount--;
      pthread_cond_signal(&writerCond);
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

/*
    Writers must write the shared variable X number of times
    Writers must print: The written value (readers should be 0)
*/
void * writer (void* args) {
  for(int i = 0; i < X; i++) {
    randomWait();
    
    pthread_mutex_lock(&mutex);
    while (readersCount) {
      //int pthread_cond_wait(pthread_cond_t *restrict cond,
      //        pthread_mutex_t *restrict mutex);
      pthread_cond_wait(&writerCond, &mutex);
    }
    
      // from outside: &writersInput[i] --> (char *)
      // param: void *args
      // 1. cast (void *) to (char *)
      // 2. dereference pointer *(char *)
      sharedVariable = *(char *)args;
      printf("Writing: %c\n", sharedVariable);
    
    pthread_mutex_unlock(&mutex);
    
//    int pthread_cond_broadcast(pthread_cond_t *cond);
//    int pthread_cond_signal(pthread_cond_t *cond);
    pthread_cond_broadcast(&readerCond);
    pthread_cond_signal(&writerCond);
  }
  return NULL;
}

int main(int argc, char* args[]) {
  //init random
  srand(time(NULL));
  pthread_t readers[READERS_COUNT];
  pthread_t writers[WRITERS_COUNT];
  
  // int pthread_mutex_init(pthread_mutex_t *restrict mutex,
  //            const pthread_mutexattr_t *restrict attr);
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&writerCond, NULL);
  pthread_cond_init(&readerCond, NULL);
  
  // struct *ReaderData reader;
  // reader->tid, reader->output[0], ...
  //
  // struct ReaderData reader;
  // reader.tid, reader.output[0], ...
  //
  // struct ReaderData { pthread_t *tId; } reader;
  // reader.tId <- correct
  // a->b <==> (*a).b
  
  char writerInputs[WRITERS_COUNT];
  
  for (int i = 0; i < READERS_COUNT; i++) {
    // We're passing the pointer to the thread, because
    // we want the create method to return it filled.
    // int pthread_create(pthread_t *restrict thread,
    //          const pthread_attr_t *restrict attr,
    //          void *(*start_routine)(void*), void *restrict arg);
      pthread_create(&readers[i], NULL, reader, NULL);
  }
  
  for (int i = 0; i < WRITERS_COUNT; i++) {
    writerInputs[i] = 'A' + (char)i;
    pthread_create(&writers[i], NULL, writer, &writerInputs[i]);
  }
  
  for (int i = 0; i < READERS_COUNT; i++) {
    // int pthread_join(pthread_t thread, void **value_ptr);
    char output[X];
    pthread_join(readers[i], (void **)(&output));
    //printf("%d\t%s\n", i, output);
  }
  
  for (int i = 0; i < WRITERS_COUNT; i++) {
    pthread_join(writers[i], NULL);
  }
  
  printf("Program terminated\n");

  return 0;
}

// char         - simple char
// char*        - pointer to char
// char args[]  - array of chars
// char* args[] - pointer to array of chars 
// char **      - pointer to pointer of chars

 
/*
pthread_mutex_t myMutex;
pthread_mutexattr_t myMutexAttr;
pthread_mutex_init(&myMutex,&myMutexAttr);
pthread_mutex_destroy(&myMutex);

pthread_cond_t myCond;
pthread_condattr_t myCondAttr;
pthread_cond_init(&consCond, &myCondAttr);
pthread_cond_destroy(&myCond);

pthread_cond_signal(&myCond);
pthread_cond_broadcast(&myCond);

pthread_attr_t myThreadAttr;
pthread_t myThread;
pthread_create(&myThread , attrPtr , &consT, &arg);
pthread_join(&myThread, &ret);
*/