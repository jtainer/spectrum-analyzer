// 
// DFT matrix functions
//
// 2022, Jonathan Tainer
//

#include "transform.h"
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct ScalarThreadArg {
	float* input;
	Complex* matrix;
	float* output;
	unsigned int samples;
} ScalarThreadArg;

Complex* CreateDFTMatrix(unsigned int samples, unsigned int channels) {
	Complex* matrix = (Complex*)malloc(sizeof(Complex) * samples * channels);

	float w = 16.35f * 2.f * M_PI / 44100.f;	// Base frequency of 16.35 Hz which is the lowest C on a grand piano
	const float g = exp((1.f/12.f) * log(2.f));	// Ratio of 1 semitone (equal temperment) between channels

	for (unsigned int y = 0; y < channels; y++) {
		for (unsigned int x = 0; x < samples; x++) {
			unsigned int index = x + (samples * y);
			float ang = w * x;
			matrix[index].real = cosf(ang);
			matrix[index].imag = sinf(ang);
		}
		w *= g;
	}

	return matrix;
}

FTransform LoadTransform(unsigned int samples, unsigned int channels) {
	FTransform transform;
	transform.samples = samples;
	transform.channels = channels;
	transform.matrix = CreateDFTMatrix(samples, channels);
	return transform;
}

void UnloadTransform(FTransform transform) {
	free(transform.matrix);
}

static sem_t block_threads;
static sem_t wait_threads;
static pthread_t* thread;
static unsigned int numthreads;
static ScalarThreadArg* arg;
static char exit_threads = 0;

void* ScalarMultiplyThreadPoolFunc(void* args) {

	ScalarThreadArg arg = *(ScalarThreadArg*)args;

	while (!exit_threads) {
		sem_wait(&block_threads);
		
		Complex sum = { 0, 0 };
		
		for (int i = 0; i < arg.samples; i++) {
			sum.real += arg.input[i] * arg.matrix[i].real;
			sum.imag += arg.input[i] * arg.matrix[i].imag;
		}
		
		*arg.output = sqrtf((sum.real * sum.real) + (sum.imag * sum.imag)) / arg.samples;

		sem_post(&wait_threads);
	}
	return NULL;
}

void InitScalarTransformThreadPool(FTransform transform, float* input, float* output) {
	sem_init(&block_threads, 0, 0);
	sem_init(&wait_threads, 0, 0);
	numthreads = transform.channels;
	thread = malloc(numthreads * sizeof(pthread_t));
	arg = malloc(numthreads * sizeof(ScalarThreadArg));
	
	// Create thread arguments
	for (int i = 0; i < numthreads; i++) {
		arg[i].input = input;
		arg[i].matrix = &transform.matrix[i * transform.samples];
		arg[i].output = &output[i];
		arg[i].samples = transform.samples;

		pthread_create(&thread[i], NULL, ScalarMultiplyThreadPoolFunc, &arg[i]);
	}
}

void UpdateScalarTransformThreadPool() {
	// Let threads through
	for (int i = 0; i < numthreads; i++) {
		sem_post(&block_threads);
	}

	// Wait for threads to finish
	for (int i = 0; i < numthreads; i++) {
		sem_wait(&wait_threads);
	}
}

void DestroyScalarTransformThreadPool() {
	// Set threads to terminate and let through while loop
	exit_threads = 1;
	for (int i = 0; i < numthreads; i++) {
		sem_post(&block_threads);
	}

	// Destroy threads
	for (int i = 0; i < numthreads; i++) {
		pthread_join(thread[i], NULL);
	}

	// Deallocate memory
	sem_destroy(&block_threads);
	sem_destroy(&wait_threads);
	free(thread);
	free(arg);
}
