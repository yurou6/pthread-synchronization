#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

struct states
{
    int numclassmates;
    int robotclass;
    int chair[3];
    int injecttimes[20]; 
    int isinline[20]; //inrobotclass=1,inchair[0]=2,inchair[1]=3,inchair[2]=4
    int firsttime[20]; //leaveclass=5
    int ifinrobot;
};
struct states temp;
struct states *ptr=&temp;
int done = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
time_t timep;
struct tm *p;

void *ThreadEntry(void *id)
{
    const int threadid = (long)id;
    int ifprint=0;
    if(threadid==0)
    {
        while(done<ptr->numclassmates)
        {
            pthread_mutex_lock(&mutex);
            if(ptr->robotclass==0 & ifprint==0)
            {
                time(&timep);
                p=localtime(&timep);
                printf("%d:%d:%d ",p->tm_hour,p->tm_min,p->tm_sec);
                printf("Robot:Sleep\n");
                ifprint=1;
            }
            pthread_mutex_unlock(&mutex);
            pthread_mutex_lock(&mutex);
            if(ptr->robotclass!=0 &&ptr->ifinrobot==0)
            {
                time(&timep);
                p=localtime(&timep);
                printf("%d:%d:%d ",p->tm_hour,p->tm_min,p->tm_sec);
                printf("student%d:Entering to got a shot\n",ptr->robotclass);
                ptr->isinline[ptr->robotclass]=-1;
                ptr->ifinrobot=1;
            }
            pthread_mutex_unlock(&mutex);
            if(ptr->robotclass!=0 && ptr->ifinrobot==1)
            {
                sleep(2);
                ptr->ifinrobot=2;
            }
            pthread_mutex_lock(&mutex);
            if(ptr->robotclass!=0 && ptr->ifinrobot==2)
            {
                time(&timep);
                p=localtime(&timep);
                printf("%d:%d:%d ",p->tm_hour,p->tm_min,p->tm_sec);
                printf("student%d:Leaving the shot robot\n",ptr->robotclass);
                ptr->firsttime[ptr->robotclass]=5;
                ptr->ifinrobot=0;
                ptr->injecttimes[ptr->robotclass]++;
                if(ptr->injecttimes[ptr->robotclass]==3)
                    done++;
                ptr->isinline[ptr->robotclass]=0;
                ptr->robotclass=ptr->chair[0];
                ptr->isinline[ptr->chair[0]]=1;
                if(ptr->robotclass==0)
                    ifprint=0;
                ptr->chair[0]=ptr->chair[1];
                ptr->isinline[ptr->chair[1]]=2;
                ptr->chair[1]=ptr->chair[2];
                ptr->isinline[ptr->chair[2]]=3;
                ptr->chair[2]=0;
                //printf("in line %d,%d,%d,%d\n",ptr->robotclass,ptr->chair[0],ptr->chair[1],ptr->chair[2]);
            }
            pthread_mutex_unlock(&mutex);
        }
        if(done==ptr->numclassmates)
        {
            time(&timep);
            p=localtime(&timep);
            printf("%d:%d:%d ",p->tm_hour,p->tm_min,p->tm_sec);
            printf("Robot:All %d students receive vaccines\n",done);
            pthread_cond_signal(&cond);
        }
    }
    else
    { 
        while(ptr->injecttimes[threadid]!=3)
        {
            if(ptr->firsttime[threadid]==0)
            {
                int w=rand()%11;
                sleep(w);
                //printf("student %d sleep %d in first time\n",threadid,w);
                ptr->firsttime[threadid]++;
            }
            if(ptr->firsttime[threadid]==5)
            {
                int w=rand()%21+10;
                sleep(w);
                //printf("student %d go out %d seconds\n",threadid,w);
                ptr->firsttime[threadid]=1;
            }
            pthread_mutex_lock(&mutex);
            if(ptr->isinline[threadid]==0 && ptr->firsttime[threadid]==1)
            {
                if(ptr->robotclass==0)
                {
                    ptr->robotclass=threadid;
                    ptr->isinline[threadid]=1;
                }
                else
                {
                    if(ptr->chair[0]==0)
                    {
                        ptr->chair[0]=threadid;
                        ptr->isinline[threadid]=2;
                        time(&timep);
                        p=localtime(&timep);
                        printf("%d:%d:%d ",p->tm_hour,p->tm_min,p->tm_sec);
                        printf("student%d:Sitting #1\n",ptr->chair[0]);
                    }
                    else
                    {
                        if(ptr->chair[1]==0)
                        {
                            ptr->chair[1]=threadid;
                            ptr->isinline[threadid]=3;
                            time(&timep);
                            p=localtime(&timep);
                            printf("%d:%d:%d ",p->tm_hour,p->tm_min,p->tm_sec);
                            printf("student%d:Sitting #2\n",ptr->chair[1]);
                        }
                        else
                        {
                            if(ptr->chair[2]==0)
                            {
                                ptr->chair[2]=threadid;
                                ptr->isinline[threadid]=4;
                                time(&timep);
                                p=localtime(&timep);
                                printf("%d:%d:%d ",p->tm_hour,p->tm_min,p->tm_sec);
                                printf("student%d:Sitting #3\n",ptr->chair[2]);
                            }
                        }
                    }
                }
            }
            pthread_mutex_unlock(&mutex);
            if(ptr->isinline[threadid]==0 && ptr->firsttime[threadid]==1)
            {
                int w=rand()%6+5;
                sleep(w);
                //printf("student %d wait for %d and come back\n",threadid,w);
            }
        }
    }
    
    return NULL;
}
 
int main(int argc, char **argv)
{
    if(argc!=3)
        exit(0);
    ptr->numclassmates=atoi(argv[1]);
    if(ptr->numclassmates<10 || ptr->numclassmates>20)
        exit(0);
    int seed=atoi(argv[2]);
    if(seed<0 || seed>100)
        exit(0);
    srand(seed);
    pthread_t threads[ptr->numclassmates];
 
    for (int t = 0; t <= ptr->numclassmates; t++)
        pthread_create(&threads[t], NULL, ThreadEntry, (void *)(long)t);

    pthread_cond_wait(&cond, &mutex);
    printf("Main thread:All students are done%d\n",done);
    //pthread_cond_destroy(&cond);
    for (int t = 0; t <= ptr->numclassmates; t++)
       pthread_join(threads[t], NULL);
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
    exit(0);
}