#ifndef HLSL_COMMON
#define HLSL_COMMON

// todo: move
// 0:off / 1:on
#define ANTIALIASING 1

#define REFLECTION_HELPER_MODE 3 // 3: implement HLSL
#include "../../ReflectionHelper.h" // set defines for next line, undefine REFLECTION_HELPER_MODE
#include "../../DataDefinitions.h" // your types go here
#include "../../VertexDefinitions.h" // your types go here
#include "../../ReflectionHelper.h" // undefine all


#endif // HLSL_COMMON