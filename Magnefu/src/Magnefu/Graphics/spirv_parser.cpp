#include "mfpch.h"

#include "spirv_parser.hpp"

#include "Magnefu/Core/Numerics.hpp"

namespace Magnefu
{

	namespace spirv
	{

		static const u32		k_bindless_texture_binding = 10;

		struct Member
		{
			u32			id_index;
			u32			offset;

			StringView	name;
		};

		struct Id
		{
			SpvOp			op;
			u32				set;
			u32				binding;

			// for ints and floats
			u8				width;
			u8				sign;

			// for arrays, vectors, and matrices
			u32				type_index;
			u32				count;

			// for variables
			SpvStorageClass storage_class;

			// for constants
			u32				value;

			// for structs
			StringView		name;
			Array<Member>	members;

		};

		VkShaderStageFlags	parse_execution_model(SpvExecutionModel model)
		{
			switch (model)
			{

				case SpvExecutionModelVertex:
					return VK_SHADER_STAGE_VERTEX_BIT;

				case SpvExecutionModelTessellationControl:
					return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

				case SpvExecutionModelTessellationEvaluation:
					return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

				case SpvExecutionModelGeometry:
					return VK_SHADER_STAGE_GEOMETRY_BIT;

				case SpvExecutionModelFragment:
					return VK_SHADER_STAGE_FRAGMENT_BIT;

				case SpvExecutionModelMeshNV:
					return VK_SHADER_STAGE_MESH_BIT_NV;

				case SpvExecutionModelMeshEXT:
					return VK_SHADER_STAGE_MESH_BIT_EXT;

				case SpvExecutionModelTaskNV:
					return VK_SHADER_STAGE_TASK_BIT_NV;

				case SpvExecutionModelTaskEXT:
					return VK_SHADER_STAGE_TASK_BIT_EXT;

			}

			return 0;
		}



