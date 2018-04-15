#include <stdio.h>
#include <pthread.h>

#define BUFFER_S 8

void * prodT (void * aargPtr);
void * consT (void * aargPtr);

//char input[32] = {'H','e','l','l','o',' ','W','o','r','l','d','!','\n',0};
char input[32] = "Hello World!";
char * inputPtr = input;

char buffer[BUFFER_S];
int w=0,r=0,s=0;

pthread_mutex_t bufMutex;
pthread_cond_t consCond;
pthread_cond_t prodCond;


int numberOfProducers = 0;


int main()
{
    pthread_attr_t * attrPtr = NULL;
    void * argPtr = NULL;

    void * retProPt;
    void * retConPt;

    pthread_t pTid;
    pthread_t cTid;

    pthread_mutexattr_t * mxAttrPt=NULL;

    pthread_mutex_init(&bufMutex,mxAttrPt);
    pthread_cond_init(&consCond, NULL);
    pthread_cond_init(&prodCond, NULL);
    
    numberOfProducers = 1;

    pthread_create(&pTid , attrPtr , &prodT, argPtr);
    pthread_create(&cTid , attrPtr , &consT, argPtr);
  
    pthread_join(pTid, &retProPt);
    pthread_join(cTid, &retConPt);

    printf("\nFINE!\nw=%d r=%d s=%d\n",w,r,s);

    return 0;
}

void * prodT (void * aargPtr) {
    while(*inputPtr != 0) {
        //printf("prod: waiting for mutex\n");
        pthread_mutex_lock(&bufMutex);
            if (*inputPtr != 0) {
                //printf("prod: there's still something to read\n");
                while(s == BUFFER_S) {
                    //printf("prod: w8in 4 signal\n");
                    pthread_cond_wait(&prodCond, &bufMutex);
                }
                //printf("prod: writing new value: %c\n", buffer[w]);
                buffer[w++] = *inputPtr++;
                w = w % BUFFER_S;
                s++;
            }
        pthread_mutex_unlock(&bufMutex);
        pthread_cond_signal(&consCond);
    }

    pthread_mutex_lock(&bufMutex);
    numberOfProducers--;
    //printf("prod: exiting\n");
    pthread_mutex_unlock(&bufMutex);
}

void * consT (void * aargPtr) {
    // 2 consumers, 1 producer
    // tot to produce: 2
    // mutex order: producer, cons1, cons2
    // produced 1, unlock, cons2 lock, consumed 1, s == 0, unlock
    // cons2 lock, wait, prod lock
    // prod
    //
    // scenario 2:
    // 2 consumers, 1 producer
    // *inputPtr == 0, s == 1
    // 2 consumers enter external while and wait both for mutex
    // cons1: lock - s == 0, unlock and signal, wait mutex
    // prod: lock, num = 0, unlock, signal
    // cons2: unlock, wait
    
    while (numberOfProducers > 0 || s > 0) {
        //printf("cons: waiting for mutex\n");
        pthread_mutex_lock(&bufMutex);
            if (numberOfProducers > 0 || s > 0) {
                while (s == 0) {
                    //printf("cons: w8in 4 signal\n");
                    pthread_cond_wait(&consCond, &bufMutex);
                }
                //printf("cons: reading new value: %c\n", buffer[r]);
                putchar(buffer[r++]);
                r = r % BUFFER_S;
                s--;
            }
        pthread_mutex_unlock(&bufMutex);
        pthread_cond_signal(&prodCond);
    }
}