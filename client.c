/**
 * @Author: nilanjan
 * @Date:   2018-09-24T12:14:19+05:30
 * @Email:  nilanjandaw@gmail.com
 * @Filename: client.c
 * @Last modified by:   nilanjan
 * @Last modified time: 2018-09-24T18:45:42+05:30
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
#define MAX_NUM_TOKENS 4


int socket_file_descriptor = 0, port, n;

void error_handler(char *msg) {
  perror(msg);
  exit(1);
}

int connect_server(char *address, char* port_address) {
  struct sockaddr_in server_address;
  port = atoi(port_address);

  if (socket_file_descriptor > 0) {
    printf("Active connection exists. Cannot connect\n");
    return -1;
  }

  if ((socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    error_handler("unable to open socket");
  printf("Client socket initialised\n");
  bzero((char *)&server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);

  if (inet_pton(AF_INET, address, &server_address.sin_addr) <= 0)
    error_handler("unable to resolve host");
  printf("Server hostname resolved\n");
  if (connect(socket_file_descriptor, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    error_handler("unable to connect to host server");
  printf("OK\n");
  return socket_file_descriptor;
}

int disconnect_server() {
  if (socket_file_descriptor > 0) {
    write(socket_file_descriptor, "exit00", 6);
    close(socket_file_descriptor);
    socket_file_descriptor = 0;
    printf("OK\n");
    return 0;
  } else {
    printf("no active connection\n");
    return -1;
  }
}

void write_server(char *buffer) {
  if (write(socket_file_descriptor, buffer, BUFFER_LENGTH) < 0)
    error_handler("unable to write to socket");
}

char **tokenize(char *line) {
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(BUFFER_LENGTH * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t') {

      token[tokenIndex] = '\0';
      if (tokenIndex != 0) {

        tokens[tokenNo] = (char*)malloc(BUFFER_LENGTH*sizeof(char));
        strcpy(tokens[tokenNo++], token);
        tokenIndex = 0;
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }

  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}

void start_interactive() {
  char buffer[BUFFER_LENGTH];
  while (1) {
    bzero(buffer, BUFFER_LENGTH);
    fgets(buffer, BUFFER_LENGTH, stdin);
    char **token = tokenize(buffer);
    printf("%s | %s | %s\n", &(*token[0]), &(*token[1]), &(*token[2]));
    if (strcmp(&(*token[0]), "connect") == 0) {
      connect_server(&(*token[1]), &(*token[2]));
    } else if (strcmp(token[0], "disconnect") == 0) {
      disconnect_server();
    } else {
      write_server(buffer);
    }
  }
}

void start_batch(const char *path) {
  char buffer[BUFFER_LENGTH];
}

int main(int argc, char const *argv[]) {

  if (argc < 2) {
    printf("Usage: ./executable interactive/batch [path_to_batch_file]\n");
    exit(0);
  }

  if (strcmp(argv[1], "interactive") == 0) {
    start_interactive();
  } else if (strcmp(argv[1], "batch") == 0) {
    if (argc < 3) {
      printf("batch file required\n");
      exit(0);
    }
    start_batch(argv[2]);
  }
  return 0;
}
