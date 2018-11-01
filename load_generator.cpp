/**
 * @Author: nilanjan
 * @Date:   2018-09-24T12:14:19+05:30
 * @Email:  nilanjandaw@gmail.com
 * @Filename: client.c
 * @Last modified by:   nilanjan
 * @Last modified time: 2018-10-06T20:12:06+05:30
 * @Copyright: Nilanjan Daw
 */
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define MAX_NUM_TOKENS 4


int socket_file_descriptor = 0, port, n;

void error_handler(const char *msg) {
  perror(msg);
  exit(1);
}

void signal_handler(int signal) {

  if (signal == SIGINT || signal == SIGTERM || signal == SIGALRM) {
    printf("timeout\n");
  }
  exit(EXIT_SUCCESS);
}

void free_token(char **token) {

  if (token != NULL) {
    for (int i = 0; i < 4; i++) {
      if (token[i] != NULL) {
        free(token[i]);
        token[i] = NULL;
      }
    }
  }

  if (token != NULL) {
    free(token);
    token = NULL;
  }
}

long int read_input(char **memory, FILE* file, int *status) {

  char *buffer = NULL;
  size_t size = 0;
  if (getline(&buffer, &size, file) == -1) {
    if (status != NULL)
      *status = 1;
  }
  *memory = buffer;
  return strlen(buffer);
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


void generate_load(void* id) {

  printf("thread\n");
  int thread_id = *((int *)id);
  // FILE *file = fopen("", "r");
  // while (!feof(file)) {
  //   char *buffer = NULL;
  //   long int buffer_len = 0;
  //   int status = 0;
  //   buffer_len = read_input(&buffer, file, &status);
  //   printf("%s\n", buffer);
  //   if (status) {
  //     fclose(file);
  //     if (buffer != NULL) {
  //       free(buffer);
  //       buffer = NULL;
  //     }
  //     printf("Shutting Client\nDone\n");
  //     break;
  //   }
  //   char **token = tokenize(buffer, buffer_len);
  //   if (strcmp(token[0], "connect") == 0) {
  //     connect_server(token[1], token[2]);
  //   } else if (strcmp(token[0], "disconnect") == 0) {
  //     disconnect_server();
  //   } else {
  //     if (socket_file_descriptor)
  //       write_server(buffer);
  //     else
  //       printf("Error: No active TCP connection\n");
  //   }
  //   free_token(token);
  //   if (buffer != NULL) {
  //     free(buffer);
  //     buffer = NULL;
  //   }
  // }
}

int main(int argc, char const *argv[]) {

  pthread_t *consumer_threads;
  if (argc < 3) {
    printf("Error mode required\nUsage: ./executable num_threads duration\n");
    exit(0);
  }
  signal(SIGALRM, signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);

  for (int i = 1; i <= atoi(argv[1]); i++) {
    printf("Thread #%d spawned\n", i);
    int id = i;
    pthread_create(&consumer_threads[i - 1], NULL, generate_load, (void *)&id);
  }

  alarm(atoi(argv[2]));
  pause();
  // generate_load();
  return 0;
}
