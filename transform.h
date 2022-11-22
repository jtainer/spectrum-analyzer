// 
// DFT matrix structure and functions
//
// 2022, Jonathan Tainer
//

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "complex.h"

typedef struct FTransform {
	unsigned int samples;
	unsigned int channels;
	Complex* matrix;
} FTransform;

Complex* CreateDFTMatrix(unsigned int samples, unsigned int channels);

FTransform LoadTransform(unsigned int samples, unsigned int channels);

void UnloadTransform(FTransform transform);

void InitScalarTransformThreadPool(FTransform transform, float* input, float* output);

void UpdateScalarTransformThreadPool();

void DestroyScalarTransformThreadPool();

#endif
