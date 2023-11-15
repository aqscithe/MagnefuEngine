#pragma once
#include "entt.hpp"	

namespace Magnefu
{

	class Scene 
	{
		public:

			Scene();
			~Scene();

		private:

			entt::registry m_Registry;
	};
}