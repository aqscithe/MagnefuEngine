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
#include <vector>
#include <string>
#include <sstream>
#include <fstream>


#include "Magnefu/Core/Maths/MathsCommon.h"
#include "Magnefu/Core/Maths/PrimitiveCommon.h"
#include "Magnefu/Core/Maths/Vectors.h"
#include "Magnefu/Core/Maths/Matrices.h"

#include "Magnefu/Core/Log.h"
#include "Magnefu/Core/SmartPointers.h"
#include "Magnefu/Core/Span.h"
#include "Magnefu/Core/Typedefs.hpp"
#include "Magnefu/ImGui/ImGuiDefs.h"
#include "Magnefu/Core/Array.h"
#include "Magnefu/Core/Assertions.h"
#include "Magnefu/Core/MemoryAllocation/Memory.hpp"

#include "Magnefu/Debug/Instrumentor.h"


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>



#ifdef MF_PLATFORM_WINDOWS
#include <Windows.h>
#endif