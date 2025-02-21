


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
	// input for getEmissiveQuadSample()
	uint rndSeed;

    // Overloaded: represents RIS weight sum during streaming,
    // then reservoir weight (inverse PDF) after FinalizeResampling
	float weightSum;

	// aka W, weight
	float targetPdf;	// Algorithm 3
	// count
	float M;	// Algorithm 3
    // Visibility information stored in the reservoir for reuse 0..1
//    float visibility;

	// aka RTXDI_EmptyDIReservoir
	void init()
	{
		rndSeed = 0;
		weightSum = 0;
		targetPdf = 0;
		M = 0;
//		visibility = 0;
	}

	// aka RTXDI_IsValidDIReservoir()
	bool isValid()
	{
		return rndSeed != 0;	// todo: improve
	}

	// aka RTXDI_StreamSample()
	// Adds a new, non-reservoir light sample into the reservoir
	// Algorithm (3) from the ReSTIR paper, Streaming RIS using weighted reservoir sampling.
	// @param inRndSeed randomInit(randomSeed.x, randomSeed.y) with some frame contribution
	// @param random 0..1
	void stream(uint inRndSeed, float random, float inTargetPdf, float invSourcePdf)
	{
		float risWeight = inTargetPdf * invSourcePdf;

		++M;
		
		// Update the weight sum
		weightSum += risWeight; 

		if (random * weightSum < risWeight)
		{
			rndSeed = inRndSeed;
			targetPdf = inTargetPdf;
		}
	}

	// aka RTXDI_InternalSimpleResample()
	// @param random 0..1
	void simple(Reservoir other, float random, float inTargetPdf, float sampleNormalization, float inM)
	{
		// What's the current weight (times any prior-step RIS normalization factor)
		float risWeight = inTargetPdf * sampleNormalization;

		// Our *effective* candidate pool is the sum of our candidates plus those of our neighbors
		M += inM;

		weightSum += risWeight;

		if (random * weightSum < risWeight)
		{
			rndSeed = other.rndSeed;
			targetPdf = inTargetPdf;
		}
	}

	// aka RTXDI_CombineDIReservoirs()
	// @param random 0..1
	void combine(Reservoir other, float random, float inTargetPdf)
	{
		float sampleNormalization = other.weightSum * other.M;

		// What's the current weight (times any prior-step RIS normalization factor)
		float risWeight = inTargetPdf * sampleNormalization;

		// Our *effective* candidate pool is the sum of our candidates plus those of our neighbors
		M += other.M;

		weightSum += risWeight;

		if (random * weightSum < risWeight)
		{
			rndSeed = other.rndSeed;
			targetPdf = inTargetPdf;
		}
	}

	// aka RTXDI_FinalizeResampling()
	//  Equation (6) from the ReSTIR paper.
	void finalize(float normalizationNumerator, float normalizationDenominator)
	{
		float denominator = targetPdf * normalizationDenominator;

		weightSum = (denominator == 0.0f) ? 0.0f : weightSum / denominator;
		M = 1;
	}

	// aka RTXDI_LoadDIReservoir()
	void loadFromRaw(ReservoirPacked rawData)
	{
		rndSeed = asuint(rawData.raw[0].x);
		weightSum = rawData.raw[0].y;
		targetPdf = rawData.raw[0].z;
		M = rawData.raw[0].w;
//		visibility = rawData.raw[1].y;
	}

	// aka RTXDI_StoreDIReservoir()
	ReservoirPacked storeToRaw()
	{
		ReservoirPacked ret;

		ret.raw[0].x = asfloat(rndSeed);
		ret.raw[0].y = weightSum;
		ret.raw[0].z = targetPdf;
		ret.raw[0].w = M;
//		ret.raw[1].y = visibility;

		return ret;
	}
};