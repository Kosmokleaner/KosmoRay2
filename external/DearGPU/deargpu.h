#pragma once

// first include for DearGPU code 
// to integrate into another project with little changes

#include <tchar.h>
typedef unsigned int UINT;
typedef unsigned int uint32;
typedef unsigned char uint8;
typedef float FLOAT;
typedef char CHAR;

// uint16:16bit (might be slower) uint32:32bit (2x more memory but larger meshes are possible)
#define INDEXBUFFER_TYPE uint16

// affects bounding box quality, building time (linear) and performance (20: very high quality), (delete cache folder if you change it)
#define BVH_TEST_SPLIT_PLANES 10


#include "../../Mathlib.h" // float3

#include <winerror.h> // HRESULT


