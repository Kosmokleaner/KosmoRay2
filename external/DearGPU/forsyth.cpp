/*
  Copyright (C) 2008 Martin Storsjo

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "deargpu.h"			// MM change
#include "forsyth.h"			// MM change

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <math.h>
#include <string.h>


// The size of these data types affect the memory usage
typedef uint16_t ScoreType;
#define SCORE_SCALING 7281

typedef uint8_t AdjacencyType;
#define MAX_ADJACENCY UINT8_MAX

typedef int8_t  CachePosType;
typedef int32_t TriangleIndexType;
typedef int32_t ArrayIndexType;

// The size of the precalculated tables
#define CACHE_SCORE_TABLE_SIZE 32
#define VALENCE_SCORE_TABLE_SIZE 32
#if CACHE_SCORE_TABLE_SIZE < VERTEX_CACHE_SIZE
#error Vertex score table too small
#endif

// Precalculated tables
static ScoreType cachePositionScore[CACHE_SCORE_TABLE_SIZE];
static ScoreType valenceScore[VALENCE_SCORE_TABLE_SIZE];

#define ISADDED(x)  (triangleAdded[(x) >> 3] &  (1 << (x & 7)))
#define SETADDED(x) (triangleAdded[(x) >> 3] |= (1 << (x & 7)))

// Score function constants
#define CACHE_DECAY_POWER	1.5
#define LAST_TRI_SCORE		0.75
#define VALENCE_BOOST_SCALE	2.0
#define VALENCE_BOOST_POWER	0.5

// Precalculate the tables
void initForsyth() {
	for (int i = 0; i < CACHE_SCORE_TABLE_SIZE; i++) {
		float score = 0;
		if (i < 3) {
			// This vertex was used in the last triangle,
			// so it has a fixed score, which ever of the three
			// it's in. Otherwise, you can get very different
			// answers depending on whether you add
			// the triangle 1,2,3 or 3,1,2 - which is silly
			score = LAST_TRI_SCORE;
		} else {
			// Points for being high in the cache.
			const float scaler = 1.0f / (CACHE_FUNCTION_LENGTH - 3);
			score = 1.0f - (i - 3) * scaler;
			score = powf(score, CACHE_DECAY_POWER);
		}
		cachePositionScore[i] = (ScoreType) (SCORE_SCALING * score);
	}

	for (int i = 1; i < VALENCE_SCORE_TABLE_SIZE; i++) {
		// Bonus points for having a low number of tris still to
		// use the vert, so we get rid of lone verts quickly
		float valenceBoost = powf((float)i, -VALENCE_BOOST_POWER);
		float score = (float)(VALENCE_BOOST_SCALE * valenceBoost);
		valenceScore[i] = (ScoreType) (SCORE_SCALING * score);
	}
}

// Calculate the score for a vertex
ScoreType findVertexScore(int numActiveTris,
                          int cachePosition) {
	if (numActiveTris == 0) {
		// No triangles need this vertex!
		return 0;
	}

	ScoreType score = 0;
	if (cachePosition < 0) {
		// Vertex is not in LRU cache - no score
	} else {
		score = cachePositionScore[cachePosition];
	}

	if (numActiveTris < VALENCE_SCORE_TABLE_SIZE)
		score += valenceScore[numActiveTris];
	return score;
}

// The main reordering function
VertexIndexType* reorderForsyth(const VertexIndexType* indices,
                                int nTriangles,
                                int nVertices) {

	// The tables need not be inited every time this function
	// is used. Either call initForsyth from the calling process,
	// or just replace the score tables with precalculated values.
	initForsyth();

	AdjacencyType* numActiveTris = new AdjacencyType[nVertices];
	memset(numActiveTris, 0, sizeof(AdjacencyType)*nVertices);

	// First scan over the vertex data, count the total number of
	// occurrances of each vertex
	for (int i = 0; i < 3*nTriangles; i++) {
		if (numActiveTris[indices[i]] == MAX_ADJACENCY) {
			// Unsupported mesh,
			// vertex shared by too many triangles
			delete [] numActiveTris;
			return NULL;
		}
		numActiveTris[indices[i]]++;
	}

	// Allocate the rest of the arrays
	ArrayIndexType* offsets = new ArrayIndexType[nVertices];
	ScoreType* lastScore = new ScoreType[nVertices];
	CachePosType* cacheTag = new CachePosType[nVertices];

	uint8_t* triangleAdded = new uint8_t[(nTriangles + 7)/8];
	ScoreType* triangleScore = new ScoreType[nTriangles];
	TriangleIndexType* triangleIndices = new TriangleIndexType[3*nTriangles];
	memset(triangleAdded, 0, sizeof(uint8_t)*((nTriangles + 7)/8));
	memset(triangleScore, 0, sizeof(ScoreType)*nTriangles);
	memset(triangleIndices, 0, sizeof(TriangleIndexType)*3*nTriangles);

	// Count the triangle array offset for each vertex,
	// initialize the rest of the data.
	int sum = 0;
	for (int i = 0; i < nVertices; i++) {
		offsets[i] = sum;
		sum += numActiveTris[i];
		numActiveTris[i] = 0;
		cacheTag[i] = -1;
	}

	// Fill the vertex data structures with indices to the triangles
	// using each vertex
	for (int i = 0; i < nTriangles; i++) {
		for (int j = 0; j < 3; j++) {
			int v = indices[3*i + j];
			triangleIndices[offsets[v] + numActiveTris[v]] = i;
			numActiveTris[v]++;
		}
	}

	// Initialize the score for all vertices
	for (int i = 0; i < nVertices; i++) {
		lastScore[i] = findVertexScore(numActiveTris[i], cacheTag[i]);
		for (int j = 0; j < numActiveTris[i]; j++)
			triangleScore[triangleIndices[offsets[i] + j]] += lastScore[i];
	}

	// Find the best triangle
	int bestTriangle = -1;
	int bestScore = -1;

	for (int i = 0; i < nTriangles; i++) {
		if (triangleScore[i] > bestScore) {
			bestScore = triangleScore[i];
			bestTriangle = i;
		}
	}

	// Allocate the output array
	TriangleIndexType* outTriangles = new TriangleIndexType[nTriangles];
	int outPos = 0;

	// Initialize the cache
	int cache[VERTEX_CACHE_SIZE + 3];
	for (int i = 0; i < VERTEX_CACHE_SIZE + 3; i++)
		cache[i] = -1;

	int scanPos = 0;

	// Output the currently best triangle, as long as there
	// are triangles left to output
	while (bestTriangle >= 0) {
		// Mark the triangle as added
		SETADDED(bestTriangle);
		// Output this triangle
		outTriangles[outPos++] = bestTriangle;
		for (int i = 0; i < 3; i++) {
			// Update this vertex
			int v = indices[3*bestTriangle + i];

			// Check the current cache position, if it
			// is in the cache
			int endpos = cacheTag[v];
			if (endpos < 0)
				endpos = VERTEX_CACHE_SIZE + i;
			// Move all cache entries from the previous position
			// in the cache to the new target position (i) one
			// step backwards
			for (int j = endpos; j > i; j--) {
				cache[j] = cache[j-1];
				// If this cache slot contains a real
				// vertex, update its cache tag
				if (cache[j] >= 0)
					cacheTag[cache[j]]++;
			}
			// Insert the current vertex into its new target
			// slot
			cache[i] = v;
			cacheTag[v] = i;

			// Find the current triangle in the list of active
			// triangles and remove it (moving the last
			// triangle in the list to the slot of this triangle).
			for (int j = 0; j < numActiveTris[v]; j++) {
				if (triangleIndices[offsets[v] + j] == bestTriangle) {
					triangleIndices[offsets[v] + j] =
					    triangleIndices[
					        offsets[v] + numActiveTris[v] - 1];
					break;
				}
			}
			// Shorten the list
			numActiveTris[v]--;
		}
		// Update the scores of all triangles in the cache
		for (int i = 0; i < VERTEX_CACHE_SIZE + 3; i++) {
			int v = cache[i];
			if (v < 0)
				break;
			// This vertex has been pushed outside of the
			// actual cache
			if (i >= VERTEX_CACHE_SIZE) {
				cacheTag[v] = -1;
				cache[i] = -1;
			}
			ScoreType newScore = findVertexScore(numActiveTris[v],
			                                     cacheTag[v]);
			ScoreType diff = newScore - lastScore[v];
			for (int j = 0; j < numActiveTris[v]; j++)
				triangleScore[triangleIndices[offsets[v] + j]] += diff;
			lastScore[v] = newScore;
		}
		// Find the best triangle referenced by vertices in the cache
		bestTriangle = -1;
		bestScore = -1;
		for (int i = 0; i < VERTEX_CACHE_SIZE; i++) {
			if (cache[i] < 0)
				break;
			int v = cache[i];
			for (int j = 0; j < numActiveTris[v]; j++) {
				int t = triangleIndices[offsets[v] + j];
				if (triangleScore[t] > bestScore) {
					bestTriangle = t;
					bestScore = triangleScore[t];
				}
			}
		}
		// If no active triangle was found at all, continue
		// scanning the whole list of triangles
		if (bestTriangle < 0) {
			for (; scanPos < nTriangles; scanPos++) {
				if (!ISADDED(scanPos)) {
					bestTriangle = scanPos;
					break;
				}
			}
		}
	}

	// Clean up
	delete [] triangleIndices;
	delete [] offsets;
	delete [] lastScore;
	delete [] numActiveTris;
	delete [] cacheTag;
	delete [] triangleAdded;
	delete [] triangleScore;

	// Convert the triangle index array into a full triangle list
	VertexIndexType* outIndices = new VertexIndexType[3*nTriangles];
	outPos = 0;
	for (int i = 0; i < nTriangles; i++) {
		int t = outTriangles[i];
		for (int j = 0; j < 3; j++) {
			int v = indices[3*t + j];
			outIndices[outPos++] = v;
		}
	}
	delete [] outTriangles;

	return outIndices;
}

