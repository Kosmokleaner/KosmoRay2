// ReflectionHelper.h, single header implementation
// 
// 1/19/2025
// by Martin Mittring
//
// #define REFLECTION_HELPER_MODE 
//    : undefine all defines
//	1 : declare types in C++
//	2 : implement reflection functions in one .cpp file
//	3 : declare types in HLSL (todo)
//
// reflection API:
// * const char* getTypeName<type>() 

#ifndef REFLECTION_HELPER_H
#define REFLECTION_HELPER_H
#ifdef __cplusplus
	struct IReflection
	{
		virtual void struct_begin(const char* name) = 0;
		virtual void struct_entry(const char* type, const char* name) = 0;
		virtual void struct_end() = 0;
		virtual void enum_begin(const char* name) = 0;
		virtual void enum_entry(const char* name) = 0;
		virtual void enum_end() = 0;
	};
	// HLSL to glm types
	typedef uint uint32;
	typedef glm::ivec2 int2;
	typedef glm::ivec3 int3;
	typedef glm::ivec4 int4;
	typedef glm::uvec2 uint2;
	typedef glm::uvec3 uint3;
	typedef glm::uvec4 uint4;
	typedef glm::vec2 float2;
	typedef glm::vec3 float3;
	typedef glm::vec4 float4;
	typedef glm::mat4 float4x4;
#endif // __cplusplus
#endif // REFLECTION_HELPER_H

#ifndef REFLECTION_HELPER_MODE
//#pragma message("REFLECTION_HELPER_MODE")

// undefine all defines used by this header except REFLECTION_HELPER_MODE
#undef STRUCT_BEGIN
#undef STRUCT_ENTRY
#undef STRUCT_END

#undef ENUM_BEGIN
#undef ENUM_ENTRY
#undef ENUM_END


#elif REFLECTION_HELPER_MODE == 1 // generate C/C++ header ********************
//#pragma message("REFLECTION_HELPER_MODE == 1")

#define STRUCT_BEGIN(name) struct name { \
	static void Reflection(IReflection& r);

#define STRUCT_ENTRY(type, name) type name;
#define STRUCT_END() };
#define CPP_STRUCT_ENTRY(type, name) STRUCT_ENTRY(type, name)

#define ENUM_BEGIN(name) struct name { \
	static void Reflection(IReflection& r);\
	enum Enum {

#define ENUM_ENTRY(name) name,

#define ENUM_END() }; };

template <class T> const char* getTypeName();

#elif REFLECTION_HELPER_MODE == 2 // implement reflection functions in one .cpp file ********************
//#pragma message("REFLECTION_HELPER_MODE == 2")

#define STRUCT_BEGIN(name) template <> const char* getTypeName<struct name>() { return #name; } \
	void name::Reflection(IReflection& r) { r.struct_begin(#name);

#define STRUCT_ENTRY(type, name) \
	r.struct_entry(#type, #name);

#define CPP_STRUCT_ENTRY(type, name) STRUCT_ENTRY(type, name)

#define STRUCT_END() \
	r.struct_end(); }

#define ENUM_BEGIN(name) void name::Reflection(IReflection& r) { r.enum_begin(#name);

#define ENUM_ENTRY(name) r.enum_entry(#name);

#define ENUM_END() r.enum_end(); }


#elif REFLECTION_HELPER_MODE == 3 // declare types in HLSL ********************

#define STRUCT_BEGIN(name) struct name {
#define STRUCT_ENTRY(type, name) type name;
#define CPP_STRUCT_ENTRY(type, name)
#define STRUCT_END() };
#define ENUM_BEGIN(name)
#define ENUM_ENTRY(name)
#define ENUM_END()


#else // unknown REFLECTION_HELPER_MODE ********************
	error

#endif // REFLECTION_HELPER_MODE


#undef REFLECTION_HELPER_MODE

