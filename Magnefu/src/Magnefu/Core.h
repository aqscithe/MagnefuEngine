#pragma once

#ifdef MF_PLATFORM_WINDOWS
#else
	#error Magnefu only supports Windows...for now

#endif

#define BIT(x) (1 << x)

#ifdef MF_DEBUG
	#define MF_ENABLE_ASSERTS
#endif

#ifdef MF_ENABLE_ASSERTS
	#define MF_ASSERT(x, ...) { if(!x) {MF_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak();}}
	#define MF_CORE_ASSERT(x, ...) { if(!x) {MF_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak();}}
#else
	#define MF_ASSERT(x, ...)
	#define MF_CORE_ASSERT(x, ...)
#endif