/******************************************************************************
 * DESCRIPTION:
 * Example simulating a process with two producer/consumer threads. A producer
 * takes a random time to produce the data. Once the data is produced, it is 
 * written copied from the producer's buffer into the consumer's buffer, BUT
 * only if the consumer's buffer is empty. The consumer empties its buffer 
 * once it is done processing the data. The processing takes a random amount
 * of time.
 *
 * In this example, we don't have an exchange buffer (see ex5.c). So, the
 * consumer's buffer has to be protected with a mutex.
 *
 * producer       consumer
 * prod_buff ---> cons_buff
 *
 * The producer is the only one adding data to the cons_buffer, and the
 * consumer the only one removing from it. The buffer can be only 1 (buffer
 * full) or 0 (empty). Two condition variables are used to send signals.
 *
 * If the consumer finds its buffer empty, it for signal buffer_full_cv.
 * The producer sends that signal once it is done producing.
 *
 * The consumer then acquires the buffer's mutex and doesn't release it 
 * untill it finishes processing the data. If the producer has produced 
 * new data and wants to write it in the consumer's buffer, it will block when
 * trying to aquire the buffer's mutex, since the consumer has the buffer
 * locked during its processing and only releases it once the buffer is empty.
 * This makes the implementation simpler than ex5.c: There is no need to send a 
 * buffer_emtpy_cv signal to the producer. 
 *
 ******************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TCOUNT 10

int buffer = 0;
pthread_mutex_t buffer_mutex;
pthread_cond_t buffer_full_cv;

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
		printf("\tprod(%d): ready - aquiring buffer mutex\n", i);
		pthread_mutex_lock(&buffer_mutex);
		printf("\tprod(%d): buffer mutex locked\n", i);

		buffer++;
		pthread_cond_signal(&buffer_full_cv);
		printf("\tprod(%d): buffer set to %d - buffer_full_cv sent - unlocking\n",
				i, buffer);
		pthread_mutex_unlock(&buffer_mutex);
	}
	pthread_exit(NULL);
}

void *consume_buffer(void *t) 
{
	/* We will mantain the buffer locked - except when waiting */
	pthread_mutex_lock(&buffer_mutex);
	printf("cons(-): buffer mutex locked\n");

	for (int i = 0; i < TCOUNT; ++i) {

		printf("cons(%d): start\n", i);

		/* Check if there is data in the buffer */
		while (buffer == 0) {
			printf("\tcons(%d): buffer empty - going into wait\n", i);
			pthread_cond_wait(&buffer_full_cv, &buffer_mutex);
			printf("\tcons(%d): buffer_full_cv received: buffer = %d\n", i, buffer);
		}

		/* Process the read data */
		int cons_time = rand() % 5 + 1;
		for (int t = 0; t < cons_time; ++t)
		{
			printf("\tcons(%d): working %d / %ds\n", i, t+1, cons_time);
			sleep(1);
		}

		/* Empty the buffer */
		buffer--;
		printf("\tcons(%d): buffer set to %d - unlocking mutex\n", i, buffer);
		pthread_mutex_unlock(&buffer_mutex);
	}

	printf("cons(-): unlocking mutex\n");
	pthread_mutex_unlock(&buffer_mutex);

	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	pthread_t threads[2];
	pthread_attr_t attr;

	/* Initialize mutex and condition variable objects */
	pthread_mutex_init(&buffer_mutex, NULL);
	pthread_cond_init (&buffer_full_cv, NULL);

	/* For portability, explicitly create threads in a joinable state */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	/* Tasks with random running time */
	time_t t;
	srand((unsigned) time(&t));

	/* Launch threads; pthread_cond_wait must be called first */
	int rc1, rc2;
	rc1 = pthread_create(&threads[0], &attr, produce_buffer, NULL);
	rc2 = pthread_create(&threads[1], &attr, consume_buffer, NULL);

	if (rc1 || rc2) {
		if (rc1) printf("ERROR; return code from pthread_create() is %d\n", rc1);
		if (rc2) printf("ERROR; return code from pthread_create() is %d\n", rc1);
		exit(-1);
	}

	/* Wait for all threads to complete */
	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);

	printf ("Main(): Waited and joined with 2 threads. "
			"Final value of buffer = %d. Done.\n", buffer);

	/* Clean up and exit */
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&buffer_mutex);
	pthread_cond_destroy(&buffer_full_cv);
	pthread_exit (NULL);
}


