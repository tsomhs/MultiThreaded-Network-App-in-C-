#include "include/serverDefinitions.h"

void init(int bufferSize) {
    // Allocate memory for the jobsLinkedList array
    jobsLinkedList = (job_struct*) malloc(sizeof(job_struct));
    lastJob = jobsLinkedList;
    
    // Initialize job_structs'
    for (int i = 0; i < bufferSize; i++) {
        lastJob->jobID = -1;
        memset(lastJob->job, 0, COMMAND_SIZE);
        lastJob->clientSocket = -1;
        if(i < (bufferSize - 1)){
            lastJob->next = (job_struct*) malloc(sizeof(job_struct));
            lastJob = lastJob->next;
        }
        else
            lastJob->next = NULL;
    }
    lastJob = jobsLinkedList;
}


int insertJob(job_struct new_job) {
    if(lastJob->next == NULL){
        printf(">Too many jobs waiting. Unable to insert more!");
        return 0;
    }
    else{
        pthread_mutex_lock(&mutex);
        if(!isEmpty())
            lastJob = lastJob->next;
        lastJob->jobID = new_job.jobID;
        strcpy(lastJob->job, new_job.job);
        lastJob->clientSocket = new_job.clientSocket;
        printf(">Inserted job_%d\n", lastJob->jobID);
        wakeUP++;
        pthread_mutex_unlock(&mutex);
        return 1;
    }
}

int isEmpty(){
  if (jobsLinkedList->jobID != -1)
    return 0;
  wakeUP = 0;
  return 1;
}

int deleteJob(int jobID) {
    job_struct *last_struct = lastJob;
    while (last_struct->next!=NULL)
        last_struct = last_struct->next;
    if(isEmpty())
        return 0;
    else if(jobsLinkedList->jobID == jobID){ // Check the first job
        if(jobsLinkedList == lastJob){
            pthread_mutex_lock(&mutex);
            jobsLinkedList->jobID = -1;
            memset(jobsLinkedList->job, 0, COMMAND_SIZE);
            jobsLinkedList->clientSocket = -1;
            printf(">Job_%d was removed\n", jobID);
            wakeUP--;
            pthread_mutex_unlock(&mutex);
            return 1;
        }else{
            pthread_mutex_lock(&mutex);
            job_struct *temp_ptr = jobsLinkedList;
            jobsLinkedList = jobsLinkedList->next;
            temp_ptr->jobID = -1;
            memset(temp_ptr->job, 0, COMMAND_SIZE);
            temp_ptr->clientSocket = -1;
            temp_ptr->next = NULL;
            last_struct->next = temp_ptr;
            printf(">Job_%d was removed\n", jobID);
            wakeUP--;
            pthread_mutex_unlock(&mutex);
            return 1;
        }
    }else { // Check the rest of the jobs
        job_struct *temp_ptr = jobsLinkedList;
        while(temp_ptr->next->jobID != jobID && temp_ptr->next->next != NULL && temp_ptr->next != lastJob)
            temp_ptr = temp_ptr->next;
        if(temp_ptr->next == last_struct && last_struct->jobID == jobID){
            pthread_mutex_lock(&mutex);
            last_struct->jobID = -1;
            memset(last_struct->job, 0, COMMAND_SIZE);
            last_struct->clientSocket = -1;
            printf(">Job_%d was removed\n", jobID);
            wakeUP--;
            lastJob = temp_ptr;
            pthread_mutex_unlock(&mutex);
            return 1;
        }
        if(temp_ptr->next->next == NULL){
            return 0;
        }
        pthread_mutex_lock(&mutex);
        job_struct *temp_ptr2 = temp_ptr->next;
        temp_ptr->next = temp_ptr2->next;
        lastJob->next = temp_ptr2;
        temp_ptr2->next = NULL;

        temp_ptr2->jobID = -1;
        memset(temp_ptr2->job, 0, COMMAND_SIZE);
        temp_ptr2->clientSocket = -1;
        printf(">Job_%d was removed\n", jobID);
        wakeUP--;
        pthread_mutex_unlock(&mutex);
        return 1;
    }
}

// Function to print the list
void display() {
    job_struct *last_struct = lastJob;
    while (last_struct->next!=NULL)
        last_struct = last_struct->next;
  if (!isEmpty()){
    printf(">Jobs currently waiting are:\n");
    job_struct *temp_ptr = jobsLinkedList;
    printf(">");
    while(temp_ptr != last_struct->next){
        printf("<%d,%s> ", temp_ptr->jobID, temp_ptr->job);
        temp_ptr = temp_ptr->next;
    }
    printf(">\n");
  }
}

