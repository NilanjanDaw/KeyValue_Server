/**
 * @Author: nilanjan
 * @Date:   2018-08-21T15:31:20+05:30
 * @Email:  nilanjandaw@gmail.com
 * @Filename: server.c
 * @Last modified by:   nilanjan
 * @Last modified time: 2018-09-24T19:20:59+05:30
 * @Copyright: Nilanjan Daw
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#define SLEEP_NANOSEC 100
#define BUFFER_LENGTH 1024
#define BACKLOG_QUEUE 5
#define MAX_NUM_TOKENS 4
#define QUEUE_SIZE 1024


int insert_index = 0, retrieve_index = 0, master_exit = 0;
int current_queue_element_count = 0;
int client_file_descriptor[QUEUE_SIZE] = {0};
int socket_file_descriptor, port;
char **hashtable;
long int table_size;

pthread_cond_t generator = PTHREAD_COND_INITIALIZER, retriever = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void error_handler(char *error_msg) {
  perror(error_msg);
  exit(1);
}

void signal_handler(int signal) {
  if (signal == SIGINT) {
    for (int i = 0; i < QUEUE_SIZE; i++) {
      if (client_file_descriptor[i] > 0) {
        close(client_file_descriptor[i]);
        client_file_descriptor[i] = 0;
        printf("Closed active client connection\n");
      }
    }
    if (socket_file_descriptor > 0) {
      printf("Closed server socket\n");
      close(socket_file_descriptor);
    }
    free(hashtable);
  }
  exit(EXIT_SUCCESS);
}


int insert(int new_client) {
  client_file_descriptor[insert_index] = new_client;
  insert_index = (++insert_index) % QUEUE_SIZE;
  current_queue_element_count++;
  return 0;
}

int retrieve(int *memory) {

  *memory = client_file_descriptor[retrieve_index];
  client_file_descriptor[retrieve_index] = 0;
  retrieve_index = (++retrieve_index) % QUEUE_SIZE;
  current_queue_element_count--;
  return 0;
}

int create(int key, char *buffer) {
  printf("%s\n", buffer);
  hashtable[key] = buffer;
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

int handle_request(int client_connection) {

  int n;
  char *buffer;
  do {
    buffer = (char *) malloc(BUFFER_LENGTH * sizeof(char));
    bzero(buffer, BUFFER_LENGTH);
    if((n = read(client_connection, buffer, BUFFER_LENGTH)) <= 0)
      error_handler("unable to read from socket");
    char **token = tokenize(buffer, strlen(buffer));
    printf("command %s buffer %s buffer_len %ld\n", token[0], token[3], strlen(token[3]));
    if (strcmp(token[0], "create") == 0) {
      long int buffer_len = strtol(token[2], NULL, 10);
      printf("asdasd\n");
      buffer = (char *)realloc(buffer, buffer_len * sizeof(char));
      printf("asdasd\n");
      strcpy(buffer, token[3]);
      printf("asdasd\n");
      int key = atoi(token[1]);
      printf("asdasd\n");
      create(key, buffer);
      printf("asdasd\n");
      printf("%s\n", hashtable[key]);
    }
    free(buffer);
  } while(strcmp(buffer, "exit00") != 0 && n != 0);
  close(client_connection);
}

//add request to queue
void *master(void *data) {

  int ret;
  int thread_id = *((int *)data);
  struct sockaddr_in address, client_address;

  printf("Master thread started with thread_id %d\n", thread_id);
  if ((socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    error_handler("unable to create socket");
  printf("Server socket initialised\n");
  bzero((char *) &address, sizeof(address));

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);
  if (bind(socket_file_descriptor, (struct sockaddr *) &address, sizeof(address)) < 0)
    error_handler("unable to bind to port");
  printf("Server socket bound to port %d\n", port);
  if (listen(socket_file_descriptor, BACKLOG_QUEUE) < 0)
    error_handler("unable to listen to given port");
  printf("Server started listening on port %d\n", port);

  while(1) {
    printf("Waiting for client connections\n");
    socklen_t cli_addr_size = sizeof(client_address);
    int new_connection = accept(socket_file_descriptor, (struct sockaddr *) &client_address,
                                    &cli_addr_size);
    if (new_connection < 0)
      error_handler("unable to accept client connection");
    printf("New client connection\n");
    pthread_mutex_lock(&mutex);
    while (current_queue_element_count == QUEUE_SIZE) {
      printf("master waiting - connection buffer full\n" );
      pthread_cond_wait(&generator, &mutex);
    }
    int result = insert(new_connection);

    pthread_cond_signal(&retriever);
    pthread_mutex_unlock(&mutex);
  }
}

// read request from queue
void* service_request(void* id) {
  int thread_id = *((int *)id);
  while (1) {
    pthread_mutex_lock(&mutex);
    while (current_queue_element_count <= 0) {
      printf("Buffer empty. Thread %d sleeping.\n", thread_id);
      pthread_cond_wait(&retriever, &mutex);
    }

    int client_connection;
    int status = retrieve(&client_connection);
    pthread_cond_signal(&generator);
    pthread_mutex_unlock(&mutex);

    handle_request(client_connection);
  }
}

int main(int argc, char *argv[]) {

  int prod_thread_id = 0, worker_thread_count;
  printf("This system has %d processors configured and "
        "%d processors available.\n",
        get_nprocs_conf(), get_nprocs());
  worker_thread_count = 2 * get_nprocs();
  table_size = (long int)pow(2, sizeof(int) * 8);
  hashtable = malloc(table_size * sizeof(char *));

  pthread_t prod_thread;
  pthread_t *consumer_threads;

  signal(SIGINT, signal_handler);
  if (argc < 2)
    error_handler("Port number must be provided");
  port = atoi(argv[1]);
  //create master thread
  pthread_create(&prod_thread, NULL, master, (void *)&prod_thread_id);
  consumer_threads = (pthread_t*) malloc(worker_thread_count * sizeof(pthread_t));
  for (int i = 1; i <= worker_thread_count; i++) {
    printf("Thread #%d spawned\n", i);
    int id = i;
    pthread_create(&consumer_threads[i - 1], NULL, service_request, (void *)&id);
  }

  sleep(1000);
  return 0;
}
