//
//// -- PCH -- //
#include "mfpch.h"
//
//// -- .h -- //
//#include "SceneManager.h"
//
//// -- Application Includes ---------//
//
//
//// -- Graphics Includes --------------------- //
//#include "Magnefu/Graphics/ResourceManagement/ResourcePaths.h"
//
//// -- Core Includes ---------------------------------- //
//
//
//
//
//namespace Magnefu
//{
//	SceneManager::SceneManager()
//	{
//		// Number to be determined by saved scene files
//		// Default reservation of 1
//		// TODO: Get this info from glTF file
//		m_Scenes.set_capacity(SCENE_COUNT);
//	}
//
//	SceneManager::~SceneManager()
//	{
//		
//	}
//
//	SceneManager* SceneManager::Create()
//	{
//		return new SceneManager();
//	}
//
//    Scene* SceneManager::CreateScene(cstring name)
//    {
//        //TODO: ERROR HANDLING to ensure name is not in use
//		m_SceneNames.emplace(name);
//		m_Scenes.push(Scene::Create(name));
//		return m_Scenes.back();
//    }
//
//	bool SceneManager::NameExists(cstring newName)
//	{
//		// O(1) Time
//		return m_SceneNames.find(newName) != m_SceneNames.end();
//
//		// O(N) Time
//		/*for (auto& scene : m_Scenes)
//		{
//			if (scene->GetName() == newName)
//			{
//				return true;
//			}
//		}
//		return false;*/
//	}
//}