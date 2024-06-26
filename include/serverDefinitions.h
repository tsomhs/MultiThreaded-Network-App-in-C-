#include "include/sharedDefinitions.h"

typedef struct job_struct{
  int jobID;
  char job[COMMAND_SIZE];
  int clientSocket;
  struct job_struct *next;
} job_struct;

int *conc_levels;

int wakeUP = 0;
int exitServer = 1;

job_struct *jobsLinkedList, *lastJob;


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

unsigned int jobID_Counter = 1;
unsigned int concurrencyLevel = 1;
 
int isEmpty(void);
void init(int);
int insertJob(job_struct);
int deleteJob(int);
void display(void);
void *controllerThreadFunc(void*);
void *workerThreadFunc(void*);
int serverSetup(int);
int inputCheck(int, char**);
char **separateWords(char *, int*, int);
void *controllerThreadFunc(void *);
void *workerThreadFunc(void *);