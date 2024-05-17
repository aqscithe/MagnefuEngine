#include "xxhash/xxhash.h"


// xxhash_config.h
#define XXH_INLINE_ALL
#define XXH_FORCE_ALIGN_CHECK

#define SAFE_XXH_MEMORY_ACCESS

#if defined(SAFE_XXH_MEMORY_ACCESS)
    #define XXH_FORCE_MEMORY_ACCESS 0 // Max portability and safetey
#elif defined(MF_ARCH_X64)
    #define XXH_FORCE_MEMORY_ACCESS 2 // Potentially improved performance on architectures where unaligned memory access is supported and does not incur a performance penalty (e.g., x86, x64).
#elif defined(MF_ARCH_ARM)
    #define XXH_FORCE_MEMORY_ACCESS 1 // GCC or Clang with support for packed attributes and if testing shows performance improvement.
#endif

#if defined(MF_ARCH_ARM)
    #define XXH_VECTOR XXH_NEON // For ARM with NEON support
#endif
