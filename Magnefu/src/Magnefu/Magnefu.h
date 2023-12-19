#pragma once

// -- Application Includes ---------//
#include "Magnefu/Application/Application.h"
#include "Magnefu/Application/ImGui/ImGuiLayer.h"
#include "Magnefu/Application/Layer.h"

#include "Magnefu/Application/Events/Input.h"
#include "Magnefu/Application/Events/KeyCodes.h"
#include "Magnefu/Application/Events/MouseButtonCodes.h"

#include "Magnefu/Application/Scene/Scene.h"
#include "Magnefu/Application/Scene/Entity.h"
#include "Magnefu/Application/Scene/Components.h"
#include "Magnefu/Application/Scene/SceneManager.h"



// -- Graphics Includes --------------------- //
#include "Magnefu/Graphics/RenderConstants.h"
#include "Magnefu/Graphics/RenderCommand.h"
#include "Magnefu/Graphics/Renderer.h"
#include "Magnefu/Graphics/Light.h"
#include "Magnefu/Graphics/Material.h"
#include "Magnefu/Graphics/Buffer.h"
#include "Magnefu/Graphics/Shader.h"
#include "Magnefu/Graphics/Texture.h"
#include "Magnefu/Graphics/SceneCamera.h"
#include "Magnefu/Graphics/GraphicsContext.h"
#include "Magnefu/Graphics/BindGroup.h"



// -- Core Includes ---------------------------------- //
#include "Magnefu/Core/Assertions.h"
#include "Magnefu/Core/Log.h"
#include "Magnefu/Core/SmartPointers.h"
#include "Magnefu/Core/Array.h"
