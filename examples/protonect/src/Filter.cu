/*
 * Filter.cu - a GPU program to perform threshold filtering
 */

#include <stdio.h>
#include <cuda_runtime.h>

#include "Filter.h"

#define BLOCK_SIZE 512

// This will output the proper CUDA error strings in the event that a CUDA host call returns an error
#define checkCudaErrors(err)  __checkCudaErrors(err, __FILE__, __LINE__)

inline void __checkCudaErrors(cudaError err, const char *file, const int line )
{
    if(cudaSuccess != err)
    {
        fprintf(stderr, "%s(%i) : CUDA Runtime API error %d: %s.\n",file, line, (int)err, cudaGetErrorString( err ) );
        exit(-1);
    }
}

__global__ 
void FilterKernel(const uchar *in, uchar* out, const int width, const uchar threshold)
{
	int idx =  blockIdx.y * width + blockIdx.x * BLOCK_SIZE + threadIdx.x;
	out[idx] = (in[idx] < threshold) ? in[idx] : 0;
}

void FilterGPU( const uchar* h_in, uchar* h_out, int height, int width, uchar threshold)
{
	int SIZE = height * width;
	cudaEvent_t start, stop;

	checkCudaErrors( cudaEventCreate(&start) );
	checkCudaErrors( cudaEventCreate(&stop) );

	// Allocate the device input image
	uchar *d_in = NULL;
	checkCudaErrors( cudaMalloc((void **)&d_in, SIZE) );

	// Allocate the device output image
	uchar *d_out = NULL;
	checkCudaErrors( cudaMalloc((void **)&d_out, SIZE ) );

	// Copy the host input image  to the device memory
	checkCudaErrors( cudaMemcpy(d_in, h_in, SIZE, cudaMemcpyHostToDevice) );
	
	checkCudaErrors( cudaEventRecord(start, NULL) );

	// Launch the CUDA Kernel
	dim3 block(BLOCK_SIZE);
	dim3 grid(width/BLOCK_SIZE, height);
	FilterKernel<<<grid, block>>>( d_in, d_out, width, threshold);
	checkCudaErrors( cudaGetLastError() );

	checkCudaErrors( cudaEventRecord(stop, NULL) );
	checkCudaErrors( cudaEventSynchronize(stop) );

	// Copy the device result to the host
	checkCudaErrors( cudaMemcpy(h_out, d_out, SIZE, cudaMemcpyDeviceToHost) );

	checkCudaErrors( cudaFree(d_in) );
	checkCudaErrors( cudaFree(d_out) );

	float msec = 0.f;
	checkCudaErrors( cudaEventElapsedTime(&msec, start, stop) );

	printf("GPU code ran for: %f ms\n", msec);
}
