/******************************************************************************
 * DESCRIPTION:
 * Example simulating a process with two producer/consumer threads. A producer
 * takes a random time to produce the data. Once the data is produced, it is 
 * written into an exchange buffer, BUT only if the buffer is empty.
 *
 * In addition to the exchange buffer, we assume that the consumer and producer
 * have their own private buffers.
 *
 * producer      exchange    consumer
 * prod_buff ---> buffer --> cons_buff
 *
 * The exchange buffer is protected with a mutex.
 *
 * The consumer empties the exchange buffer (only if it is full) when it 
 * copies the data into its own buffer, and begins to process it. The
 * processing takes a random amount of time.
 *
 * The producer is the only one adding data to the buffer, and the consumer the
 * only one removing from it. The buffer can be only 1 (buffer full) or 0
 * (empty). Two condition variables are used to send signals.
 *
 * If the producer finds that the buffer is currently full (meaning that the
 * consumer didn't have yet time to read it, then it waits for signal
 * buffer_empty_cv, which has to be sent by the consumer once he has read the 
 * buffer. When the buffer is empty, the producer fills it and sends a 
 * buffer_full_cv.
 *
 * If the consumer finds the buffer empty, it sends the signal buffer_empty_cv
 * and waits for signal buffer_full_cv.
 ******************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TCOUNT 10

int buffer = 0;
pthread_mutex_t buffer_mutex;
pthread_cond_t buffer_full_cv;
pthread_cond_t buffer_empty_cv;

void *produce_buffer(void *t) 
{
	for (int i=0; i < TCOUNT; i++) {

		printf("prod(%d): start\n", i);

		/* Produce some data */
		int prod_time = rand() % 5 + 1;
		for (int t = 0; t < prod_time; ++t)
		{
			printf("\tprod(%d): working %d / %ds\n", i, t+1, prod_time);
			sleep(1);
		}

		/* Data ready: if consumer is ready, write to buffer */
		pthread_mutex_lock(&buffer_mutex);
		printf("\tprod(%d): buffer mutex locked\n", i);

		while (buffer > 0) {
			printf("\tprod(%d): buffer = %d, going into wait\n", i, buffer);
			pthread_cond_wait(&buffer_empty_cv, &buffer_mutex);
			printf("\tprod(%d): buffer_empty_cv received: buffer = %d\n", i, buffer);
		}
		buffer++;
		pthread_cond_signal(&buffer_full_cv);
		printf("\tprod(%d): buffer set to %d, buffer_full_cv sent\n", i, buffer);

		printf("\tprod(%d): unlocking mutex\n", i);
		pthread_mutex_unlock(&buffer_mutex);
	}
	pthread_exit(NULL);
}

void *consume_buffer(void *t) 
{
	for (int i = 0; i < TCOUNT; ++i) {

		printf("cons(%d): start\n", i);

		pthread_mutex_lock(&buffer_mutex);
		printf("\tcons(%d): buffer mutex locked\n", i);

		if (buffer == 0) {
			pthread_cond_signal(&buffer_empty_cv);
			printf("\tcons(%d): buffer_empty_cv sent\n", i);

			while (buffer == 0) {
				printf("\tcons(%d): going into wait\n", i);
				pthread_cond_wait(&buffer_full_cv, &buffer_mutex);
				printf("\tcons(%d): buffer_full_cv received: buffer = %d\n", i, buffer);
			}
		}

		buffer--;
		printf("\tcons(%d): buffer set to %d\n", i, buffer);
		pthread_cond_signal(&buffer_empty_cv);
		printf("\tcons(%d): buffer_empty_cv sent - unloking mutex\n", i);
		pthread_mutex_unlock(&buffer_mutex);

		/* Process the read data */
		int cons_time = rand() % 5 + 1;
		for (int t = 0; t < cons_time; ++t)
		{
			printf("\tcons(%d): working %d / %ds\n", i, t+1, cons_time);
			sleep(1);
		}
	}
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	pthread_t threads[2];
	pthread_attr_t attr;

	/* Initialize mutex and condition variable objects */
	pthread_mutex_init(&buffer_mutex, NULL);
	pthread_cond_init (&buffer_full_cv, NULL);
	pthread_cond_init (&buffer_empty_cv, NULL);

	/* For portability, explicitly create threads in a joinable state */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	/* Tasks with random running time */
	time_t t;
	srand((unsigned) time(&t));

	/* Launch threads; pthread_cond_wait must be called first */
	pthread_create(&threads[0], &attr, produce_buffer, NULL);
	pthread_create(&threads[1], &attr, consume_buffer, NULL);

	/* Wait for all threads to complete */
	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);

	printf ("Main(): Waited and joined with 2 threads. "
			"Final value of buffer = %d. Done.\n", buffer);

	/* Clean up and exit */
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&buffer_mutex);
	pthread_cond_destroy(&buffer_full_cv);
	pthread_cond_destroy(&buffer_empty_cv);
	pthread_exit (NULL);
}
