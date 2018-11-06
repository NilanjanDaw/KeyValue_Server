/**
 * @Author: nilanjan
 * @Date:   2018-09-24T12:14:19+05:30
 * @Email:  nilanjandaw@gmail.com
 * @Filename: client.c
 * @Last modified by:   nilanjan
 * @Last modified time: 2018-11-06T19:47:56+05:30
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
#include <math.h>
#include <sys/time.h>

#define MAX_NUM_TOKENS 4
#define MAX_KEY 10000
#define SEED 99

int disconnect_server(int *socket_file_descriptor);

int port_address, n;
char *ip_address;
int EXIT_FLAG = 0;

unsigned long long total_requests = 0, total_successful = 0, total_execution_time = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void signal_handler(int signal) {

  if (signal == SIGINT || signal == SIGTERM) {
    printf("timeout\n");
    EXIT_FLAG = 1;
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


int send_header(int length,int *socket_file_descriptor) {
  char header[11];
  int current_read = 0;
  sprintf(header, "%10d", length);
  if ((current_read = write(*socket_file_descriptor, header, 11)) < 0) {
    perror("unable to read from socket");
    disconnect_server(socket_file_descriptor);
  }
  return 0;
}

int disconnect_server(int *socket_file_descriptor) {
  if (*socket_file_descriptor > 0) {
    send_header(strlen("exit00"), socket_file_descriptor);
    write(*socket_file_descriptor, "exit00", 6);
    close(*socket_file_descriptor);
    *socket_file_descriptor = 0;
    return 0;
  } else {
    return -1;
  }
}


void error_handler(const char *msg, int *socket_file_descriptor) {
  if (*socket_file_descriptor != 0) {
    close(*socket_file_descriptor);
    *socket_file_descriptor = 0;
  }
}


long int read_input(char **memory, int *status) {
  const char* operators[6] = {"create ", "read ", "update ", "delete "};
  char *buffer = NULL;
  size_t size = 0;
  int index = rand() % 4;
  *status = index;
  char key_string[5];
  char message[5000] = {0};
  int key = rand() % MAX_KEY;
  sprintf(key_string, "%d", key);
  strcat(message, operators[index]);
  strcat(message, key_string);
  if (index == 0 || index == 2) {
    char value[] = " 1001 askhdasdasbdhagdasyidadbasjdabsdajbdassd sdsdahsdajsdhasdhkasdansd,as ashdgayewehbaskhdasdasbdhagdasyidadbasjdabsdajbdassd sdsdahsdajsdhasdhkasdansd,as ashdgayewehbaskhdasdasbdhagdasyidadbasjdabsdajbdassd sdsdahsdajsdhasdhkasdansd,as ashdgayewehbaskhdasdasbdhagdasyidadbasjdabsdajbdassd sdsdahsdajsdhasdhkasdansd,as ashdgayewehbaskhdasdasbdhagdasyidadbasjdabsdajbdassd sdsdahsdajsdhasdhkasdansd,as ashdgayewehbaskhdasdasbdhagdasyidadbasjdabsdajbdassd sdsdahsdajsdhasdhkasdansd,as ashdgayewehbaskhdasdasbdhagdasyidadbasjdabsdajbdassd sdsdahsdajsdhasdhkasdansd,as ashdgayewehbaskhdasdasbdhagdasyidadbasjdabsdajbdassd sdsdahsdajsdhasdhkasdansd,as ashdgayewehbaskhdasdasbdhagdasyidadbasjdabsdajbdassd sdsdahsdajsdhasdhkasdansd,as ashdgayewehbaskhdasdasbdhagdasyidadbasjdabsdajbdassd sdsdahsdajsdhasdhkasdansd,as ashdgayewehbaskhdasdasbdhagdasyidadbasjdabsdajbdassd sdsdahsdajsdhasdhkasdansd,as ashdgayewehbaskhdasdasbdhagdasyidadbasjdabsdajbdassd sdsdahsdajsdhasdhkasdansd,as ashdgayewehbaskhdasdasbdhagd";
    strcat(message, value);
  }
  *memory = (char*)malloc((strlen(message) + 1) * sizeof(char));
  strcpy(*memory, message);
  return strlen(message);
}

int connect_server(char *address, int port, int *socket_file_descriptor) {
  struct sockaddr_in server_address;

  if (*socket_file_descriptor > 0) {
    return -1;
  }

  if ((*socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    error_handler("unable to open socket", socket_file_descriptor);
  bzero((char *)&server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);

  if (inet_pton(AF_INET, address, &server_address.sin_addr) <= 0)
    error_handler("unable to resolve host", socket_file_descriptor);
  if (connect(*socket_file_descriptor, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    error_handler("unable to connect to host server", socket_file_descriptor);
  return *socket_file_descriptor;
}

int write_server(char *buffer, int *socket_file_descriptor) {

  int response_read = 0;
  char *read_reply;
  send_header(strlen(buffer), socket_file_descriptor);
  if (write(*socket_file_descriptor, buffer, strlen(buffer)) < 0)
    error_handler("unable to write to socket", socket_file_descriptor);
  char header[11];

  if ((response_read = read(*socket_file_descriptor, header, 11)) < 0) {
    error_handler("unable to read from socket 153", socket_file_descriptor);
    return response_read;
  }
  int packet_length = atoi(header) + 1;
  read_reply = (char *) malloc(packet_length * sizeof(char));
  bzero(read_reply, packet_length);

  if((response_read = read(*socket_file_descriptor, read_reply, packet_length)) < 0) {
      error_handler("unable to read from socket 160", socket_file_descriptor);
  }
  else if (response_read > 0) {
  }
  free(read_reply);
  read_reply = NULL;
  return response_read;
}


void* generate_load(void* id) {

  unsigned long long per_thread_total_requests = 0, per_thread_total_successful = 0, per_thread_total_execution_time = 0;

  // printf("thread\n");
  int thread_id = *((int *)id);
  int connection_status = 0, socket_file_descriptor = 0;
  while (1) {
    char *buffer = NULL;
    long int buffer_len = 0;
    int status = 0;

    if (EXIT_FLAG == 1) {
      disconnect_server(&socket_file_descriptor);
      pthread_mutex_lock(&mutex);
      total_requests += per_thread_total_requests;
      total_successful += per_thread_total_successful;
      total_execution_time += per_thread_total_execution_time;
      // printf("Thread %d total request: %llu total_successful: %llu\n", thread_id, per_thread_total_requests, per_thread_total_successful);
      pthread_mutex_unlock(&mutex);
      break;
    }
    per_thread_total_requests++;
    if (socket_file_descriptor == 0) {
      struct timespec start, end;
      clock_gettime(CLOCK_MONOTONIC_RAW, &start);
      int status = connect_server(ip_address, port_address, &socket_file_descriptor);
      if (status > 0) {
        per_thread_total_successful++;
      }
      clock_gettime(CLOCK_MONOTONIC_RAW, &end);
      per_thread_total_execution_time += round(((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000) / 1000);

    } else if (connection_status > 9600) {
      struct timespec start, end;
      clock_gettime(CLOCK_MONOTONIC_RAW, &start);
      int status = disconnect_server(&socket_file_descriptor);
      if (status >= 0) {
        per_thread_total_successful++;
      }
      clock_gettime(CLOCK_MONOTONIC_RAW, &end);
      per_thread_total_execution_time += round(((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000) / 1000);
      connection_status = 0;
      continue;

    } else {
      buffer_len = read_input(&buffer, &status);
      if (socket_file_descriptor && buffer_len != 0) {
        int response_read = 0;
        per_thread_total_requests--;
        for (size_t i = 0; i < 10; i++) {
          per_thread_total_requests++;
          struct timespec start, end;
          clock_gettime(CLOCK_MONOTONIC_RAW, &start);
          response_read = write_server(buffer, &socket_file_descriptor);

          if (response_read < 0) {
            disconnect_server(&socket_file_descriptor);
            connection_status = 0;
            socket_file_descriptor = 0;
            break;
          } else {
            per_thread_total_successful++;
          }

          clock_gettime(CLOCK_MONOTONIC_RAW, &end);
          per_thread_total_execution_time += round(((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000) / 1000);
        }
      }
      else {
        // printf("Error: No active TCP connection\n");
        connect_server(ip_address, port_address, &socket_file_descriptor);
      }

      if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
      }

    }
    connection_status = rand() % MAX_KEY;
  }
}

int main(int argc, char const *argv[]) {

  pthread_t *consumer_threads;
  if (argc < 3) {
    printf("Error mode required\nUsage: ./executable server_ip server_port num_threads duration\n");
    exit(0);
  }

  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);
  ip_address = (char *)argv[1];
  port_address = atoi(argv[2]);
  int num_threads = atoi(argv[3]);
  int runtime = atoi(argv[4]);
  srand(SEED);
  consumer_threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));
  for (int i = 1; i <= num_threads; i++) {
    // printf("Thread #%d spawned\n", i);
    int id = i;
    pthread_create(&consumer_threads[i - 1], NULL, generate_load, (void *)&id);
  }

  sleep(runtime);
  EXIT_FLAG = 1;
  printf("done\n");
  for (int i = 1; i <= num_threads; i++) {
    pthread_join(consumer_threads[i - 1], NULL);
  }

  printf("total request: %llu total_successful: %llu\n", total_requests, total_successful);
  float mean_response_time = (float)total_execution_time / (float) total_requests;
  float mean_throughput = (float) total_successful / (float) runtime;
  printf("mean response time: %f mean throughput %f\n", mean_response_time, mean_throughput);
  return 0;
}
