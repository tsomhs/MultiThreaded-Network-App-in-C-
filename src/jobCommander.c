#include "include/clientFunctions.h"

int main(int argc, char **argv){
    if(argc < 3){
    printf(">ERROR: Missing arguments!\n");
    return 1;
    } 

    // Extract input to variables
    // Extract server's name
    char *serverName = malloc(sizeof(char) * strlen(argv[2]));
    strcpy(serverName, argv[1]);

    // Extract port
    int portNum = atoi(argv[2]); 

    // Extract input command
    int JCinputLenght = 0;
    for (int i = 3; i < argc; i++)  {
    JCinputLenght += strlen(argv[i]);
    }
    char *JCinput = malloc(sizeof(char) * JCinputLenght);
    for(int i = 3; i < argc; i++){
    strcat(JCinput, argv[i]);
    if (i < argc - 1)
        strcat(JCinput, " ");
    }

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
    clientSend(JCinput, sockfd, serverName);
    return 0;
}