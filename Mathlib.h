#pragma once

#include <math.h>
#include <cmath>
#include <assert.h>

#include <directxmath.h>
using namespace DirectX;

#include <algorithm>			// std::swap
#include <stdlib.h> // rand()

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef char int8;
typedef short int16;
typedef int int32;

#define PI 3.14159265f

#define ARRLEN(a) (sizeof(a) / sizeof(a[0]))


inline float sqr(float x)
{
	return x * x;
}

struct FVector2I
{
	FVector2I()
	{
	}

	FVector2I(int32 InX, int32 InY)
		: x(InX), y(InY)
	{
	}

	int32 x;
	int32 y;
};

#define MAX_RESOURCE_VIEW_CACHE_SIZE 8

template<class T>
void SafeRelease(T*& p)
{
	if (p)
	{
		p->Release();
		p = 0;
	}
}


inline float frac(float x)
{
	return x - floorf(x);
}

inline double frac(double x)
{
	return x - floor(x);
}

inline uint32 DivideAndRoundUp(uint32 Value, uint32 Div)
{
	return (Value + Div - 1) / Div;
}

struct TNoSafeRelease
{
	template <class T>
	static void DoRelease(T* Ptr)
	{
		assert(Ptr);
	}

	template <class T>
	static void DoAddRef(T* Ptr)
	{
		assert(Ptr);
	}
};

struct TSafeRelease
{
	template <class T>
	static void DoRelease(T* Ptr)
	{
		assert(Ptr);
		Ptr->Release();
	}

	template <class T>
	static void DoAddRef(T* Ptr)
	{
		assert(Ptr);
		Ptr->AddRef();
	}
};

// simple pointer wrapper
// zero init on construction
// @param TTrait = TSafeRelease / TNoSafeRelease
template <class T, class TTrait = TNoSafeRelease>
class Pointer
{
public:
	// default constructor
	Pointer() :Ptr(0)
	{
	}

	// 0 constructor
	//	Pointer(int Null) :Ptr(0)
	//	{
	//		assert(!Null);		// todo: compile time error
	//	}

	// constructor
	//	Pointer(T *ref) :Ptr(ref)
	//	{
	//	}

	// copy constructor
	Pointer(const Pointer<T, TTrait>& ref) :Ptr(ref.Get())
	{
		if (Ptr)
		{
			TTrait::DoAddRef(Ptr);
		}
	}

	// copy constructor
	template<class Derived>
	Pointer(const Pointer<Derived, TTrait> ref) :Ptr(ref.Get())
	{
	}

	// destructor
	~Pointer()
	{
		Release();
	}

	// assignment operator
	template<class Derived>
	Pointer<T, TTrait> &operator=(const Pointer<Derived, TTrait> ref)
	{
		if (this != &ref) // protect against invalid self-assignment
		{
			Ptr = ref.Get();
		}
		return *this;
	}

	// assignment operator
	Pointer<T, TTrait> &operator=(T* ptr)
	{
		Release();
		Ptr = ptr;
		return *this;
	}

	// assignment operator
	Pointer<T, TTrait> &operator=(int Null)
	{
		assert(Null == 0);

		Release();
		return *this;
	}

	T& operator*()
	{
		return *Ptr;
	}

	// doesn't work with void type
	const T& operator*() const
	{
		return *Ptr;
	}

	T* operator->()
	{
		return Ptr;
	}

	const T* operator->() const
	{
		return Ptr;
	}

	// dangerous? Maybe better we go though Get()
	T** operator&()
	{
		return &Ptr;
	}

	// dangerous?
	operator T*()
	{
		return Ptr;
	}

	T* Get() const
	{
		return Ptr;
	}

	T* &Get()
	{
		return Ptr;
	}

	operator bool() const
	{
		return Ptr != 0;
	}

	/*	bool operator ==(const Pointer<T> &ref) const
	{
	return Ptr == ref.Ptr;
	}


	bool operator ==(const T* ref) const
	{
	return Ptr == ref;
	}


	bool operator !=(const Pointer<T> &ref) const
	{
	return !(*this == ref);
	}


	bool operator !=(const T* ref) const
	{
	return !(*this == ref);
	}*/

