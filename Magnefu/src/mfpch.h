#pragma once

// TODO: I currently need to add this file to the imgui cpp files in order to compile
// See if there is a way to avoid this.

#include <stdint.h>

#include <iostream>
#include <memory>
#include <algorithm>
#include <utility>
#include <functional>
#include <numeric>
#include <chrono>
#include <random>
#include <any>

#include <thread>
#include <future>
#include <mutex>

#include <unordered_map>
#include <string>
#include <sstream>
#include <fstream>


#include "Magnefu/Application/ImGui/ImGuiDefs.h"

#include "Magnefu/Core/Maths/MathsCommon.h"
#include "Magnefu/Core/Maths/Vectors.h"
#include "Magnefu/Core/Maths/Matrices.h"

#include "Magnefu/Core/Log.h"
#include "Magnefu/Core/SmartPointers.h"
#include "Magnefu/Core/Span.h"
#include "Magnefu/Core/Platform.hpp"
#include "Magnefu/Core/Array.h"
#include "Magnefu/Core/Assertions.h"
#include "Magnefu/Core/MemoryAllocation/Memory.hpp"
#include "Magnefu/Core/File.hpp"
#include "Magnefu/Core/String.hpp"
#include "Magnefu/Core/Time.hpp"
#include "Magnefu/Core/HashMap.hpp"
#include "Magnefu/Core/Debug/Instrumentor.h"
#include "Magnefu/Core/DataStructures.hpp"


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>



#ifdef MF_PLATFORM_WINDOWS
#include <Windows.h>
#endif