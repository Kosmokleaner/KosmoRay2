#define PI 3.14159265359

float3 IndexToColor(uint Index)
{
	bool a = Index & (1 << 0);
	bool d = Index & (1 << 1);
	bool g = Index & (1 << 2);

	bool b = Index & (1 << 3);
	bool e = Index & (1 << 4);
	bool h = Index & (1 << 5);

	bool c = Index & (1 << 6);
	bool f = Index & (1 << 7);
	bool i = Index & (1 << 8);

	return float3(a * 4 + b * 2 + c, d * 4 + e * 2 + f, g * 4 + h * 2 + i) / 7.0f;
}

// https://www.diva-portal.org/smash/get/diva2:1573746/FULLTEXT01.pdf
// Generates a seed for a random number generator from 2 inputs
uint initRand(uint val0, uint val1, uint backoff = 16)
{
	uint v0 = val0, v1 = val1, s0 = 0;
	
	[unroll]
	for (uint n = 0; n < backoff; n++)
	{
		s0 += 0x9e3779b9;
		v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
		v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
	}
	return v0;
}

// Returns a pseudorandom float in [0..1] from seed
float nextRand(inout uint s)
{
	s = (1664525u * s + 1013904223u);
	return float(s & 0x00FFFFFF) / float(0x01000000);
}
// @return float2(0..1, 0..1)
float2 nextRand2(inout uint s)
{
	return float2(nextRand(s), nextRand(s));
}
// can be optimized
// https://www.pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations#SamplingaUnitDisk
// @return float2(-1..1, -1..1)
float2 nextRand2Disc(inout uint s)
{
	// 0..1
	float2 ret = nextRand2(s);
	float2 uOffset = ret * 2 - 1;

	if (uOffset.x == 0 && uOffset.y == 0)
		return float2(0, 0);

	float theta, r;
	if (abs(uOffset.x) > abs(uOffset.y)) {
		r = uOffset.x;
		theta = PI*4 * (uOffset.y / uOffset.x);
	}
	else {
		r = uOffset.y;
		theta = PI*2 - PI*4 * (uOffset.x / uOffset.y);
	}
	return r * float2(cos(theta), sin(theta));
}
// @return float3(-1..1, -1..1, 0..1), length()=1
float3 nextRand2CosineWeightedHemisphere(inout uint s)
{
	float2 disc = nextRand2Disc(s);

	return float3(disc.x, disc.y, sqrt(1.0f - disc.x * disc.x - disc.y * disc.y));
}