	bool operator!() const
	{
		return Ptr == 0;
	}

	void Release()
	{
		if (Ptr)
		{
			TTrait::DoRelease(Ptr);
			Ptr = 0;
		}
	}

private:
	T *				Ptr;

};


template <class T>
class TInitZero
{
public:
	TInitZero()
		: Value((T)0)
	{
	}

	TInitZero(const T& InValue)
		: Value(InValue)
	{
	}

	/*
	// assignment operator
	TInitZero<T> &operator=(const T& in)
	{
	Value = in;
	return *this;
	}
	*/

	operator const T() const
	{
		return Value;
	}

	/*
	const T& T::operator++()
	{
	++itsVal;
	return *this;
	}

	const T T::operator++(int)
	{
	T temp(*this);
	++itsVal;
	return temp;
	}
	*/


	T operator ->() const
	{
		return Value;
	}

	// dangerous to expose? needed for ++Value
	operator T& ()
	{
		return Value;
	}

	T& GetRef()
	{
		return Value;
	}

	// dangerous? Maybe better we go though Get()
	T* operator&()
	{
		return &Value;
	}


private:
	T Value;
};

template<class T, class TTrait>
void SafeRelease(Pointer<T, TTrait>& p)
{
	if (p)
	{
		p->Release();
		p = 0;
	}
};

