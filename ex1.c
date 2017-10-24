#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// first example: create and joint threads

#define NUM_THREADS 2

// create thread argument struct for thr_func()
typedef struct _thread_data_t {
	int tid;
	double stuff;
} thread_data_t;

// thread function
void *thr_func(void *arg) {
	thread_data_t *data = (thread_data_t *)arg;
	printf("thread %d: start\n", data->tid);
	sleep(3 * (data->tid + 1));
	printf("thread %d: stop\n", data->tid);
	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

	// threads
	pthread_t thr[NUM_THREADS];

	// create a thread_data_t argument array
	thread_data_t thr_data[NUM_THREADS];

	// create threads
	for (int i = 0; i < NUM_THREADS; ++i) {
		thr_data[i].tid = i;
		int rc;
		if ((rc = pthread_create(&thr[i], NULL, thr_func, &thr_data[i]))) {
			fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
			return EXIT_FAILURE;
		}
	}

	// block until all threads complete
	for (int i = 0; i < NUM_THREADS; ++i) {
		pthread_join(thr[i], NULL);
	}
	
	return EXIT_SUCCESS;
}
