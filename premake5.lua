workspace "Magnefu"
    architecture "x64"
    configurations { "Debug", "Release", "Dist" }
    startproject "Sandbox"


outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
resourcedir = "%{prj.name}/res"

prebuildcommands {
    "{MKDIR} ../bin/" .. outputdir .. "/Sandbox",
    "{MKDIR} ../bin/" .. outputdir .. "/Magnefu",
    "{MKDIR} ../bin-intermediates/" .. outputdir .. "/Sandbox",
    "{MKDIR} ../bin-intermediates/" .. outputdir .. "/Magnefu",
}

project "Magnefu"
    location "Magnefu"
    kind "SharedLib" -- means dynamic linked lib
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-intermediates/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/vendor/**.h",
        "%{prj.name}/vendor/**.cpp",
        resourcedir .. "/**",
    }

    includedirs {
        "%{prj.name}/vendor/spdlog",
        "%{prj.name}/vendor",
        "%{prj.name}/src",
        "%{prj.name}/src/Maths",
        "Dependencies/",
        "Dependencies/GLEW_2.1.0/include",
        "Dependencies/GLFW/include",
    }

    libdirs { 
        "Dependencies/GLEW_2.1.0/lib/Release/x64",
        "Dependencies/GLFW/lib-vc2022",
    }

    links {
        "glfw3_mt",
        "glew32s",
        "opengl32"
    }


    filter "system:windows"
        cppdialect "C++20"
        staticruntime "On"
        systemversion "latest"

        defines {
            "GLEW_STATIC",
            "_GLFW_USE_HYBRID_HPG",
            "MF_PLATFORM_WINDOWS",
            "MF_BUILD_DLL",
        }

    postbuildcommands {
        "{COPYFILE} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox"
    }

    filter "configurations:Debug"
        defines "MF_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines  "MF_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines  "MF_DIST"
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

    -- temporary
    removefiles {
        "%{prj.name}/src/Application.*"
    }

    includedirs {
        "Magnefu/src",
        "Magnefu/vendor",
        "Magnefu/vendor/spdlog/include",
        "Magnefu/src/Maths",
        "%{prj.name}/src",
        "Dependencies/",
        "Dependencies/GLEW_2.1.0/include",
        "Dependencies/GLFW/include",
    }

    links {
        "Magnefu"
    }

    filter "system:windows"
        cppdialect "C++20"
        staticruntime "On"
        systemversion "latest"

        defines {
            "MF_PLATFORM_WINDOWS",
        }

    postbuildcommands {
        "{COPYDIR} %{prj.location}/res/* %{cfg.buildtarget.directory}/res" 
        --"XCOPY $(ProjectDir)res* %{cfg.buildtarget.directory}/res /S /Y"
    }

    filter "configurations:Debug"
        defines "MF_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines  "MF_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines  "MF_DIST"
        optimize "On"





