#include "include/serverFunctions.h"  
#define PORT 2040
#define BUFFERSIZE 10
#define THREADPOLLSIZE 5

int main(int argc, char **argv){
    int portNum = PORT;     
    int threadPoolSize = THREADPOLLSIZE;
    int bufferSize = BUFFERSIZE;
           
    //Initialize buffer(LinkedList)
    init(bufferSize);   
 
    //Setup the server        
    int serverSocketFD = serverSetup(portNum);   
   
    //Allocate memory for the worker threads and create them
    pthread_t *workerThreads;
    workerThreads = (pthread_t*)malloc(sizeof(pthread_t)*threadPoolSize);
    if(workerThreads == NULL){
      perror("Error on malloc"); 
      close(serverSocketFD); 
      exit(1); 
    }  
    
    //concurrency levels in a array
    //informs worker threads when to "wake up" or not
    conc_levels = malloc(sizeof(int)*threadPoolSize);
    conc_levels[0] = 1;
    for(int i = 1; i < threadPoolSize; i++)
        conc_levels[i] = 0;
    for(int i = 0; i < threadPoolSize; i++){
        int *threadNum = malloc(sizeof(int)); 
        *threadNum = i;   
        if (pthread_create(workerThreads+i, NULL, workerThreadFunc, threadNum) != 0) {
                perror("pthread_create");  
                free(workerThreads);
                close(serverSocketFD);
                free(threadNum);
                exit(1);   
            }
    }    
    int jobCommanderFD = -1; 
    struct sockaddr_in clientAddress;
    socklen_t clientAddressSize = sizeof(clientAddress);
        
    while(exitServer){     
      //Accept jobCommander's connection
      if((jobCommanderFD = accept(serverSocketFD, (struct sockaddr *)&clientAddress, &clientAddressSize)) < 0){
          perror("Error on accepting connection");
          close(serverSocketFD);
          exit(1);   
      }          
      // Create a controller thread
      pthread_t controllerThread;
      if(pthread_create(&controllerThread, NULL, controllerThreadFunc, (void*)&jobCommanderFD) != 0){
          perror("pthread_create");
          close(serverSocketFD);
          exit(1); 
      }    
      pthread_join(controllerThread, NULL); 
    } 
 
    close(serverSocketFD);
    free(workerThreads);  
    return 0;  
}            