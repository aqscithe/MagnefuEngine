#pragma once

// TODO: I currently need to add this file to the imgui cpp files in order to compile
// See if there is a way to avoid this.

#include <iostream>
#include <memory>
#include <algorithm>
#include <utility>
#include <functional>
#include <chrono>

#include <future>
#include <mutex>

#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include "MathsCommon.h"
#include "Globals.h"
#include "Vectors.h"
#include "Matrices.h"
#include "Magnefu/Log.h"

#ifdef MF_PLATFORM_WINDOWS
#include <Windows.h>
#endif