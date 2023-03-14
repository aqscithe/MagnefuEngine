workspace "Magnefu"
    architecture "x64"
    configurations { "Debug", "Release", "Dist" }
    startproject "Sandbox"


outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
resourcedir = "%{prj.name}/res"

IncludeDir = {}
IncludeDir["GLFW"] = "Magnefu/vendor/GLFW/include"
IncludeDir["GLAD"] = "Magnefu/vendor/GLAD/include"

LibDir = {}
LibDir["GLFW"] = "Magnefu/vendor/GLFW/lib-vc2022"

include "Magnefu/vendor/GLFW"
include "Magnefu/vendor/GLAD"

prebuildcommands {
    "{MKDIR} ../bin/" .. outputdir .. "/Sandbox",
    "{MKDIR} ../bin/" .. outputdir .. "/Magnefu",
    "{MKDIR} ../bin-intermediates/" .. outputdir .. "/Sandbox",
    "{MKDIR} ../bin-intermediates/" .. outputdir .. "/Magnefu",
    "{MKDIR} ../bin/" .. outputdir .. "/GLFW",
    "{MKDIR} ../bin-intermediates/" .. outputdir .. "/GLFW",
    "{MKDIR} ../bin/" .. outputdir .. "/GLAD",
    "{MKDIR} ../bin-intermediates/" .. outputdir .. "/GLAD"
}

project "Magnefu"
    location "Magnefu"
    kind "SharedLib" -- means dynamically linked lib
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-intermediates/" .. outputdir .. "/%{prj.name}")

    pchheader "mfpch.h"
    pchsource "Magnefu/src/mfpch.cpp"

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/vendor/**.h",
        "%{prj.name}/vendor/**.cpp",
        resourcedir .. "/**",
    }

    includedirs {
        "%{prj.name}/vendor",
        "%{prj.name}/src",
        "%{prj.name}/src/Maths",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.GLAD}"
    }

    libdirs { 
        "%{LibDir.GLFW}",
    }

    links {
        "glfw3",
        "GLAD",
        "opengl32"
    }


    filter "system:windows"
        cppdialect "C++20"
        staticruntime "On"
        systemversion "latest"

        defines {
            "MF_PLATFORM_WINDOWS",
            "MF_BUILD_DLL",
            "_GLFW_USE_HYBRID_HPG",
            "GLFW_INCLUDE_NONE"
        }

    postbuildcommands {
        "{COPYFILE} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox"
    }

    filter "configurations:Debug"
        defines {
            "MF_DEBUG",
            "MF_ENABLE_ASSERTS"
        }
        buildoptions "/MDd"
        symbols "On"

    filter "configurations:Release"
        defines  "MF_RELEASE"
        buildoptions "/MD"
        optimize "On"

    filter "configurations:Dist"
        defines  "MF_DIST"
        buildoptions "/MD"
        optimize "On"


project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp" 
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-intermediates/" .. outputdir .. "/%{prj.name}")

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
    }

    links {
        "Magnefu"
    }

    filter "system:windows"
        cppdialect "C++20"
        staticruntime "On"
        systemversion "latest"

        defines {
            "MF_PLATFORM_WINDOWS"
        }

    postbuildcommands {
        "{COPYDIR} %{prj.location}/res/* %{cfg.buildtarget.directory}/res" 
        --"XCOPY $(ProjectDir)res* %{cfg.buildtarget.directory}/res /S /Y"
    }

    filter "configurations:Debug"
        defines {
            "MF_DEBUG",
            "MF_ENABLE_ASSERTS"
        }
        buildoptions "/MDd"
        symbols "On"

    filter "configurations:Release"
        defines  "MF_RELEASE"
        buildoptions "/MD"
        optimize "On"

    filter "configurations:Dist"
        defines  "MF_DIST"
        buildoptions "/MD"
        optimize "On"





