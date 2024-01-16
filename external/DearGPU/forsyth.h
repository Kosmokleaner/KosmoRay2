
// Set these to adjust the performance and result quality
#define VERTEX_CACHE_SIZE 8
#define CACHE_FUNCTION_LENGTH 32


//typedef int32_t VertexIndexType;
typedef INDEXBUFFER_TYPE VertexIndexType;

void initForsyth();

VertexIndexType* reorderForsyth(const VertexIndexType* indices, int nTriangles,	int nVertices);
