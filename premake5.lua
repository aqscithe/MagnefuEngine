include "common_settings.lua"

workspace "Magnefu"
    configurations { "Debug", "Release", "Dist" }
    startproject "Editor"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
resourcedir = "%{prj.name}/res"

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

project "Magnefu"
    location "Magnefu"
    kind "StaticLib"
    staticruntime "on"
    language "C++"
    cppdialect "C++20"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "%{prj.name}/src/mfpch.h"
    pchsource "%{prj.name}/src/mfpch.cpp"

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
        "%{prj.name}/src/Maths",
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
            "-fmodules",  -- Example: Enabling Clang modules
            "-fcxx-modules"
        }

        linkoptions {
            "-dead_strip"  -- Corresponds to enabling dead code stripping
        }

        includedirs {
            --"%{IncludeDir.MoltenVK}",
        }

        libdirs {
            "%{LibDir.MoltenVK}",
        }

        links {
            "Cocoa.framework",
            "IOKit.framework",
            "CoreVideo.framework",
            "libMoltenVK.dylib"
        }

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
        "Magnefu/src/Maths",
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
            "-fmodules",  -- Example: Enabling Clang modules
            "-fcxx-modules"
        }
        linkoptions {
            "-dead_strip"  -- Corresponds to enabling dead code stripping
        }

        includedirs {
            "%{IncludeDir.MoltenVK}",
        }
        
    postbuildcommands {
        "{COPYDIR} %{prj.location}/res/* %{cfg.buildtarget.directory}/res" 
    }

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
