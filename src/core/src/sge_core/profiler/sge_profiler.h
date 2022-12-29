#pragma once

#include <tracy/Tracy.hpp>
#include <common/TracySystem.hpp>

#define SGE_PROFILE_SCOPED ZoneScoped
#define SGE_PROFILE_FRAME FrameMark
#define SGE_PROFILE_SECTION(name) ZoneScopedN(name)
#define SGE_PROFILE_TAG(str) ZoneText(str, ::strlen(str))
#define SGE_PROFILE_LOG(text) TracyMessage(text, ::strlen(text))
#define SGE_PROFILE_VALUE(text, value) TracyPlot(text, value)

#define SGE_PROFILE_ALLOC(p, size) TracyCAllocS(p, size, 12)
#define SGE_PROFILE_FREE TracyCFreeS(p, 12)

#define SGE_PROFILE_SET_THREAD_NAME(name) tracy::SetThreadName(name)
