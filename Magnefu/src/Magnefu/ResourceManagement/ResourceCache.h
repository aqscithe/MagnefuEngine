#pragma once

#include "CacheableResource.h"
#include "Magnefu/Renderer/Material.h"
#include "Magnefu/Renderer/Shader.h"
#include "Magnefu/Renderer/Texture.h"

#include <typeindex>
#include <memory>
#include <future>
#include <xhash>

#include "glm/glm.hpp"
#include "glm/gtx/hash.hpp"


//template <>
//struct std::hash<Magnefu::TextureProps> {
//	size_t operator()(const Magnefu::TextureProps& props) {
//		return 0; 
//	}
//};


namespace Magnefu
{
	

	class ResourceCache
	{
		using CacheState = std::unordered_map<std::type_index, std::unordered_map<size_t, Ref<CacheableResource>>>;
		using MutexMap = std::unordered_map<std::type_index, std::mutex>;


	public:
		ResourceCache() = default;
		~ResourceCache() = default;

		ResourceCache(const ResourceCache&) = delete;
		ResourceCache& operator=(const ResourceCache&) = delete;

		ResourceCache(ResourceCache&&) = delete;
		ResourceCache& operator=(ResourceCache&&) = delete;

	public:
		template <typename T, typename... Args>
		Ref<T> RequestResource(Args&& ...args)
		{
			static_assert(std::is_base_of<CacheableResource, T>::value, "Class must be derived from CacheableResource");

			auto& mutex = m_Mutexes[typeid(T)];
			std::lock_guard<std::mutex> guard(mutex);

			size_t seed{ 0 };

			HashArgs(seed, args...);

			// get Hash Map for Resource Type
			auto& resourceHashMap = m_State[typeid(T)];
			auto resource_it = resourceHashMap.find(seed);

			// if we find the resource, return it
			if (resource_it != resourceHashMap.end())
				return std::dynamic_pointer_cast<T>(resource_it->second);
			
			// otherwise create the resource
			auto newResource = CreateRef<T>(args...);

			// Move resource to hash map
			resourceHashMap.emplace(seed, std::move(newResource));

			return std::dynamic_pointer_cast<T>(resourceHashMap[seed]);
		}

		template<typename T, typename...Args>
		Ref<T> Get(const Args&...args)
		{
			static_assert(std::is_base_of<CacheableResource, T>::value, "Class must be derived from CacheableResource");

			auto& mutex = m_Mutexes[typeid(T)];
			std::lock_guard<std::mutex> guard(mutex);

			size_t seed{ 0 };

			HashArgs(seed, args...);

			auto& resourceHashMap = m_State[typeid(T)];
			auto resource_it = resourceHashMap.find(seed);

			if (resource_it != resourceHashMap.end())
				return std::dynamic_pointer_cast<T>(resource_it->second);

			MF_CORE_ASSERT(false, "RESOURCE NOT FOUND");
			return nullptr;
		}

		template <typename T>
		size_t size()
		{
			static_assert(std::is_base_of<CacheableResource, T>::value, "Class must be derived from CacheableResource");
			return m_State[typeid(T)].size();
		}

		static ResourceCache* Create();

	private:

		template <typename T, typename... A>
		inline void HashArgs(size_t& seed, const T& firstArg, const A&...otherArgs)
		{
			HashArgs(seed, firstArg);

			HashArgs(seed, otherArgs...);
		}

		template <typename T>
		inline void HashArgs(size_t& seed, const T& arg)
		{
			std::hash<T> hasher;
			glm::detail::hash_combine(seed, hasher(arg));
		}

		
	private:
		CacheState m_State;
		MutexMap m_Mutexes;
	};
}

