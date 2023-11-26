#pragma once

#include "Magnefu/Core/SmartPointers.h"
#include "Magnefu/Scene/Scene.h"


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


		bool NameExists(std::string& newName);
		Scene* CreateScene(std::string& name);
		inline std::vector<Scope<Scene>>& GetScenes() { return m_Scenes; }

	private:

		std::vector<Scope<Scene>> m_Scenes;
		std::unordered_set<std::string> m_SceneNames;
	};
}
