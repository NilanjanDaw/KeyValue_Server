/**
 * @Author: nilanjan
 * @Date:   2018-09-24T00:04:11+05:30
 * @Email:  nilanjandaw@gmail.com
 * @Filename: server_single.c
 * @Last modified by:   nilanjan
 * @Last modified time: 2018-09-24T00:57:21+05:30
 * @Copyright: Nilanjan Daw
 */



#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#ifndef BACKLOG_QUEUE
  #define BACKLOG_QUEUE 5
#endif

int port;
void error_handler(char *error_msg) {
  perror(error_msg);
  exit(1);
}

int main(int argc, char const *argv[]) {
  struct sockaddr_in address;
  int socket_file_descriptor;

  if (argc < 2)
    error_handler("IP Address and port number must be provided");

  if ((socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    error_handler("unable to create socket");

  bzero((char *) &address, sizeof(address));
  port = atoi(argv[1]);

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);
  if (bind(socket_file_descriptor, (struct sockaddr *) &address, sizeof(address)) < 0)
    error_handler("unable to bind to port");
  if (listen(socket_file_descriptor, BACKLOG_QUEUE) < 0)
    error_handler("unable to listen to given port");
  
  return 0;
}
