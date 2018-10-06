/**
 * @Author: nilanjan
 * @Date:   2018-09-24T12:14:19+05:30
 * @Email:  nilanjandaw@gmail.com
 * @Filename: client.c
 * @Last modified by:   nilanjan
 * @Last modified time: 2018-10-06T18:10:54+05:30
 * @Copyright: Nilanjan Daw
 */
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define MAX_NUM_TOKENS 4


int socket_file_descriptor = 0, port, n;

void error_handler(const char *msg) {
  perror(msg);
  exit(1);
}

long int read_input(char **memory, FILE* file, int *status) {

  // char buffer[BUFFER_LENGTH];
  // bzero(buffer, BUFFER_LENGTH);
  // char* result = fgets(buffer, BUFFER_LENGTH, file);
  // if (result == NULL && status != NULL)
  //   *status = 1;
  // buffer[strlen(buffer)] = ' ';
  // *memory = buffer;

  char *buffer = NULL;
  size_t size = 0;
  if (getline(&buffer, &size, file) == -1) {
    if (status != NULL)
      *status = 1;
  }
  *memory = buffer;
  return strlen(buffer);
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

int send_header(int length) {
  char header[11];
  int current_read = 0;
  sprintf(header, "%10d", length);
  if ((current_read = write(socket_file_descriptor, header, 11)) < 0) {
    error_handler("unable to read from socket");
  }
  return 0;
}

int disconnect_server() {
  if (socket_file_descriptor > 0) {
    send_header(strlen("exit00"));
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

  int response_read = 0;
  char *read_reply;
  send_header(strlen(buffer));
  if (write(socket_file_descriptor, buffer, strlen(buffer)) < 0)
    error_handler("unable to write to socket");
  char header[11];

  if ((response_read = read(socket_file_descriptor, header, 11)) < 0) {
    error_handler("unable to read from socket");
  }
  int packet_length = atoi(header) + 1;
  read_reply = (char *) malloc(packet_length * sizeof(char));
  bzero(read_reply, packet_length);

  if((response_read = read(socket_file_descriptor, read_reply, packet_length)) < 0)
    error_handler("unable to read from socket");
  else if (response_read > 0) {
    printf("Server Reply: %s\n", read_reply);
  }
  free(read_reply);
  read_reply = NULL;
}

char **tokenize(char *line, long int buffer_length) {
  char **tokens = (char **)malloc((MAX_NUM_TOKENS + 1) * sizeof(char *));
  char *token = (char *)malloc(buffer_length * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i = 0; i < strlen(line); i++){

    char readChar = line[i];

    if ((tokenNo < MAX_NUM_TOKENS - 1) && (readChar == ' ' || readChar == '\n' || readChar == '\t')) {
      token[tokenIndex] = '\0';
      if (tokenIndex != 0) {

        tokens[tokenNo] = (char*)malloc(buffer_length * sizeof(char));
        strcpy(tokens[tokenNo++], token);
        tokenIndex = 0;
        bzero(token, buffer_length);
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
  tokens[tokenNo] = (char*)malloc(buffer_length * sizeof(char));
  strcpy(tokens[tokenNo++], token);

  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}

void start_interactive() {
  while (1) {
    char *buffer = NULL;
    long int buffer_len = 0;
    buffer_len = read_input(&buffer, stdin, NULL);
    char **token = tokenize(buffer, strlen(buffer));
    // printf("%s | %s | %s | %s\n", token[0], token[1], token[2], token[3]);
    if (strcmp(token[0], "connect") == 0) {
      connect_server(token[1], token[2]);
    } else if (strcmp(token[0], "disconnect") == 0) {
      disconnect_server();
    } else {
      write_server(buffer);
    }
    for (size_t i = 0; i < MAX_NUM_TOKENS; i++) {
      if (token[i] != NULL)
        free(token[i]);
      token[i] = NULL;
    }
    if (token != NULL) {
      free(token);
      token = NULL;
    }
    if (buffer != NULL) {
      free(buffer);
      buffer = NULL;
    }
  }
}

void start_batch(const char *path) {

  FILE *file = fopen(path, "r");
  while (!feof(file)) {
    char *buffer = NULL;
    long int buffer_len = 0;
    int status = 0;
    buffer_len = read_input(&buffer, file, &status);
    printf("%s\n", buffer);
    if (status) {
      fclose(file);
      printf("Shutting Client\nDone\n");
      break;
    }
    char **token = tokenize(buffer, buffer_len);
    // printf("%s | %s | %s | %s\n", token[0], token[1], token[2], token[3]);
    if (strcmp(token[0], "connect") == 0) {
      connect_server(token[1], token[2]);
    } else if (strcmp(token[0], "disconnect") == 0) {
      disconnect_server();
    } else {
      write_server(buffer);
    }
    for (size_t i = 0; i < MAX_NUM_TOKENS; i++) {
      if (token[i] != NULL)
        free(token[i]);
      token[i] = NULL;
    }
    if (token != NULL) {
      free(token);
      token = NULL;
    }
    if (buffer != NULL) {
      free(buffer);
      buffer = NULL;
    }
  }
}

int main(int argc, char const *argv[]) {

  if (argc < 2) {
    printf("Error mode required\nUsage: ./executable interactive/batch [path_to_batch_file]\n");
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
