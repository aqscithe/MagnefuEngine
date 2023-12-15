#pragma once


#include "Magnefu/Application.h"
#include "Magnefu/ImGui/ImGuiLayer.h"
#include "Magnefu/Layer.h"

//----Core--------------------------------
#include "Magnefu/Core/Assertions.h"
#include "Magnefu/Core/Log.h"
#include "Magnefu/Core/TimeStep.h"
#include "Magnefu/Core/SmartPointers.h"
#include "Magnefu/Core/Array.h"
//----------------------------------------

//----Maths-------------------------------

//----------------------------------------

//----Input-------------------------------
#include "Magnefu/Core/Events/Input.h"
#include "Magnefu/Core/Events/KeyCodes.h"
#include "Magnefu/Core/Events/MouseButtonCodes.h"
//----------------------------------------


//----Renderer----------------------------
#include "Magnefu/Renderer/RenderConstants.h"
#include "Magnefu/Renderer/RenderCommand.h"
#include "Magnefu/Renderer/Renderer.h"
#include "Magnefu/Renderer/Light.h"
#include "Magnefu/Renderer/Material.h"
#include "Magnefu/Renderer/Buffer.h"
#include "Magnefu/Renderer/Shader.h"
#include "Magnefu/Renderer/Texture.h"
#include "Magnefu/Renderer/SceneCamera.h"
#include "Magnefu/Renderer/SceneObject.h"
#include "Magnefu/Renderer/GraphicsContext.h"
#include "Magnefu/Renderer/BindGroup.h"
//----------------------------------------

//----Scene-------------------------------
#include "Magnefu/Scene/Scene.h"
#include "Magnefu/Scene/Entity.h"
#include "Magnefu/Scene/Components.h"
#include "Magnefu/Scene/SceneManager.h"
//----------------------------------------




// ----Entry Point---------------------
//#include "Magnefu/Entrypoint.h"
//-------------------------------------