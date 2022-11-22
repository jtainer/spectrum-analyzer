// 
// Music visualizer using CUDA Discrete Fourier Transform
//
// 2022, Jonathan Tainer
//

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <math.h>
#include <string.h>
#include <raylib.h>

#include "transform.h"

#define FT_SAMPLES 256	// num of samples for fourier transform
#define FT_CHANNELS 256	// num of channels for fourier transform, should be equal to FT_SAMPLES

#define SMOOTHING_CONST 0.9f	// 0.f == no smoothing, 1.f = max smoothing (no movement)

static float sampleBuffer[FT_SAMPLES];
static float outputBuffer[FT_CHANNELS];
static float resultBuffer[FT_CHANNELS];
static sem_t sampleBufferMutex;

void StreamProcessor(void* data, unsigned int frameCount) {
	sem_wait(&sampleBufferMutex);
	float* sample = (float*) data;
	for (int i = 0; i < frameCount && i < FT_SAMPLES; i++) {
		sampleBuffer[i] = sample[i * 2];
	}
	sem_post(&sampleBufferMutex);
}

int main(int argc, char** argv) {

	// Get command line input
	if (argc < 2) {
		printf("No files specified\n");
		return 0;
	}

	// Window setup
	const int screenWidth = 1920;
	const int screenHeight = 1080;
	SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN | FLAG_VSYNC_HINT);
	InitWindow(screenWidth, screenHeight, argv[1]);
	SetTargetFPS(120);
	ToggleFullscreen();
	InitAudioDevice();

	// Setup buffers and DFT matrix
	memset(sampleBuffer, 0, sizeof(float) * FT_SAMPLES);
	memset(outputBuffer, 0, sizeof(float) * FT_CHANNELS);
	memset(resultBuffer, 0, sizeof(float) * FT_CHANNELS);
	FTransform transform = LoadTransform(FT_SAMPLES, FT_CHANNELS);
	InitScalarTransformThreadPool(transform, sampleBuffer, outputBuffer);

	// Load music file and setup stream processor
	sem_init(&sampleBufferMutex, 0, 1);
	Music music = LoadMusicStream(argv[1]);
	AttachAudioStreamProcessor(music.stream, StreamProcessor);
	PlayMusicStream(music);

	// Colors for background and bars
	Color bgColor = BLACK, barColor = WHITE;

	while (!WindowShouldClose() && IsMusicStreamPlaying(music)) {
		UpdateMusicStream(music);

		// Process most recent samples
		sem_wait(&sampleBufferMutex);
		UpdateScalarTransformThreadPool();
		sem_post(&sampleBufferMutex);

		BeginDrawing();
		ClearBackground(bgColor);
		for (int i = 0; i < FT_CHANNELS; i++) {
			resultBuffer[i] = (SMOOTHING_CONST * resultBuffer[i]) + ((1.f - SMOOTHING_CONST) * outputBuffer[i]);
			Vector2 size = { (float) screenWidth / FT_CHANNELS, log(resultBuffer[i] * 1000) * 100 };
			Vector2 pos = { i * size.x, screenHeight - size.y };
			DrawRectangleV(pos, size, barColor);
		}
//		DrawFPS(10, 10);
		EndDrawing();
	}

	// Cleanup
	StopMusicStream(music);
	UnloadMusicStream(music);
	CloseAudioDevice();
	CloseWindow();
	DestroyScalarTransformThreadPool();
	UnloadTransform(transform);
	sem_destroy(&sampleBufferMutex);
	return 0;
}
