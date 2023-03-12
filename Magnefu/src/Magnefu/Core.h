#pragma once

#ifdef MF_PLATFORM_WINDOWS
	#ifdef MF_BUILD_DLL
		#define MAGNEFU_API __declspec(dllexport)
	#else
		#define MAGNEFU_API __declspec(dllimport)
	#endif
#else
	#error Magnefu only supports Windows...for now

#endif

#define BIT(x) (1 << x)