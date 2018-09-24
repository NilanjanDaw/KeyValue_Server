/**
 * @Author: nilanjan
 * @Date:   2018-09-24T00:04:11+05:30
 * @Email:  nilanjandaw@gmail.com
 * @Filename: server_single.c
 * @Last modified by:   nilanjan
 * @Last modified time: 2018-09-24T15:37:16+05:30
 * @Copyright: Nilanjan Daw
 */



#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#define BACKLOG_QUEUE 5
#define BUFFER_LENGTH 1024


int port;
int socket_file_descriptor = 0, client_file_descriptor = 0;
char buffer[BUFFER_LENGTH];

void error_handler(char *error_msg) {
  perror(error_msg);
  exit(1);
}

void signal_handler(int signal) {
  if (signal == SIGINT) {
    if (client_file_descriptor > 0) {
      close(client_file_descriptor);
      printf("Closed active client connection\n");
    }
    if (socket_file_descriptor > 0) {
      printf("Closed server socket\n");
      close(socket_file_descriptor);
    }
  }
  exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[]) {
  struct sockaddr_in address, client_address;
  int n;

  signal(SIGINT, signal_handler);
  if (argc < 2)
    error_handler("Port number must be provided");

  if ((socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    error_handler("unable to create socket");
  printf("Server socket initialised\n");
  bzero((char *) &address, sizeof(address));
  port = atoi(argv[1]);

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);
  if (bind(socket_file_descriptor, (struct sockaddr *) &address, sizeof(address)) < 0)
    error_handler("unable to bind to port");
  printf("Server socket bound to port %d\n", port);
  if (listen(socket_file_descriptor, BACKLOG_QUEUE) < 0)
    error_handler("unable to listen to given port");
  printf("Server started listening on port %d\n", port);
  while (1) {
    printf("Waiting for client connections\n");
    socklen_t cli_addr_size = sizeof(client_address);
    client_file_descriptor = accept(socket_file_descriptor, (struct sockaddr *) &client_address,
                                    &cli_addr_size);
    if (client_file_descriptor < 0)
      error_handler("unable to accept client connection");
    printf("New client connection\n");
    bzero(buffer, BUFFER_LENGTH);
    if((n = read(client_file_descriptor, buffer, BUFFER_LENGTH)) < 0)
      error_handler("unable to read from socket");
    printf("%s\n", buffer);
  }
  return 0;
}
