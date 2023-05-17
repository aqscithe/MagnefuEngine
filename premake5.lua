

workspace "Magnefu"
    architecture "x64"
    configurations { "Debug", "Release", "Dist" }
    startproject "Sandbox"


outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
resourcedir = "%{prj.name}/res"

IncludeDir = {}
IncludeDir["GLFW"] = "Magnefu/vendor/GLFW/include"
IncludeDir["GLAD"] = "Magnefu/vendor/GLAD/include"
IncludeDir["ImGui"] = "Magnefu/vendor/imgui"
IncludeDir["Vulkan"] = "Magnefu/vendor/vulkan/include"

LibDir = {}
LibDir["GLFW"] = "Magnefu/vendor/GLAD/lib"
LibDir["Vulkan"] = "Magnefu/vendor/vulkan/lib"

-- Includes Premake files
include "Magnefu/vendor/GLFW"
include "Magnefu/vendor/GLAD"
include "Magnefu/vendor/imgui"

prebuildcommands {
    "{MKDIR} ../bin/" .. outputdir .. "/Sandbox",
    "{MKDIR} ../bin-int/" .. outputdir .. "/Sandbox"
}

project "Magnefu"
    location "Magnefu"
    kind "StaticLib"
    staticruntime "on"
    language "C++"
    cppdialect "C++20"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "mfpch.h"
    pchsource "Magnefu/src/mfpch.cpp"

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/vendor/**.h",
        "%{prj.name}/vendor/**.cpp",
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
        "%{prj.name}/vendor/imgui/backends/imgui_impl_glfw.cpp",
        "%{prj.name}/vendor/imgui/backends/imgui_impl_opengl3.h",
        "%{prj.name}/vendor/imgui/backends/imgui_impl_opengl3.cpp"
    }

    includedirs {
        "%{prj.name}/vendor",
        "%{prj.name}/src",
        "%{prj.name}/src/Maths",
        "%{prj.name}/src/Renderer",
        "%{IncludeDir.Vulkan}",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.GLAD}",
        "%{IncludeDir.ImGui}",
        
    }

    libdirs {
        "%{LibDir.Vulkan}",
        "%{LibDir.GLFW}",
        
    }

    links {
        "GLFW",
        "GLAD",
        "ImGui",
        "glfw3_mt",
        "vulkan-1",
        "opengl32",
    }

    filter "system:windows"
        systemversion "latest"

        defines {
            "MF_PLATFORM_WINDOWS",
            "MF_BUILD_DLL",
            "_GLFW_USE_HYBRID_HPG",
            "GLFW_INCLUDE_NONE",
            "_CRT_SECURE_NO_WARNINGS"
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


project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp" 
    staticruntime "on"
    language "C++"
    cppdialect "C++20"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        resourcedir .. "/**",
    }

    includedirs {
        "Magnefu/src",
        "Magnefu/vendor",
        "Magnefu/vendor/spdlog/include",
        "Magnefu/src/Maths",
        "%{prj.name}/src",
        "%{IncludeDir.GLAD}",
    }

    links {
        "Magnefu"
    }

    filter "system:windows"
        systemversion "latest"

        defines {
            "MF_PLATFORM_WINDOWS"
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