inline XMFLOAT3 operator+(const XMFLOAT3& a, const XMFLOAT3& b)
{
	// would be faster if using SIMD
	return XMFLOAT3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline XMFLOAT3 operator-(const XMFLOAT3& a, const XMFLOAT3& b)
{
	// would be faster if using SIMD
	return XMFLOAT3(a.x - b.x, a.y - b.y, a.z - b.z);
}


inline XMFLOAT3 operator*(const XMFLOAT3& a, const float b)
{
	// would be faster if using SIMD
	return XMFLOAT3(a.x * b, a.y * b, a.z * b);
}

template <class T>
T lerp(const T& a, const T& b, float x)
{
	return a * (1 - x) + b * x;
}

inline float frand()
{
	return rand() / (float)RAND_MAX;
}

// @return 0 .. Count-1
inline uint32 randInt(uint32 Count)
{
	uint32 Ret = (uint32)(uint64(rand() * Count) / RAND_MAX);

	assert(Ret < Count);

	return Ret;
}

inline float frand2()
{
	return frand() * 2 - 1.0f;
}


#include <stdio.h>														// FILE
#include <fcntl.h>														// filesize
#include <io.h>															// filesize
#include <string.h>														// strlen

#pragma warning( disable : 4996 )

/*
inline uint32 IO_GetFileSize(const wchar_t *Name)
{
	assert(Name);

	WIN32_FILE_ATTRIBUTE_DATA fileInfo;

	if (!GetFileAttributesEx(Name, GetFileExInfoStandard, (void*)&fileInfo))
	{
		return 0;
	}

	// cannot handle files larger than 32bit 4GB
	assert(0 == fileInfo.nFileSizeHigh);

	return (uint32)fileInfo.nFileSizeLow;
}

inline uint32 IO_GetFileSize(const char *Name)
{
	assert(Name);
	int handle, size;	// test2

	handle = open(Name, O_RDONLY);
	if (handle == -1)
	{
		return 0;
	}

	size = filelength(handle);
	close(handle);

	return size;
}

*/


// ---------------------------------------------------------------------------------------------------------


inline float saturate(float x)
{
	if (x < 0)
		return 0;
	if (x > 1)
		return 1;

	return x;
}


/*
// from: http://stackoverflow.com/questions/19301538/converting-tchar-to-char
// multi byte to wide char:
inline std::wstring s2ws(const std::string& str)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}
// wide char to multi byte:
inline std::string ws2s(const std::wstring& wstr)
{
	int size_needed = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), int(wstr.length() + 1), 0, 0, 0, 0);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), int(wstr.length() + 1), &strTo[0], size_needed, 0, 0);
	return strTo;
}
*/

//------------------------------------------------------------------------------------
//Helper functions to convert from Oculus types to XM types - consider to add to SDK
#if G_OCULUS_VR
inline XMVECTOR ConvertToXM(ovrQuatf q) { return XMVectorSet(q.x, q.y, q.z, q.w); }
inline XMFLOAT4 ConvertToXMF(ovrQuatf q) { return XMFLOAT4(q.x, q.y, q.z, q.w); }
inline XMVECTOR ConvertToXM(ovrVector3f v) { return XMVectorSet(v.x, v.y, v.z, 0); }
inline XMFLOAT3 ConvertToXMF(ovrVector3f v) { return XMFLOAT3(v.x, v.y, v.z); }
inline XMMATRIX ConvertToXM(ovrMatrix4f p)
{
	return XMMatrixSet(p.M[0][0], p.M[1][0], p.M[2][0], p.M[3][0],
		p.M[0][1], p.M[1][1], p.M[2][1], p.M[3][1],
		p.M[0][2], p.M[1][2], p.M[2][2], p.M[3][2],
		p.M[0][3], p.M[1][3], p.M[2][3], p.M[3][3]);
}
#endif


// Note: values are used in shader
enum EEyeMode
{
	EEM_NoStereo = -1,	// non VR
	EEM_Eye0 = 0,		// left
	EEM_Eye1 = 1		// right
};

//------------------------------------------------------------------------------
struct float2
{
	float x, y;

	float2(float InX, float InY)
		: x(InX)
		, y(InY)
	{
	}

	float& operator[](int index)
	{
		assert((uint32)index <= 2);
		return ((float*)this)[index];
	}
	const float& operator[](int index) const
	{
		assert((uint32)index <= 2);
		return ((float*)this)[index];
	}

	float2 operator*(const float rhs) const { return float2(x * rhs, y * rhs); }
	float2 operator*(const float2 rhs) const { return float2(x * rhs.x, y * rhs.y); }
	float2 operator/(const float rhs) const { return *this * (1.0f / rhs); }

	// length squared
	float Length2() const
	{
		return x * x + y * y;
	}
};

//------------------------------------------------------------------------------
// 3D Vector; 32 bit floating point components
struct float3
{
	float x;
	float y;
	float z;

	float3() {}
	float3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	float3(float2 xy, float _z) : x(xy.x), y(xy.y), z(_z) {}
	explicit float3(_In_reads_(3) const float *pArray) : x(pArray[0]), y(pArray[1]), z(pArray[2]) {}

	float3& operator=(const float3 rhs) { x = rhs.x; y = rhs.y; z = rhs.z; return *this; }
	float3 operator-() const { return float3(-x, -y, -z); }

	float3 operator*(const float rhs) const { return float3(x * rhs, y * rhs, z * rhs); }
	float3 operator*(const float3 rhs) const { return float3(x * rhs.x, y * rhs.y, z * rhs.z); }
	float3 operator/(const float rhs) const { return *this * (1.0f / rhs); }
	//	float3 operator/(const float3 rhs) const { return float3(x / rhs.x, y / rhs.y, z / rhs.z); }
	float3 operator+(const float3 rhs) const { return float3(x + rhs.x, y + rhs.y, z + rhs.z); }
	float3 operator-(const float3 rhs) const { return float3(x - rhs.x, y - rhs.y, z - rhs.z); }

	float3& operator+=(const float3 rhs) { *this = *this + rhs; return *this; }
	float3& operator-=(const float3 rhs) { *this = *this - rhs; return *this; }
	float3& operator*=(const float rhs) { *this = *this * rhs; return *this; }
	float3& operator*=(const float3 rhs) { *this = *this * rhs; return *this; }
	float3& operator/=(const float rhs) { *this = *this / rhs; return *this; }
	//	float3& operator/=(const float3 rhs) { *this = *this / rhs; return *this; }

	float& operator[](int index)
	{
		assert((uint32)index <= 3);
		return ((float*)this)[index];
	}
	const float& operator[](int index) const
	{
		assert((uint32)index <= 3);
		return ((float*)this)[index];
	}

	// @return 0/1/2 for x/y/z
	int GetSmallestComponent() const
	{
		if (x < y)
		{
			return x < z ? 0 : 2;
		}
		else
		{
			return y < z ? 1 : 2;
		}
	}

	// @return 0/1/2 for x/y/z
	int GetLargestComponent() const
	{
		if (x > y)
		{
			return x > z ? 0 : 2;
		}
		else
		{
			return y > z ? 1 : 2;
		}
	}


	// not normalized
	static float3 ComputeRandomDirection()
	{
		float3 Ret;

		for (;;)
		{
			Ret = float3(frand2(), frand2(), frand2());
			float l2 = Ret.Length2();

			if (l2 > 0.0001f && l2 <= 1.00f)
			{
				break;
			}
		}

		return Ret;
	}

	// normalized
	static float3 ComputeNormalizedRandomDirection();

	void GetOtherBaseVec(float3 &a, float3 &b) const;

	// length squared
	float Length2() const
	{
		return x * x + y * y + z * z;
	}

	XMVECTOR GetXMVECTOR(float w = 0.0f) const { return XMVectorSet(x, y, z, w); }
	XMFLOAT3 GetXMFLOAT3() const { return XMFLOAT3(x, y, z); }

	float2& xy() { return *(float2*)this; }
};

inline float length(float2 a)
{
	return sqrtf(a.Length2());
}

inline float2 normalize(float2 A)
{
	return A / length(A);
}

inline float dot(float2 A, float2 B)
{
	return A.x * B.x + A.y * B.y;
}

inline float SafeInv(float x)
{
	if (fabs(x) > 1.175494351e-38F)
	{
		return 1.0f / x;
	}

	return x > 0 ? FLT_MAX : -FLT_MAX;
}

inline float3 cross(float3 A, float3 B)
{
	return float3(
		A.y * B.z - A.z * B.y,
		A.z * B.x - A.x * B.z,
		A.x * B.y - A.y * B.x
	);
}

inline float dot(float3 A, float3 B)
{
	return A.x * B.x + A.y * B.y + A.z * B.z;
}

inline float length(float3 a)
{
	return sqrtf(a.Length2());
}

inline float3 normalize(float3 A)
{
	return A / length(A);
}

// don't use macro min() max(), use this instead
inline int Max(int x, int y) { return (x > y) ? x : y; }
inline int Min(int x, int y) { return (x < y) ? x : y; }
inline uint32 Max(uint32 x, uint32 y) { return (x > y) ? x : y; }
inline uint32 Min(uint32 x, uint32 y) { return (x < y) ? x : y; }
inline size_t Max(size_t x, size_t y) { return (x > y) ? x : y; }
inline size_t Min(size_t x, size_t y) { return (x < y) ? x : y; }
inline float Max(float x, float y) { return (x > y) ? x : y; }
inline float Min(float x, float y) { return (x < y) ? x : y; }


// ported from HLSL, should be moved in MathUtils
// public domain: https://github.com/elfrank/raytracer-gpupro4/blob/master/RayTracerCS/Shaders/Core/Intersection.hlsl
// Ray-Box Intersection Test
// @return hit is if Ret.y >= 0
inline float2 IntersectBox(float3 RayStart, float3 RayInvDir, float3 BBoxMin, float3 BBoxMax)
{
	float2 Ret(0, 0);
	float3 DiffMax = BBoxMax - RayStart;
	float3 DiffMin = BBoxMin - RayStart;
	DiffMax *= RayInvDir;
	DiffMin *= RayInvDir;

	Ret[0] = Min(DiffMin.x, DiffMax.x);
	Ret[1] = Max(DiffMin.x, DiffMax.x);

	Ret[0] = Max(Ret[0], Min(DiffMin.y, DiffMax.y));
	Ret[1] = Min(Ret[1], Max(DiffMin.y, DiffMax.y));

	Ret[0] = Max(Ret[0], Min(DiffMin.z, DiffMax.z));
	Ret[1] = Min(Ret[1], Max(DiffMin.z, DiffMax.z));

	//empty interval
	if (Ret[0] > Ret[1])
	{
		//		T[1] = -1.0f;			// not really faster but would work with the test we do later (I guess compiler optimizes it out anyway)
		Ret[0] = Ret[1] = -1.0f;
	}

	return Ret;
}

// input vector don't have to be normalized
// can be optimized
inline void float3::GetOtherBaseVec(float3 &a, float3 &b) const
{
	float3 v = normalize(*this);			// normalized input allows stable asserts

	if (v.z < -0.5f || v.z > 0.5f)
	{
		a.x = v.z;
		a.y = v.y;
		a.z = -v.x;
	}
	else
	{
		a.x = v.y;
		a.y = -v.x;
		a.z = v.z;
	}

	b = normalize(cross(*this, a));
	a = normalize(cross(b, *this));

	//	assert(fabs(a*b)<0.01f);
	//	assert(fabs((*this)*a)<0.01f);
	//	assert(fabs((*this)*b)<0.01f);
}

// 4D Vector; 32 bit floating point components
struct float4
{
	float x;
	float y;
	float z;
	float w;

	float4() {}
	float4(float3 p, float _w) : x(p.x), y(p.y), z(p.z), w(_w) {}
	float4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
	float4& operator=(const float4 rhs) { x = rhs.x; y = rhs.y; z = rhs.z; w = rhs.w; return *this; }
	float4 operator-() const { return float4(-x, -y, -z, -w); }

	float4 operator*(const float rhs) const { return float4(x * rhs, y * rhs, z * rhs, w * rhs); }
	float4 operator*(const float4 rhs) const { return float4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w); }
	float4 operator/(const float rhs) const { return *this * (1.0f / rhs); }
	//	float4 operator/(const float3 rhs) const { return float3(x / rhs.x, y / rhs.y, z / rhs.z); }
	float4 operator+(const float4 rhs) const { return float4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
	float4 operator-(const float4 rhs) const { return float4(x - rhs.x, y - rhs.y, z - rhs.z, w + rhs.w); }

	float4& operator+=(const float4 rhs) { *this = *this + rhs; return *this; }
	float4& operator-=(const float4 rhs) { *this = *this - rhs; return *this; }
	float4& operator*=(const float rhs) { *this = *this * rhs; return *this; }
	float4& operator*=(const float4 rhs) { *this = *this * rhs; return *this; }
	float4& operator/=(const float rhs) { *this = *this / rhs; return *this; }
	//	float3& operator/=(const float3 rhs) { *this = *this / rhs; return *this; }

	float& operator[](int index)
	{
		assert((uint32)index <= 4);
		return ((float*)this)[index];
	}
	const float& operator[](int index) const
	{
		assert((uint32)index <= 4);
		return ((float*)this)[index];
	}
};

inline float dot(float4 A, float4 B)
{
	return A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w;
}

class CPlane3D : public float4
{
public:
	// left outside [0]: ComputeDistance(p) > 0
	// right inside [1]: ComputeDistance(p) <= 0

	// .xyz normalized, .w:distance to 0

	CPlane3D(float _x, float _y, float _z, float _w)
		: float4(_x, _y, _z, _w)
	{}

	// @param normal needs to be normalized
	CPlane3D(float3 normal, float3 point)
	{
		assert(std::abs(normal.Length2() - 1.0f) < 0.001f);

		x = normal.x;
		y = normal.y;
		z = normal.z;
		w = -dot(normal, point);
	}

	CPlane3D(float3 a, float3 b, float3 c)
	{
		float3 u = b - a;
		float3 v = c - a;

		// todo: needs to be verified 
		float3 n = cross(u, v);
		float d = -dot(n, a);

		*this = CPlane3D(n.x, n.y, n.z, d);
	}

	// left outside [0]: ComputeDistance(p) > 0
	// right inside [1]: ComputeDistance(p) <= 0
	float ComputeDistance(float3 Pos) const
	{
		return dot(*this, float4(Pos, 1));
	}
};


struct int3
{
	int x;
	int y;
	int z;

	int3() {}
	int3(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}

	int3 operator-() const { return int3(-x, -y, -z); }

	int3& operator+=(const int3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
	int3& operator-=(const int3& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }

	int3 operator+(const int3& rhs) { return int3(x + rhs.x, y + rhs.y, z + rhs.z); }
	int3 operator-(const int3& rhs) { return int3(x - rhs.x, y - rhs.y, z - rhs.z); }

	bool operator==(const int3& rhs) const { return (x == rhs.x) && (y == rhs.y) && (z == rhs.z); }
	bool operator!=(const int3& rhs) const { return !(*this == rhs); }

	int& operator[](int index)
	{
		return ((int*)this)[index];
	}
};

struct float3x3
{
	float m[9];

	void SetIdentity(float Scale = 1.0f)
	{
		SetX(float3(Scale, 0, 0));
		SetY(float3(0, Scale, 0));
		SetZ(float3(0, 0, Scale));
	}

	void SetX(float3 p) { m[0] = p.x; m[1] = p.y; m[2] = p.z; }
	float3 GetX() const { return float3(m[0], m[1], m[2]); }
	void SetY(float3 p) { m[3] = p.x; m[4] = p.y; m[5] = p.z; }
	float3 GetY() const { return float3(m[3], m[4], m[5]); }
	void SetZ(float3 p) { m[6] = p.x; m[7] = p.y; m[8] = p.z; }
	float3 GetZ() const { return float3(m[6], m[7], m[8]); }

	float3 TransformPosition(float3 In) const
	{
		return GetX() * In.x + GetY() * In.y + GetZ() * In.z;
	}

	void Transpose()
	{
		std::swap(m[1], m[3]);
		std::swap(m[2], m[6]);
		std::swap(m[5], m[7]);
	}

	void SetBaseFromZ(float3 Z)
	{
		Z = normalize(Z);

		float3 X, Y;
		Z.GetOtherBaseVec(X, Y);

		SetX(X);
		SetY(Y);
		SetZ(Z);
	}

	XMMATRIX GetXMMATRIX(float3 Pos) const
	{
		// like XMMatrixTranslation
		return XMMatrixSet(
			m[0], m[1], m[2], 0,
			m[3], m[4], m[5], 0,
			m[6], m[7], m[8], 0,
			Pos.x, Pos.y, Pos.z, 1);
	}
};

// Note: W is dropped
inline float3 ToFloat3(const XMVECTOR &In)
{
	return float3(XMVectorGetX(In), XMVectorGetY(In), XMVectorGetZ(In));
}

#if G_OCULUS_VR
inline float3 ToFloat3(const ovrVector3f &In)
{
	return float3(In.x, In.y, In.z);
}
#endif

class float4x3 : public float3x3
{
public:
	float3 Pos;

	float4x3()
		: Pos(0, 0, 0)
	{
	}

	void SetFrom(const XMMATRIX &In)
	{
		XMFLOAT4X3 temp;

		XMStoreFloat4x3(&temp, In);

		SetX(float3(temp.m[0]));
		SetY(float3(temp.m[1]));
		SetZ(float3(temp.m[2]));
		Pos = float3(temp.m[3]);
	}

	// todo: move
	float3 TransformPosition(float3 In) const
	{
		float3x3 base = *this;

		return base.TransformPosition(In) + Pos;
	}

	// todo: move
	float3 InvTransformPosition(float3 In) const
	{
		float3 Local = In - Pos;

		float3x3 base = *this;

		return base.TransformPosition(In);
	}

	XMMATRIX GetXMMATRIX() const
	{
		// like XMMatrixTranslation
		return XMMatrixSet(
			m[0], m[1], m[2], 0,
			m[3], m[4], m[5], 0,
			m[6], m[7], m[8], 0,
			Pos.x, Pos.y, Pos.z, 1);
	}
};

template <class T>
inline T clamp(T Value, T MinValue, T MaxValue)
{
	if (Value < MinValue)
	{
		Value = MinValue;
	}

	if (Value > MaxValue)
	{
		Value = MaxValue;
	}

	return Value;
}

inline float asfloat(uint32 x)
{
	float* p = (float*)&x;
	return *p;
}

inline uint32 asuint(float x)
{
	uint32* p = (uint32*)&x;
	return *p;
}

#include "DirectXPackedVector.h"

inline float Pack2HalfFloats(float x, float y)
{
	uint32 hx = PackedVector::XMConvertFloatToHalf(x);
	uint32 hy = PackedVector::XMConvertFloatToHalf(y);

	return asfloat((hx << 16) | hy);
}

inline float ShortsToFloat(uint16 x, uint16 y)
{
	uint32 dx = x;
	uint32 dy = y;
	return asfloat((dx << 16) | dy);
}



inline float3 Min(const float3& a, const float3& b)
{
	return float3(Min(a.x, b.x), Min(a.y, b.y), Min(a.z, b.z));
}

inline float3 Max(const float3& a, const float3& b)
{
	return float3(Max(a.x, b.x), Max(a.y, b.y), Max(a.z, b.z));
}

struct CFloatRange
{
	float Min;
	float Max;

	CFloatRange(float InMin, float InMax)
		: Min(InMin)
		, Max(InMax)
	{
	}
};

struct CFloatRect
{
	float2 Min;
	float2 Max;

	CFloatRect(float2 InMin, float2 InMax)
		: Min(InMin)
		, Max(InMax)
	{
	}
	CFloatRect(float InMinX, float InMinY, float InMaxX, float InMaxY)
		: Min(InMinX, InMinY)
		, Max(InMaxX, InMaxY)
	{
	}
};


struct FFloat3Int1
{
	// default constructor (empty for best performance)
	FFloat3Int1()
	{
	}

	FFloat3Int1(float Inx, float Iny, float Inz, int32 Inw)
		:x(Inx), y(Iny), z(Inz), w(Inw)
	{
	}

	FFloat3Int1(float3 In, int32 Inw)
		:x(In.x), y(In.y), z(In.z), w(Inw)
	{
	}

	XMFLOAT3 GetXMFLOAT3() const
	{
		return XMFLOAT3(x, y, z);
	}

	float x, y, z; int32 w;
};


/*
inline COLORREF IndexToColor(uint32 Index)
{
	uint32 a = (Index & (1 << 0)) != 0;
	uint32 d = (Index & (1 << 1)) != 0;
	uint32 g = (Index & (1 << 2)) != 0;

	uint32 b = (Index & (1 << 3)) != 0;
	uint32 e = (Index & (1 << 4)) != 0;
	uint32 h = (Index & (1 << 5)) != 0;

	uint32 c = (Index & (1 << 6)) != 0;
	uint32 f = (Index & (1 << 7)) != 0;
	uint32 i = (Index & (1 << 8)) != 0;

	uint32 R = ((a * 4 + b * 2 + c) * 255) / 7;
	uint32 G = ((d * 4 + e * 2 + f) * 255) / 7;
	uint32 B = ((g * 4 + h * 2 + i) * 255) / 7;

	return RGB(R, G, B);
}
*/

// 0: no key
// 38: up
// 40: down
typedef int32 CKey;


// like HLSL
typedef unsigned int uint;

// like HLSL
struct uint2
{
	uint2()
	{
	}

	uint2(int32 InX, int32 InY)
		: x(InX), y(InY)
	{
	}

	uint32 x;
	uint32 y;
	uint32& operator[](int i)
	{
		return i ? y : x;
	}
};

// like HLSL
struct int2
{
	int2()
	{
	}

	int2(int32 InX, int32 InY)
		: x(InX), y(InY)
	{
	}

	int32 x;
	int32 y;
	int32& operator[](int i)
	{
		return i ? y : x;
	}
};

