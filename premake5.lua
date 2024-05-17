include "common_settings.lua"


        -- MAGNEFU WORKSPACE

workspace "Magnefu"
    configurations { "Debug", "Release", "Dist" }
    startproject "Editor"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
resourcedir = "%{prj.name}/res"

SysIncludeDir = {}

-- May need to be adjusted for your system
-- To figure out where std c++ headers are run:
-- find /Applications/Xcode.app -name "iostream" 2>/dev/null
SysIncludeDir["mac_stdcplusplus"] = "/Applications/Xcode.app/Contents/Developer/Platforms/DriverKit.platform/Developer/SDKs/DriverKit.sdk/System/DriverKit/usr/include/c++/v1"

IncludeDir = {}
IncludeDir["GLFW"] = "Magnefu/vendor/GLFW/include"
IncludeDir["GLAD"] = "Magnefu/vendor/GLAD/include"
IncludeDir["ImGui"] = "Magnefu/vendor/imgui"
IncludeDir["spdlog"] = "Magnefu/vendor/spdlog/include"
IncludeDir["Vulkan"] = "Magnefu/vendor/vulkan/include"
IncludeDir["SOIL2"] = "Magnefu/vendor/SOIL2/include"
IncludeDir["entt"] = "Magnefu/vendor/entt/include"
IncludeDir["assimp"] = "Magnefu/vendor/assimp/include"
IncludeDir["meshoptimizer"] = "Magnefu/vendor/meshoptimizer"
IncludeDir["vma"] = "Magnefu/vendor/vma"
-- IncludeDir["MoltenVK"] = "path/to/MoltenVK/include"  -- Adjust this path

LibDir = {}
LibDir["GLFW"] = "Magnefu/vendor/GLAD/lib"
LibDir["Vulkan"] = "Magnefu/vendor/vulkan/lib"
LibDir["SOIL2"] = "Magnefu/vendor/SOIL2/lib"
LibDir["assimp"] = "Magnefu/vendor/assimp/lib/x64"
LibDir["MoltenVK"] = "/usr/local/lib"  

-- Includes Premake files
include "Magnefu/vendor/GLFW"
include "Magnefu/vendor/GLAD"
include "Magnefu/vendor/imgui"

prebuildcommands {
    "{MKDIR} ../bin/" .. outputdir .. "/Editor",
    "{MKDIR} ../bin-int/" .. outputdir .. "/Editor"
}


        -- MAGNEFU PROJECT PREMAKE BUILD SETTINGS

