/**
 * @Author: nilanjan
 * @Date:   2018-09-24T12:14:19+05:30
 * @Email:  nilanjandaw@gmail.com
 * @Filename: client.c
 * @Last modified by:   nilanjan
 * @Last modified time: 2018-11-06T01:25:07+05:30
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
#define MAX_KEY 10000
#define SEED 99

int disconnect_server(int *socket_file_descriptor);

int port_address, n;
char *ip_address;
int EXIT_FLAG = 0;

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
    // printf("disconnect OK\n");
    return 0;
  } else {
    printf("no active connection\n");
    return -1;
  }
}


void error_handler(const char *msg, int *socket_file_descriptor) {
  perror(msg);
  if (*socket_file_descriptor != 0) {
    close(*socket_file_descriptor);
    *socket_file_descriptor = 0;
  }
  // exit(1);
}


long int read_input(char **memory, int *status) {
  const char* operators[6] = {"create ", "read ", "update ", "delete "};
  char *buffer = NULL;
  size_t size = 0;
  int index = rand() % 4;
  // index = 1;
  // printf("%s\n", operators[index]);
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
    // printf("%s %ld\n", message, strlen(message));
  }
  *memory = (char*)malloc((strlen(message) + 1) * sizeof(char));
  strcpy(*memory, message);
  return strlen(message);
  // return strlen(buffer);
}

int connect_server(char *address, int port, int *socket_file_descriptor) {
  struct sockaddr_in server_address;

  if (*socket_file_descriptor > 0) {
    printf("Active connection exists. Cannot connect\n");
    return -1;
  }

  if ((*socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    error_handler("unable to open socket", socket_file_descriptor);
  // printf("Client socket initialised\n");
  bzero((char *)&server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);

  if (inet_pton(AF_INET, address, &server_address.sin_addr) <= 0)
    error_handler("unable to resolve host", socket_file_descriptor);
  // printf("Server hostname resolved\n");
  if (connect(*socket_file_descriptor, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    error_handler("unable to connect to host server", socket_file_descriptor);
  // printf("OK\n");
  return *socket_file_descriptor;
}

int write_server(char *buffer, int *socket_file_descriptor) {

  int response_read = 0;
  char *read_reply;
  // printf("%ld\n", strlen(buffer));
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
    // printf("Server Reply: %s\n", read_reply);
  }
  free(read_reply);
  read_reply = NULL;
  return response_read;
}


void* generate_load(void* id) {

  // printf("thread\n");
  int thread_id = *((int *)id);
  int connection_status = 0, socket_file_descriptor = 0;
  while (1) {
    char *buffer = NULL;
    long int buffer_len = 0;
    int status = 0;
    // printf("EXIT_FLAG %d\n", EXIT_FLAG);

    if (EXIT_FLAG == 1) {
      // printf("timed out\n");
      disconnect_server(&socket_file_descriptor);
      break;
    }

    if (socket_file_descriptor == 0)
      connect_server(ip_address, port_address, &socket_file_descriptor);
    else if (connection_status > 9600) {
      disconnect_server(&socket_file_descriptor);
      connection_status = 0;
      continue;
    } else {
      buffer_len = read_input(&buffer, &status);
      if (socket_file_descriptor && buffer_len != 0) {
        int response_read = 0;
        for (size_t i = 0; i < 10; i++) {
          response_read = write_server(buffer, &socket_file_descriptor);
          if (response_read < 0) {
            disconnect_server(&socket_file_descriptor);
            connection_status = 0;
            socket_file_descriptor = 0;
            break;
          }
        }
      }
      else {
        printf("Error: No active TCP connection\n");
        connect_server(ip_address, port_address, &socket_file_descriptor);
      }
      if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
      }

    }
    connection_status = rand() % MAX_KEY;
    // if (status) {
    //   if (buffer != NULL) {
    //     free(buffer);
    //     buffer = NULL;
    //   }
    //   printf("Shutting Client\nDone\n");
    //   break;
    // }

    // break;
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
  return 0;
}