		void	parse_binary(u32* data, sizet data_size, StringBuffer& name_buffer, ParseResult* parse_result)
		{
			MF_CORE_ASSERT(data_size % 4 == 0, "Data size not divisible by 4");

			u32 spv_word_count = safe_cast<u32>(data_size / 4);

			u32 magic_number = data[0];
			MF_CORE_ASSERT(magic_number == 0x07230203, "");

			u32 id_bound = data[3];

			Allocator* allocator = &MemoryService::Instance()->systemAllocator;
			Array<Id> ids;
			ids.init(allocator, id_bound, id_bound);

			memset(ids.begin(), 0, id_bound * sizeof(Id));

			VkShaderStageFlags stage;

			sizet word_index = 5;
			while (word_index < spv_word_count)
			{
				SpvOp op = (SpvOp)(data[word_index] & 0xFF);
				u16 word_count = (u16)(data[word_index] >> 16);

				switch (op)
				{
					case SpvOpEntryPoint:
					{
						MF_CORE_ASSERT(word_count >= 4, "");

						SpvExecutionModel model = (SpvExecutionModel)data[word_index + 1];

						stage = parse_execution_model(model);
						MF_CORE_ASSERT(stage != 0, "");

						break;
					}

					case SpvOpDecorate:
					{
						MF_CORE_ASSERT(word_count >= 3, "");

						u32 id_index = data[word_index + 1];
						MF_CORE_ASSERT(id_index < id_bound, "");

						Id& id = ids[id_index];

						SpvDecoration decoration = (SpvDecoration)data[word_index + 2];
						switch (decoration)
						{
							case (SpvDecorationBinding):
							{
								id.binding = data[word_index + 3];
								break;
							}

							case (SpvDecorationDescriptorSet):
							{
								id.set = data[word_index + 3];
								break;
							}
						}

						break;
					}

					case SpvOpMemberDecorate:
					{
						MF_CORE_ASSERT(word_count >= 4, "");

						u32 id_index = data[word_index + 1];
						MF_CORE_ASSERT(id_index < id_bound, "");

						Id& id = ids[id_index];

						u32 member_index = data[word_index + 2];

						if (id.members.capacity == 0) {
							id.members.init(allocator, 64, 64);
						}

						Member& member = id.members[member_index];

						SpvDecoration decoration = (SpvDecoration)data[word_index + 3];
						switch (decoration)
						{
							case (SpvDecorationOffset):
							{
								member.offset = data[word_index + 4];
								break;
							}
						}

						break;
					}

					case SpvOpName:
					{
						MF_CORE_ASSERT(word_count >= 3, "");

						u32 id_index = data[word_index + 1];
						MF_CORE_ASSERT(id_index < id_bound, "");

						Id& id = ids[id_index];

						char* name = (char*)(data + (word_index + 2));
						char* name_view = name_buffer.append_use(name);

						id.name.text = name_view;
						id.name.length = strlen(name_view);

						break;
					}

					case SpvOpMemberName:
					{
						MF_CORE_ASSERT(word_count >= 4, "");

						u32 id_index = data[word_index + 1];
						MF_CORE_ASSERT(id_index < id_bound, "");

						Id& id = ids[id_index];

						u32 member_index = data[word_index + 2];

						if (id.members.capacity == 0) {
							id.members.init(allocator, 64, 64);
						}

						Member& member = id.members[member_index];

						char* name = (char*)(data + (word_index + 3));
						char* name_view = name_buffer.append_use(name);
						
						member.name.text = name_view;
						member.name.length = strlen(name_view);

						break;

					}

					case SpvOpTypeInt:
					{
						MF_CORE_ASSERT(word_count == 4, "");

						u32 id_index = data[word_index + 1];
						MF_CORE_ASSERT(id_index < id_bound, "");

						Id& id = ids[id_index];
						id.op = op;
						id.width = (u8)data[word_index + 2];
						id.sign = (u8)data[word_index + 3];

						break;
					}

					case SpvOpTypeFloat:
					{
						MF_CORE_ASSERT(word_count == 3, "");

						u32 id_index = data[word_index + 1];
						MF_CORE_ASSERT(id_index < id_bound, "");

						Id& id = ids[id_index];
						id.op = op;
						id.width = (u8)data[word_index + 2];

						break;
					}

					case SpvOpTypeVector:
					{
						MF_CORE_ASSERT(word_count == 4, "");

						u32 id_index = data[word_index + 1];
						MF_CORE_ASSERT(id_index < id_bound, "");

						Id& id = ids[id_index];
						id.op = op;
						id.type_index = data[word_index + 2];
						id.count = data[word_index + 3];

						break;
					}

					case SpvOpTypeMatrix:
					{
						MF_CORE_ASSERT(word_count == 4, "");

						u32 id_index = data[word_index + 1];
						MF_CORE_ASSERT(id_index < id_bound, "");

						Id& id = ids[id_index];
						id.op = op;
						id.type_index = data[word_index + 2];
						id.count = data[word_index + 3];

						break;
					}

					case SpvOpTypeImage:
					{
						// NOTE(marco): not sure we need this information just yet
						MF_CORE_ASSERT(word_count >= 9, "");
						// not sure 9 is the correct value...
						break;
					}
					

					case SpvOpTypeSampler:
					{
						MF_CORE_ASSERT(word_count == 2, "");

						u32 id_index = data[word_index + 1];
						MF_CORE_ASSERT(id_index < id_bound, "");

						Id& id = ids[id_index];
						id.op = op;

						break;
					}

					case SpvOpTypeSampledImage:
					{
						MF_CORE_ASSERT(word_count == 3, "");

						u32 id_index = data[word_index + 1];
						MF_CORE_ASSERT(id_index < id_bound, "");

						Id& id = ids[id_index];
						id.op = op;

						break;
					}

					case SpvOpTypeArray:
					{
						MF_CORE_ASSERT(word_count == 4, "");

						u32 id_index = data[word_index + 1];
						MF_CORE_ASSERT(id_index < id_bound, "");

						Id& id = ids[id_index];
						id.op = op;
						id.type_index = data[word_index + 2];
						id.count = data[word_index + 3];

						break;
					}

					case SpvOpTypeRuntimeArray:
					{
						MF_CORE_ASSERT(word_count == 3, "");

						u32 id_index = data[word_index + 1];
						MF_CORE_ASSERT(id_index < id_bound, "");

						Id& id = ids[id_index];
						id.op = op;
						id.type_index = data[word_index + 2];

						break;

					}

					case SpvOpTypeStruct:
					{
						MF_CORE_ASSERT(word_count >= 2, "");

						u32 id_index = data[word_index + 1];
						MF_CORE_ASSERT(id_index < id_bound, "");

						Id& id = ids[id_index];
						id.op = op;

						if (word_count > 2) {
							for (u16 member_index = 0; member_index < word_count - 2; ++member_index) {
								id.members[member_index].id_index = data[word_index + member_index + 2];
							}
						}

						break;
					}

					case SpvOpTypePointer:
					{
						MF_CORE_ASSERT(word_count == 4, "");

						u32 id_index = data[word_index + 1];
						MF_CORE_ASSERT(id_index < id_bound, "");

						Id& id = ids[id_index];
						id.op = op;
						id.type_index = data[word_index + 3];

						break;
					}

					case SpvOpConstant:
					{
						MF_CORE_ASSERT(word_count >= 4, "");

						u32 id_index = data[word_index + 1];
						MF_CORE_ASSERT(id_index < id_bound, "");

						Id& id = ids[id_index];
						id.op = op;
						id.type_index = data[word_index + 2];
						id.value = data[word_index + 3]; // NOTE(marco): we assume all constants to have maximum 32bit width

						break;
					}

					case SpvOpVariable:
					{
						MF_CORE_ASSERT(word_count >= 4, "");

						u32 id_index = data[word_index + 2];
						MF_CORE_ASSERT(id_index < id_bound, "");

						Id& id = ids[id_index];
						id.op = op;
						id.type_index = data[word_index + 1];
						id.storage_class = (SpvStorageClass)data[word_index + 3];

						break;
					}

				}

				word_index += word_count;
			}

			for (u32 id_index = 0; id_index < ids.size; id_index++)
			{
				Id& id = ids[id_index];

				if (id.op == SpvOpVariable)
				{
					switch (id.storage_class)
					{

						case SpvStorageClassUniform:
						case SpvStorageClassUniformConstant:
						{
							if (id.set == 1 && (id.binding == k_bindless_texture_binding || id.binding == (k_bindless_texture_binding + 1))) 
							{
								// NOTE(marco): these are managed by the GPU device
								continue;
							}

							// NOTE(marco): get actual type
							Id& uniform_type = ids[ids[id.type_index].type_index];

							DescriptorSetLayoutCreation& setLayout = parse_result->sets[id.set];
							setLayout.set_set_index(id.set);

							DescriptorSetLayoutCreation::Binding binding{ };
							binding.start = id.binding;
							binding.count = 1;

							switch (uniform_type.op)
							{
								case SpvOpTypeStruct:
								{
									binding.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
									binding.name = uniform_type.name.text;
									break;
								}

								case SpvOpTypeSampledImage:
								{
									binding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
									binding.name = id.name.text;
									break;
								}
							}

							setLayout.add_binding_at_index(binding, id.binding);

							parse_result->set_count = max(parse_result->set_count, (id.set + 1));

							break;
						}
					
					}
				}
			}

		}
	}
}


