/**
 * @Author: nilanjan
 * @Date:   2018-08-21T15:31:20+05:30
 * @Email:  nilanjandaw@gmail.com
 * @Filename: 183059004.c
 * @Last modified by:   nilanjan
 * @Last modified time: 2018-09-02T17:42:12+05:30
 * @Copyright: Nilanjan Daw
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>
#include <unistd.h>
#define SLEEP_NANOSEC 100

#ifndef QUEUE_SIZE
  #define QUEUE_SIZE 1024
#endif

#ifndef WORKER_THREAD_COUNT
  #define WORKER_THREAD_COUNT 4
#endif

#ifndef MAX_REQUEST
  #define MAX_REQUEST 10000
#endif

int number_to_produce, insert_index = 0, retrieve_index = 0, master_exit = 0;
int current_queue_element_count = 0;
int buffer[QUEUE_SIZE];

pthread_cond_t generator, retriever;
pthread_mutex_t mutex;

int insert(int number_to_produce) {
  buffer[insert_index] = number_to_produce;
  insert_index = (++insert_index) % QUEUE_SIZE;
  current_queue_element_count++;
  return 0;
}

int retrieve(int *memory) {

  *memory = buffer[retrieve_index];
  retrieve_index = (++retrieve_index) % QUEUE_SIZE;
  current_queue_element_count--;
  return 0;
}

//add request to queue
void *generate_requests_loop(void *data) {

  int ret;
  int thread_id = *((int *)data);
  while(1) {
    pthread_mutex_lock(&mutex);
    while (current_queue_element_count == QUEUE_SIZE) {
      printf("master waiting\n" );
      pthread_cond_wait(&generator, &mutex);
    }
    int result = insert(number_to_produce);
    printf("Produced item %d\n", number_to_produce);
    number_to_produce++;

    if (number_to_produce >= MAX_REQUEST) {
      master_exit = 1;
      printf("Master exit\n");
    }

    pthread_cond_signal(&retriever);
    pthread_mutex_unlock(&mutex);
    //optional sleep to allow workers to run

    if (number_to_produce >= MAX_REQUEST) {
      break;
    }

    struct timespec delay;
    delay.tv_sec = 0;
    delay.tv_nsec = SLEEP_NANOSEC;
    nanosleep(&delay, NULL);
  }

}

// read request from queue
void* consume_request(void* id) {
  int thread_id = *((int *)id);

  while (1) {
    pthread_mutex_lock(&mutex);
    while (current_queue_element_count <= 0) {
      printf("Buffer empty. Thread %d sleeping.\n", thread_id);
      pthread_cond_wait(&retriever, &mutex);
    }

    int data;
    int status = retrieve(&data);
    printf("%d printed by thread: %d\n", data, thread_id);

    if (current_queue_element_count <= 0 && master_exit == 1) {
      printf("Workers exit\n");
      exit(0);
    }


    pthread_cond_signal(&generator);
    pthread_mutex_unlock(&mutex);

    struct timespec delay;
    delay.tv_sec = 0;
    delay.tv_nsec = SLEEP_NANOSEC;
    nanosleep(&delay, NULL);
  }
}

int main(int argc, char *argv[]) {

  int prod_thread_id = 0, t1 = 1, t2 = 2, t3 = 3, t4 = 4;
  pthread_t prod_thread;
  pthread_t consumer_threads[WORKER_THREAD_COUNT];

  number_to_produce = 0;

  //create master thread
  pthread_create(&prod_thread, NULL, generate_requests_loop, (void *)&prod_thread_id);

  //create worker threads
  // for (int i = 1; i <= WORKER_THREAD_COUNT; i++) {
  //   printf("Thread #%d spawned\n", i);
  //   int id = i;
  //   pthread_create(&consumer_threads[i - 1], NULL, consume_request, (void *)&id);
  // }
  printf("Thread #%d spawned\n", t1);
  pthread_create(&consumer_threads[0], NULL, consume_request, (void *)&t1);
  printf("Thread #%d spawned\n", t2);
  pthread_create(&consumer_threads[1], NULL, consume_request, (void *)&t2);
  printf("Thread #%d spawned\n", t3);
  pthread_create(&consumer_threads[2], NULL, consume_request, (void *)&t3);
  printf("Thread #%d spawned\n", t4);
  pthread_create(&consumer_threads[3], NULL, consume_request, (void *)&t4);

  sleep(1000);

  return 0;
}
