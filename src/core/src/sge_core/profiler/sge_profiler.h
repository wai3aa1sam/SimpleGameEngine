#pragma once

#include <tracy/Tracy.hpp>
#include <common/TracySystem.hpp>

#if SGE_ENABLE_PROFILER

#define SGE_PROFILE_SCOPED ZoneScoped
#define SGE_PROFILE_FRAME FrameMark
#define SGE_PROFILE_SECTION(name) ZoneScopedN(name)
#define SGE_PROFILE_TAG(str) ZoneText(str, ::strlen(str))
#define SGE_PROFILE_LOG(text) TracyMessage(text, ::strlen(text))
#define SGE_PROFILE_VALUE(text, value) TracyPlot(text, value)

#define SGE_PROFILE_ALLOC(p, size) TracyCAllocS(p, size, 12)
#define SGE_PROFILE_FREE TracyCFreeS(p, 12)

#define SGE_PROFILE_SET_THREAD_NAME(name) tracy::SetThreadName(name)

#else

#define SGE_PROFILE_SCOPED 
#define SGE_PROFILE_FRAME 
#define SGE_PROFILE_SECTION(name) 
#define SGE_PROFILE_TAG(str) 
#define SGE_PROFILE_LOG(text) 
#define SGE_PROFILE_VALUE(text, value) 

#define SGE_PROFILE_ALLOC(p, size) 
#define SGE_PROFILE_FREE 

#define SGE_PROFILE_SET_THREAD_NAME(name) 

#endif