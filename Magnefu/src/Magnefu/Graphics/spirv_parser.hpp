#pragma once

#include "GPUResources.hpp"


#if defined(_MSC_VER)
	#include <spirv-headers/spirv.h>
#else 
	#include <spirv_cross/spirv.h>
#endif

namespace Magnefu
{
	struct StringBuffer;


	namespace spirv
	{
		static const u32 k_max_count = 8;

		struct ParseResult
		{
			u32								set_count;
			u32                         push_constants_stride = 0;

			DescriptorSetLayoutCreation		sets[k_max_count];

			ComputeLocalSize            compute_local_size;
		};

		void	parse_binary(const u32* data, sizet data_size, StringBuffer& name_buffer, ParseResult* parse_result);

	} // namespace spirv

} // namespace Magnefu