project "Magnefu"
    location "Magnefu"
    kind "StaticLib"
    staticruntime "on"
    language "C++"
    cppdialect "C++20"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    --[[
    pchheader "%{prj.name}/src/mfpch.h"
    pchsource "%{prj.name}/src/mfpch.cpp"
    ]]

    pchheader "src/mfpch.h"   -- Changed for xcode. Testing needed to determine if this works for visual studio
    pchsource "src/mfpch.cpp"

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/vendor/**.h",
        "%{prj.name}/vendor/**.hpp",
        "%{prj.name}/vendor/**.cpp",

        -- Explicitly makes c files available
        "%{prj.name}/vendor/tlsf/tlsf.c",
        resourcedir .. "/**",
    }

    removefiles {
        "%{prj.name}/vendor/imgui/misc/**.h",
        "%{prj.name}/vendor/imgui/misc/**.cpp",
        "%{prj.name}/vendor/imgui/examples/**.h",
        "%{prj.name}/vendor/imgui/examples/**.cpp",
        "%{prj.name}/vendor/imgui/backends/**.h",
        "%{prj.name}/vendor/imgui/backends/**.cpp",
    }

    files {
        "%{prj.name}/vendor/imgui/backends/imgui_impl_glfw.h",
        "%{prj.name}/vendor/imgui/backends/imgui_impl_glfw.cpp"
    }

    includedirs {
        "%{prj.name}/vendor",
        "%{prj.name}/src",
        "%{prj.name}/src/Renderer",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.Vulkan}",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.GLAD}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.SOIL2}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.meshoptimizer}",
        "%{IncludeDir.vma}",
    }

    libdirs {
        "%{LibDir.Vulkan}",
        "%{LibDir.GLFW}",
        "%{LibDir.SOIL2}",
        "%{LibDir.assimp}",
    }

    links {
        "GLFW",
        "GLAD",
        "ImGui",
        "glfw3_mt",
        "vulkan-1",
        "shaderc_shared",
        "soil2",
        "opengl32",
        "assimp-vc143-mt"
    }

    -- C files ignore PCH
    filter "files:magnefu/vendor/**.c"
        flags {
            "NoPCH"
        }

    -- All vendor files ignore PCH
    filter "files:magnefu/vendor/**"
        flags {
            "NoPCH"
        }

    filter "system:windows"
        systemversion "latest"
        architecture "x64"

        defines {
            "MF_PLATFORM_WINDOWS",
            "MF_ARCH_X64",
            "MF_BUILD_DLL",
            "_GLFW_USE_HYBRID_HPG",
            "GLFW_INCLUDE_NONE",
            "_CRT_SECURE_NO_WARNINGS"
        }

    filter "system:macosx"
        systemversion "11.0" -- Earliest MacOS with m1 support (ARM Architecture)
        architecture "ARM64"  -- Specifies building for both Intel (x64) and Apple Silicon (ARM64)

        defines {
            "MF_PLATFORM_MACOS",
            "GLFW_INCLUDE_NONE",
            "MF_ARCH_ARM"
        }

        buildoptions {
            "-stdlib=libc++",
            "-fmodules",  -- Example: Enabling Clang modules
            "-fcxx-modules",
            "-fprofile-arcs", 
            "-ftest-coverage",
            "-Wall",        -- Enables all the warnings about constructions
            "-Wextra",      -- Enables some extra warning flags not enabled by -Wall
            --"-Werror",      -- Make all warnings into errors.
            "-Wshadow",     -- Warn the user if a variable declaration shadows one from a parent context.
            "-Wnon-virtual-dtor", -- Warn when a class with a virtual function has a non-virtual destructor.
            "-Wold-style-cast", -- Warn for c-style casts.
            "-Wunused",     -- Warn on anything being unused.
            "-Woverloaded-virtual", -- Warn if you overload (not override) a virtual function.
            "-Wpedantic",   -- (or -Wpendantic) Warn if non-standard C++ is used.
            "-Wconversion", -- Warn on type conversions that may lose data.
            "-Wsign-conversion", -- Warn on sign conversions.
            "-Wmisleading-indentation", -- Warn if indentation implies blocks where blocks do not exist.
            "-Wnull-dereference", -- Warn if a null dereference is detected.
            "-Wdouble-promotion", -- Warn on implicit conversion from float to double.
            "-Wformat=2"   -- Warn on security issues around functions that format output (like printf).
        }

        linkoptions {
            "-Wl, -dead_strip"  -- Corresponds to enabling dead code stripping
        }

        includedirs {
            --"%{IncludeDir.MoltenVK}",
        }

        -- Add your system header search paths here
        externalincludedirs { 
            "%{SysIncludeDir.mac_stdcplusplus}" 
        }

        libdirs {
            "%{LibDir.MoltenVK}",
        }

        links {
            "c++",
            "Cocoa.framework",
            "IOKit.framework",
            "CoreVideo.framework",
            "libMoltenVK.dylib"
        }

        filter { "system:macosx", "configurations:Debug" }
            buildoptions { "-fobjc-weak" }

        filter { "system:macosx", "configurations:Release" }
            buildoptions { "-fobjc-weak" }

        filter { "system:macosx", "configurations:Dist" }
            buildoptions { "-fobjc-weak" }

    filter "configurations:Debug"
        defines {
            "MF_DEBUG"
        }
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines  "MF_RELEASE"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        defines  "MF_DIST"
        runtime "Release"
        optimize "on"


        -- EDITOR PROJECT PREMAKE BUILD SETTINGS

project "Editor"
    location "Editor"
    kind "ConsoleApp" 
    staticruntime "on"
    language "C++"
    cppdialect "C++20"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp",
        resourcedir .. "/**",
    }

    includedirs {
        "Magnefu/src",
        "Magnefu/vendor",
        "Magnefu/vendor/vulkan/include",  -- TODO: Other projects shouldn't have access to Graphics API code
        "%{IncludeDir.spdlog}",
        "%{prj.name}/src",
        "%{IncludeDir.GLAD}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.meshoptimizer}",
        "%{IncludeDir.vma}",
    }

    libdirs {
    }

    links {
        "Magnefu"
    }

    filter "system:windows"
        systemversion "latest"
        architecture "x64"

        defines {
            "MF_PLATFORM_WINDOWS",
            "MF_ARCH_X64",
        }

    filter "system:macosx"
        systemversion "11.0" -- Earliest MacOS with m1 support (ARM Architecture)
        architecture  "ARM64"  -- Specifies building for both Intel (x64) and Apple Silicon (ARM64)

        defines {
            "MF_PLATFORM_MACOS",
            "MF_ARCH_ARM",
        }

        buildoptions {
            "-stdlib=libc++",
            "-fmodules",  -- Example: Enabling Clang modules
            "-fcxx-modules",
            "-fprofile-arcs", 
            "-ftest-coverage",
            "-Wall",        -- Enables all the warnings about constructions
            "-Wextra",      -- Enables some extra warning flags not enabled by -Wall
            --"-Werror",      -- Make all warnings into errors.
            "-Wshadow",     -- Warn the user if a variable declaration shadows one from a parent context.
            "-Wnon-virtual-dtor", -- Warn when a class with a virtual function has a non-virtual destructor.
            "-Wold-style-cast", -- Warn for c-style casts.
            "-Wunused",     -- Warn on anything being unused.
            "-Woverloaded-virtual", -- Warn if you overload (not override) a virtual function.
            "-Wpedantic",   -- (or -Wpendantic) Warn if non-standard C++ is used.
            "-Wconversion", -- Warn on type conversions that may lose data.
            "-Wsign-conversion", -- Warn on sign conversions.
            "-Wmisleading-indentation", -- Warn if indentation implies blocks where blocks do not exist.
            "-Wnull-dereference", -- Warn if a null dereference is detected.
            "-Wdouble-promotion", -- Warn on implicit conversion from float to double.
            "-Wformat=2"   -- Warn on security issues around functions that format output (like printf).
        }

        linkoptions {
            "-Wl, -dead_strip"  -- Corresponds to enabling dead code stripping
        }

        links {
            "c++"
        }

        includedirs {
            "%{IncludeDir.MoltenVK}",
        }
        
        filter { "system:macosx", "configurations:Debug" }
            buildoptions { "-fobjc-weak", "-fprofile-arcs", "-ftest-coverage" }
            defines { "ENABLE_TESTABILITY" }

        filter { "system:macosx", "configurations:Release" }
            buildoptions { "-fobjc-weak" }

        filter { "system:macosx", "configurations:Dist" }
            buildoptions { "-fobjc-weak" }


    filter "configurations:Debug"
        defines {
            "MF_DEBUG"
        }
        symbols "on"

    filter "configurations:Release"
        defines  "MF_RELEASE"
        optimize "on"

    filter "configurations:Dist"
        defines  "MF_DIST"
        optimize "on"

    postbuildcommands {
        "{COPYDIR} %{prj.location}/res/* %{cfg.buildtarget.directory}/res" 
    }