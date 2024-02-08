#pragma once

#include <math.h>
//#include <cmath>
#include <assert.h>

//#include <directxmath.h>
//using namespace DirectX;

#include "external/glm/glm.hpp"
#include "external/glm/gtc/quaternion.hpp"


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

inline float ConvertToRadians(float degrees)
{
    return degrees / 180 * PI;
}


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



// ported from HLSL, should be moved in MathUtils
// public domain: https://github.com/elfrank/raytracer-gpupro4/blob/master/RayTracerCS/Shaders/Core/Intersection.hlsl
// Ray-Box Intersection Test
// @return hit is if Ret.y >= 0
inline glm::vec2 IntersectBox(glm::vec3 RayStart, glm::vec3 RayInvDir, glm::vec3 BBoxMin, glm::vec3 BBoxMax)
{
    glm::vec2 Ret(0, 0);
    glm::vec3 DiffMax = BBoxMax - RayStart;
    glm::vec3 DiffMin = BBoxMin - RayStart;
	DiffMax *= RayInvDir;
	DiffMin *= RayInvDir;

	Ret[0] = glm::min(DiffMin.x, DiffMax.x);
	Ret[1] = glm::max(DiffMin.x, DiffMax.x);

	Ret[0] = glm::max(Ret[0], glm::min(DiffMin.y, DiffMax.y));
	Ret[1] = glm::min(Ret[1], glm::max(DiffMin.y, DiffMax.y));

	Ret[0] = glm::max(Ret[0], glm::min(DiffMin.z, DiffMax.z));
	Ret[1] = glm::min(Ret[1], glm::max(DiffMin.z, DiffMax.z));

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
inline void GetOtherBaseVec(glm::vec3& outU, glm::vec3& outV, const glm::vec3& n)
{
    // normalized input allows stable asserts
    glm::vec3 v = glm::normalize(n);

	if (v.z < -0.5f || v.z > 0.5f)
	{
		outU.x = v.z;
        outU.y = v.y;
        outU.z = -v.x;
	}
	else
	{
        outU.x = v.y;
        outU.y = -v.x;
        outU.z = v.z;
	}

    outV = glm::normalize(cross(v, outU));
    outU = glm::normalize(cross(outV, v));

	//	assert(fabs(a*b)<0.01f);
	//	assert(fabs((*this)*a)<0.01f);
	//	assert(fabs((*this)*b)<0.01f);
}



// 0: no key
// 38: up
// 40: down
typedef int32 CKey;


// like HLSL
typedef unsigned int uint;

