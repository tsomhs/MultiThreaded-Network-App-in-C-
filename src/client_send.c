#include "include/sharedDefinitions.h"

int clientSend(char* JCinput, int sockfd, char* serverName){
    // Send the message to the server
    send(sockfd, JCinput, strlen(JCinput), 0);
    printf(">Message sent: %s\n", JCinput);


    char read_buffer[COMMAND_SIZE];
    int bytes_read = read(sockfd, read_buffer, COMMAND_SIZE - 1);
    if (bytes_read < 0) {
        perror("Error reading from socket");
        close(sockfd);
        free(serverName);
        exit(1); 
    }
    read_buffer[bytes_read] = '\0';
    printf(">Server responded: %s\n", read_buffer);

    char issueJob[] = "issueJob";
    int flag = 1;
    char jobOutput[FILE_OUTPUT_SIZE] = {0};
    for(int i=0; i < strlen(issueJob); i++){
        if(JCinput[i]  != issueJob[i])
            flag = 0;
    }
    if (flag) { 
        int bytes_received;
        while ((bytes_received = recv(sockfd, jobOutput, FILE_OUTPUT_SIZE - 1, 0)) > 0) {
            jobOutput[bytes_received] = '\0';
            if (strcmp(jobOutput, "output end-----") == 0) {
                break;
            }
        }

        printf(">Server responded: %s\n", jobOutput);

        if (bytes_received < 0)
            perror("Receive error");
    } 

    free(serverName);

    // Close the socket
    close(sockfd);
    return 0;
}