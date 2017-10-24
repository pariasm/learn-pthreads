#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// mutexes

#define NUM_THREADS 5

// create thread argument struct for thr_func()
typedef struct _thread_data_t {
	int tid;
	double stuff;
	double *p_shared_x;
	pthread_mutex_t *p_lock_x;
} thread_data_t;

// thread function
void *thr_func(void *arg) {
	thread_data_t *data = (thread_data_t *)arg;
	printf("thread %d\n", data->tid);

	// get mutex before modifying and printing shared_x
	pthread_mutex_lock(data->p_lock_x);
	{
		*data->p_shared_x += data->stuff;
		printf("thread %d: x = %f\n", data->tid, *data->p_shared_x);
		sleep(1); // do some stuff
	}
	pthread_mutex_unlock(data->p_lock_x);
		
	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

	// threads
	pthread_t thr[NUM_THREADS];

	// create a thread_data_t argument array
	thread_data_t thr_data[NUM_THREADS];

	// shared data between threads
	double shared_x = 0;
	pthread_mutex_t lock_x;

	// initialize pthread mutex protecting shared_x
	pthread_mutex_init(&lock_x, NULL);

	// create threads
	for (int i = 0; i < NUM_THREADS; ++i) {
		thr_data[i].tid = i;
		thr_data[i].stuff = (i + 1) * NUM_THREADS;
		thr_data[i].p_shared_x = &shared_x;
		thr_data[i].p_lock_x = &lock_x;
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

	// destroy mutex
	pthread_mutex_destroy(&lock_x);

	// is this necessary after the join?
	pthread_exit(NULL);
	
	return EXIT_SUCCESS;
}
