


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

struct ReservoirPacked
{
	float4 raw[2];
};

struct Reservoir
{
	// aka lightSeed, lightData (Light index (bits 0..30) and validity bit (31))
	uint rndState;

    // Overloaded: represents RIS weight sum during streaming,
    // then reservoir weight (inverse PDF) after FinalizeResampling
	float weightSum;

	// aka W, weight
	float targetPdf;	// Algorithm 3
	// count
	float M;	// Algorithm 3
	//todo: 3 bits might be enough, todo: could be replaced by stochastic exponential
	float age;
    // Visibility information stored in the reservoir for reuse 0..1
    float visibility;

	// aka RTXDI_EmptyDIReservoir
	void init()
	{
		rndState = 0;
		weightSum = 0;
		targetPdf = 0;
		M = 0;
		age = 0;
		visibility = 0;
	}

	// aka RTXDI_IsValidDIReservoir()
	bool isValid()
	{
		return rndState != 0;	// todo: improve
	}

	// aka RTXDI_StreamSample()
	// Adds a new, non-reservoir light sample into the reservoir
	// Algorithm (3) from the ReSTIR paper, Streaming RIS using weighted reservoir sampling.
	// @param inRndState randomInit(randomSeed.x, randomSeed.y) with some frame contribution
	// @param random 0..1
	bool stream(uint inRndState, float random, float inTargetPdf, float invSourcePdf)
	{
		float risWeight = inTargetPdf * invSourcePdf;

		// Add one sample to the counter
		M += 1;
		
		// Update the weight sum
		weightSum += risWeight; 

		bool selectSample = (random * weightSum < risWeight);
		if (selectSample)
		{
			rndState = inRndState;
			targetPdf = inTargetPdf;
		}
/*
		// WeightedReserviour Sampling Algorithm 2 (plus a depth test)
		// Min-Te Chao. 1982. A General Purpose Unequal Probability Sampling Plan. Biometrika 69, 3 (Dec. 1982), 653?656. https://doi.org/10/fd87zs
		// https://research.nvidia.com/sites/default/files/pubs/2020-07_Spatiotemporal-reservoir-resampling/ReSTIR.pdf
*/
		return selectSample;
	}

	// aka RTXDI_InternalSimpleResample()
	// @param random 0..1
	bool simple(Reservoir other, float random, float inTargetPdf, float sampleNormalization, float inM)
	{
		// What's the current weight (times any prior-step RIS normalization factor)
		float risWeight = inTargetPdf * sampleNormalization;

		// Our *effective* candidate pool is the sum of our candidates plus those of our neighbors
		M += inM;

		weightSum += risWeight;

		bool selectSample = (random * weightSum < risWeight);
		if (selectSample)
		{
			rndState = other.rndState;
			targetPdf = inTargetPdf;
			age = other.age;
		}
		return selectSample;
	}

	// aka RTXDI_CombineDIReservoirs()
	// @param random 0..1
	bool combine(Reservoir other, float random, float inTargetPdf)
	{
		return simple(other, random, inTargetPdf, other.weightSum * other.M, other.M);
	}

	// aka RTXDI_FinalizeResampling()
	//  Equation (6) from the ReSTIR paper.
	void finalize(float normalizationNumerator, float normalizationDenominator)
	{
//		float denominator = targetPdf;
		float denominator = targetPdf * normalizationDenominator;

		weightSum = (denominator == 0.0f) ? 0.0f : weightSum / denominator;
		M = 1;
	}

	void loadFromRaw(ReservoirPacked rawData)
	{
		rndState = asuint(rawData.raw[0].x);
		weightSum = rawData.raw[0].y;
		targetPdf = rawData.raw[0].z;
		M = rawData.raw[0].w;
		age = rawData.raw[1].x;
		visibility = rawData.raw[1].y;
	}

	ReservoirPacked storeToRaw()
	{
		ReservoirPacked ret;

		ret.raw[0].x = asfloat(rndState);
		ret.raw[0].y = weightSum;
		ret.raw[0].z = targetPdf;
		ret.raw[0].w = M;
		ret.raw[1].x = age;
		ret.raw[1].y = visibility;

		return ret;
	}
};