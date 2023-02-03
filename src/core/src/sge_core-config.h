#pragma once
#define SGE_MATH_USE_SSE 1

#define SGE_ENABLE_PROFILER 1

#if _DEBUG

#define SGE_BUILD_CONFIG Debug
#define SGE_BUILD_CONFIG_STR "Debug"

#else

#define SGE_BUILD_CONFIG Release
#define SGE_BUILD_CONFIG_STR "Release"

#endif

