/**
 * @Author: nilanjan
 * @Date:   2018-09-24T12:14:19+05:30
 * @Email:  nilanjandaw@gmail.com
 * @Filename: client.c
 * @Last modified by:   nilanjan
 * @Last modified time: 2018-09-24T16:00:44+05:30
 * @Copyright: Nilanjan Daw
 */
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_LENGTH 1024

char buffer[BUFFER_LENGTH];

void error_handler(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char const *argv[]) {

  int socket_file_descriptor, port, n;

  struct sockaddr_in server_address;

  if (argc < 3) {
    printf("IP address and port must be provided\n");
    exit(0);
  }

  port = atoi(argv[2]);
  printf("%s\n", argv[1]);
  if ((socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    error_handler("unable to open socket");
  printf("Client socket initialised\n");
  bzero((char *)&server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);

  if (inet_pton(AF_INET, argv[1], &server_address.sin_addr) <= 0)
    error_handler("unable to resolve host");
  printf("Server hostname resolved\n");
  if (connect(socket_file_descriptor, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    error_handler("unable to connect to host server");
  printf("Successfully connected to server\n");
  printf("Enter message\n");
  bzero(buffer, BUFFER_LENGTH);
  fgets(buffer, BUFFER_LENGTH, stdin);
  if (write(socket_file_descriptor, buffer, BUFFER_LENGTH) < 0)
    error_handler("unable to write to socket");
  if (socket_file_descriptor > 0) {
    printf("closing socket\n");
    close(socket_file_descriptor);
  }
  return 0;
}
