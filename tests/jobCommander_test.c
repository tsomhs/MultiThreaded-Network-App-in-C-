#include "include/clientFunctions.h"

#define ADDRESS "linux12.di.uoa.gr" //CHANGE IF NEEDED
#define PORT 2040

int main(int argc, char **argv){


    char *serverName = malloc(sizeof(char) * strlen(argv[2]));
    strcpy(serverName, ADDRESS); 

    int portNum = PORT;

    char **JCinput = malloc(sizeof(char*)*5);
    JCinput[0] = "issueJob cat Makefile";
    JCinput[1] = "issueJob ls -la";
    JCinput[2] = "poll";
    JCinput[3] = "stop job_2"; 
    JCinput[4] = "setConcurrency 4";

    // Set up the server address structure 
    struct sockaddr_in serv_addr;
    serverAddrSetup(&serv_addr, serverName, portNum);

    // Create the socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
    perror("Error on connecting");
    exit(1);
    }
    printf(">Connected to %s on port %d\n", serverName, portNum);

    clientSend(JCinput[0], sockfd, serverName);
    return 0;
}