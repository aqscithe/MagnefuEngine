#pragma once

// -- Application Includes ---------//
#include "Scene.h"

// -- Graphics Includes --------------------- //


// -- Core Includes ---------------------------------- //
#include "Magnefu/Core/SmartPointers.h"



namespace Magnefu
{
	class SceneManager
	{
	public:
		SceneManager();
		~SceneManager();

		SceneManager(const SceneManager&) = delete;
		SceneManager& operator=(const SceneManager&) = delete;

		SceneManager(const SceneManager&&) = delete;
		SceneManager& operator=(const SceneManager&&) = delete;

		static SceneManager* Create();


		bool NameExists(cstring newName);
		Scene* CreateScene(cstring name);
		inline Array<Scene*>& GetScenes() { return m_Scenes; }

	private:

		Array<Scene*> m_Scenes;
		std::unordered_set<cstring> m_SceneNames;
	};
}
