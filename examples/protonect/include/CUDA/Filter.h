/*
 * Filter.h -- a header file with the GPU filter wrapper declaration
 */

typedef unsigned char uchar;

void FilterGPU(const uchar* in, uchar* out, int ROWS, int COLS, uchar threshold);