int serverSetup(int portNum){
    // Create a socket
        int serverSocketFD;
        if ((serverSocketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
            perror("Error opening socket");
            exit(1);
        }

        // Set the SO_REUSEADDR option
        int optval = 1;
        if (setsockopt(serverSocketFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
            perror("setsockopt");
            close(serverSocketFD);
            exit(1);
        }

        // Set up the server address structure
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port = htons(portNum);

        // Bind the socket to the address and port
        if (bind(serverSocketFD, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("Error on binding");
            close(serverSocketFD);
            exit(1);
        }

        // Listen for incoming connections
        if (listen(serverSocketFD, 5) < 0) {
            perror("Error on listening");
            close(serverSocketFD);
            exit(1);
        }
        else
            printf(">Server is listening on port %d...\n", portNum);
        return serverSocketFD;
}

int inputCheck(int argc, char **argv){
    if (argc < 4) {
        printf(">ERROR: Missing arguments!\n");
        return 1;
    }
    if (argc > 4) {
        printf(">ERROR: Too many arguments!\n");
        return 1;
    }

    if(atoi(argv[2]) <= 0){
      printf(">BufferSize must be greater than zero!\n");
      return 1;
    }
    return 0;
}

char **separateWords(char* clientMessage, int *wordCount, int jobCommanderFD){
    char **words = NULL;
    *wordCount = 0;
    char *token = strtok(clientMessage, " ");
    while (token != NULL){
        char **newWords = (char**)realloc(words, ((*wordCount) + 1) * sizeof(char *));
        if (newWords == NULL) {
            perror("realloc");
            for (int i = 0; i < *wordCount; i++) {
                free(words[i]);
            }
            free(words);
            close(jobCommanderFD);
            return NULL;
        }
        words = newWords;
        words[*wordCount] = strdup(token);
        if (words[*wordCount] == NULL) {
            perror("strdup");
            for (int i = 0; i < *wordCount; i++)
                free(words[i]);
            free(words);
            close(jobCommanderFD);
            return NULL;
        }
        (*wordCount)++;
        token = strtok(NULL, " ");
    }
    return words;
}


void *controllerThreadFunc(void *argp){
    int jobCommanderFD = *(int*)argp;
    
    // Get command from client
    char clientMessage[COMMAND_SIZE];
    memset(clientMessage, 0, COMMAND_SIZE);

    ssize_t bytesRead = 0;
    if((bytesRead = read(jobCommanderFD, clientMessage, COMMAND_SIZE - 1)) <= 0){    
        perror("Error on reading");
        close(jobCommanderFD);
        return NULL;
    }
    clientMessage[bytesRead] = '\0';


    // Separate the message into words
    int wordCount = 0;
    char **words = separateWords(clientMessage, &wordCount, jobCommanderFD);

    char serverResponse[COMMAND_SIZE];

    if(!strcmp(words[0], "issueJob")){ // Handle command: issueJob
        //Copy job received to a local variable
        job_struct new_job;
        new_job.jobID = jobID_Counter;
        new_job.clientSocket = jobCommanderFD;

        memset(new_job.job, 0, COMMAND_SIZE);
        for(int i = 1; i < wordCount; i++){
            strncat(new_job.job, words[i], COMMAND_SIZE - strlen(new_job.job) - 1);
            if(i != (wordCount - 1))
                strncat(new_job.job, " ", COMMAND_SIZE - strlen(new_job.job) - 1);
        }

        display();

        if(insertJob(new_job)){
            char start_message[34];
            snprintf(start_message, COMMAND_SIZE, "JOB <job_%d,", new_job.jobID);
            strcat(start_message, new_job.job);
            strcat(start_message, "> SUBMITTED");
            jobID_Counter++;
        }
        else {
            strcpy(serverResponse, "TOO MANY JOBS WAITING! UNABLE TO INSERT MORE");
            
        }
    }else if(!strcmp(words[0], "stop")){ // Handle command: stop
        char serverResponse[COMMAND_SIZE];
        // Extract the job ID number from the string "job_1"
        int jobID = -1;
        if(sscanf(words[1], "job_%d", &jobID) == 1){
            if(deleteJob(jobID))
                snprintf(serverResponse, COMMAND_SIZE, "JOB <%d> REMOVED", jobID);
            else
                snprintf(serverResponse, COMMAND_SIZE, "JOB <%d> NOTFOUND", jobID);
        } 
        else{
            printf(">Invalid job ID format\n");
            strcpy(serverResponse, "Invalid job ID format");
        }

    }else if(!strcmp(words[0], "setConcurrency")){ // Handle command: setConcurrency
        int new_concurrencyLevel = atoi(words[1]);

        if(new_concurrencyLevel > concurrencyLevel){
            for(int i = concurrencyLevel; i < new_concurrencyLevel; i++)
                conc_levels[i] = 1;

            concurrencyLevel = new_concurrencyLevel;
        }
        else if(new_concurrencyLevel < concurrencyLevel){
            for(int i = new_concurrencyLevel; i < concurrencyLevel; i++)
                conc_levels[i] = 0;
            concurrencyLevel = new_concurrencyLevel;
        }

        snprintf(serverResponse, COMMAND_SIZE, "CONCURRENCY SET AT %d", new_concurrencyLevel);
    }else if(!strcmp(words[0], "poll")){ // Handle command: poll
        int current_length = 0;
        if(isEmpty()){
            strcpy(serverResponse, "No jobs found!");
            send(jobCommanderFD, serverResponse, strlen(serverResponse),0);
            return NULL;
        }

        job_struct *temp_ptr = jobsLinkedList;
        int added_length = 0; 
        while (temp_ptr != NULL){
            if(temp_ptr->jobID != -1){
                added_length = snprintf(serverResponse + current_length, COMMAND_SIZE - current_length, "<%d,%s>\n ", temp_ptr->jobID, temp_ptr->job);
                current_length += added_length;
            }
            temp_ptr = temp_ptr->next;
        }

        if(current_length > 0)
            send(jobCommanderFD, serverResponse, current_length, 0);
        
        for(int i = 0; i < wordCount; i++)
            free(words[i]);
        free(words);
        return NULL;
    }
    else if(!strcmp(words[0], "exit")){ // Handle command: exit
        memset(serverResponse, 0, strlen(serverResponse));
        strcpy(serverResponse, "SERVER TERMINATED BEFORE EXECUTION");
        job_struct *temp_ptr = jobsLinkedList;
        job_struct *delete_ptr = temp_ptr; 

        while (temp_ptr != lastJob->next){
            if(send(temp_ptr->clientSocket, serverResponse, strlen(serverResponse), 0) < 0)
                perror("Error on writing");
            temp_ptr = temp_ptr->next;
            deleteJob(delete_ptr->jobID);
            delete_ptr = temp_ptr;
        }
        strcpy(serverResponse, "SERVER TERMINATED");
        if(send(jobCommanderFD, serverResponse, strlen(serverResponse), 0) < 0)
            perror("Error on writing");
        exitServer = !exitServer;
        return NULL;
    }else
        strcpy(serverResponse, "Invalid job ID format");



    if(send(jobCommanderFD, serverResponse, strlen(serverResponse), 0) < 0)
        perror("Error on writing");


    for(int i = 0; i < wordCount; i++)
        free(words[i]);
    free(words);
    
    // close(jobCommanderFD);
    return NULL;
}

void fileContent(int client_socket, const char *filename, int jobID) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("File open error");
        return ;
    }

    // Go to the end of the file to determine its size
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the file content
    char *file_content = malloc(filesize + 1);
    if (file_content == NULL) {
        perror("Memory allocation error");
        fclose(file);
        return;
    }

    // Read the file into the buffer
    fread(file_content, 1, filesize, file);
    file_content[filesize] = '\0'; 

    char start_message[34];
    snprintf(start_message, COMMAND_SIZE, "-----jobID <%d> output start-----\n", jobID);

    char end_message[32];
    snprintf(end_message, COMMAND_SIZE, "\n-----jobID <%d> output end-----", jobID);
  
    if (send(client_socket, start_message, strlen(start_message), 0) == -1)
        perror("Send error");


    if (send(client_socket, file_content, strlen(file_content), 0) == -1) {
        perror("Send error");
        fclose(file);
        return;
    }

    if (send(client_socket, end_message, strlen(end_message), 0) == -1)
        perror("Send error");

    fclose(file);
    shutdown(client_socket, SHUT_WR);
}

void *workerThreadFunc(void *argp){
    int thread_num = *(int*)argp;
        
    while(1){
        if(conc_levels[thread_num]){
            if(wakeUP && exitServer){
                printf(">Worker thread %d woke up\n", thread_num);
                int jobID = jobsLinkedList->jobID;
                
                int clientSocket = jobsLinkedList->clientSocket;

                char *job = malloc(sizeof(char) * (strlen(jobsLinkedList->job) + 1));
                strcpy(job, jobsLinkedList->job);

                int forkID = fork();
                if(forkID == 0){ // Child process
                    // Create file "pid.output"
                    char filename[64];
                    snprintf(filename, sizeof(filename), "%d.output", getpid());
                    int fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
                    if(fd < 0){
                        perror("open");
                        exit(1);
                    }

                    printf(">File %d.output was created\n", getpid());
                    // Change file's output to fd
                    if(dup2(fd, STDOUT_FILENO) < 0){
                        perror("dup2");
                        // close(fd);
                        exit(1);
                    }
                    // close(fd);
                    
                    // Prepare args for exec
                    char *args[] = {"/bin/sh", "-c", job, NULL};
                    
                    execvp(args[0], args);

                    perror("execvp");
                    exit(1);
                }
                else{ // Parent process

                    // Wait for child proccess to finish
                    int status;
                    waitpid(forkID, &status, 0);

                    // Read from pid.output
                    char filename[64];
                    snprintf(filename, sizeof(filename), "%d.output", forkID);
                    fileContent(clientSocket, filename, jobID);
                    
                    // Cleanup
                    if (remove(filename) != 0) {
                        perror("Unable to delete the file");
                        exit(1);
                    }
                    printf(">File %d.output was deleted\n", getpid());
                    close(clientSocket);
                    printf(">Worker thread %d sleeping...\n", thread_num);
                    printf("wakeUP: %d, exitServer: %d\n", wakeUP, exitServer);
                }

                free(job);
                deleteJob(jobsLinkedList->jobID);
            }
        }
    }
}