#include "clientDefinitions.h"

void serverAddrSetup(struct sockaddr_in *serv_addr, char *serverName, int portNum){
  memset(&(*serv_addr), 0, sizeof(*serv_addr));
  (*serv_addr).sin_family = AF_INET;
  memcpy(&(*serv_addr).sin_addr.s_addr, gethostbyname(serverName)->h_addr_list[0], gethostbyname(serverName)->h_length);
  (*serv_addr).sin_port = htons(portNum);
}