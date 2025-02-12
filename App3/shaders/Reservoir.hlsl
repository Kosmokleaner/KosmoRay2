


// Mersenne Twister, see randomNext()
uint randomInit(uint val0, uint val1, uint backoff = 16)
{
	uint v0 = val0;
	uint v1 = val1;
	uint s0 = 0;

	for (uint n = 0; n < backoff; n++)
	{
		s0 += 0x9e3779b9;
		v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
		v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
	}

	return v0;
}

// @param s see randomInit()
// @return 0..1
float randomNext(inout uint s)
{
	uint LCG_A = 1664525u;
	uint LCG_C = 1013904223u;
	s = (LCG_A * s + LCG_C);
	return float(s & 0x00FFFFFF) / float(0x01000000);
}



struct Reservoir
{
	//
	uint rndState;
	// weight sum
	float wSum;
	//
	float W;	// Algorithm 3
	// count
	float M;	// Algorithm 3

	void init()
	{
		// >0 to avoid div by 0, needed?
//		wSum = 0.00001f;
		wSum = 0;
		W = 0;
		M = 0;
		rndState = 0;
	}

	// @param inRndState randomInit(randomSeed.x, randomSeed.y) with some frame contribution
	void push(uint inRndState, float weight)
	{
		++M;

		// WeightedReserviour Sampling Algorithm 2 (plus a depth test)
		// Min-Te Chao. 1982. A General Purpose Unequal Probability Sampling Plan. Biometrika 69, 3 (Dec. 1982), 653?656. https://doi.org/10/fd87zs
		// https://research.nvidia.com/sites/default/files/pubs/2020-07_Spatiotemporal-reservoir-resampling/ReSTIR.pdf


//		float depth = splatZ.x;
		// todo: improve

		wSum += weight;

		// make a copy as we don't want to change rndState
		float rnd;
		{
			uint copy = rndState;
			rnd = randomNext(copy);
		}

//		if (rnd < weight / wSum)
		if (rnd * wSum < weight) // optimized divide
		{
			rndState = inRndState;
			W = weight;					// ??
		}
	}

	void loadFromRaw(float4 rawData)
	{
		rndState = asuint(rawData.x);
		wSum = rawData.y;
		W = rawData.z;
		M = rawData.w;
	}

	float4 storeToRaw()
	{
		return float4(asfloat(rndState), wSum, W, M);
	}
};