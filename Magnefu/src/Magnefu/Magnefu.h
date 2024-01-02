#pragma once

// -- Application Includes ---------//
#include "Magnefu/Application/Entrypoint.h"
#include "Magnefu/Application/Application.h"
#include "Magnefu/Application/ImGui/ImGuiService.hpp"
#include "Magnefu/Application/Layer.h"
#include "Magnefu/Application/LayerStack.h"

#include "Magnefu/Application/Input/Input.h"

#include "Magnefu/Application/Scene/Scene.h"
#include "Magnefu/Application/Scene/Entity.h"
#include "Magnefu/Application/Scene/Components.h"
#include "Magnefu/Application/Scene/SceneManager.h"

#include "Magnefu/Application/Windows/WindowsWindow.h"
#include "Magnefu/Application/Camera/Camera.h"
#include "Magnefu/Application/Camera/SceneCamera.h"



// -- Graphics Includes --------------------- //
#include "Magnefu/Graphics/RenderConstants.h"
#include "Magnefu/Graphics/Light.h"
#include "Magnefu/Graphics/Material.h"
#include "Magnefu/Graphics/GraphicsContext.h"
#include "Magnefu/Graphics/Renderer.hpp"
#include "Magnefu/Graphics/CommandBuffer.hpp"
#include "Magnefu/Graphics/GPUProfiler.hpp"



// -- Core Includes ---------------------------------- //
#include "Magnefu/Core/Assertions.h"
#include "Magnefu/Core/Log.h"
#include "Magnefu/Core/SmartPointers.h"
#include "Magnefu/Core/Array.h"
#include "Magnefu/Core/Debug/Instrumentor.h"
#include "Magnefu/Core/Time.hpp"
#include "Magnefu/Core/MemoryAllocation/Memory.hpp"
#include "Magnefu/Core/SmartPointers.h"
#include "Magnefu/Core/ServiceManager.hpp"
#include "Magnefu/Core/Maths/MathsCommon.h"