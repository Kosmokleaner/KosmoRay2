#include "Mathlib.h"

float3 float3::ComputeNormalizedRandomDirection()
{
	return normalize(ComputeRandomDirection());
}
