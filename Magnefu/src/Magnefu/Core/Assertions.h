#pragma once

#include "Log.h"

#if defined(MF_PLATFORM_WINDOWS)

#elif defined(MF_PLATFORM_MACOS)

#else
	#error Magnefu only supports Windows and MacOS for now...
#endif

#define BIT(x) (1 << x)

#ifdef MF_DEBUG
	#define MF_ENABLE_ASSERTS
#endif

#ifdef MF_ENABLE_ASSERTS
	#define MF_ASSERT(x, ...) { if(!x) {MF_APP_ERROR("Assertion Failed: {0}\n\t {1} {2}", __VA_ARGS__, __FILE__, __LINE__); __debugbreak();}}
	#define MF_CORE_ASSERT(x, ...) { if(!(x)) {MF_CORE_ERROR("Assertion Failed: {0}\n\t {1} {2}", __VA_ARGS__, __FILE__, __LINE__); __debugbreak();}}
#else
	#define MF_ASSERT(x, ...)
	#define MF_CORE_ASSERT(x, ...)
#endif
