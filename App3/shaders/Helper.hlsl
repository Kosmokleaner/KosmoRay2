

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