workspace "Magnefu"
    architecture "x64"
    configurations { "Debug", "Release", "Dist" }


outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
resourcedir = "%{prj.name}/res"

project "Magnefu"
    location "Magnefu"
    kind "SharedLib" -- means dynamic linked lib
    language "C++"

    targetdir ("bin/" .. outputdir .. "%{prj.name}")
    objdir ("bin-intermediates/" .. outputdir .. "%{prj.name}")

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        --"%{prj.name}/vendor/imgui/**.h",
        --"%{prj.name}/vendor/imgui/**.cpp",
        --"%{prj.name}/vendor/imgui/**.h",
        --"%{prj.name}/vendor/imgui/**.cpp",
        resourcedir .. "/**",
    }

    includedirs {
        "%{prj.name}/vendor/spdlog/include",
        "%{prj.name}/vendor",
        "%{prj.name}/src",
        "Maths/src",
        "Dependencies/",
        "Dependencies/GLEW_2.1.0/include",
        "Dependencies/GLFW/include",
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
        "{COPYFILE} %{cfg.buildtarget.relpath} ../bin" .. outputdir .. "/Sandbox"
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


project "Maths"
    location "Maths"
    kind "SharedLib" -- means dynamic linked lib
    language "C++"

    targetdir ("bin/" .. outputdir .. "%{prj.name}")
    objdir ("bin-intermediates/" .. outputdir .. "%{prj.name}")

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
    }

    includedirs {}

    filter "system:windows"
        cppdialect "C++20"
        staticruntime "On"
        systemversion "latest"

        defines {}

        postbuildcommands {}

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
    kind "ConsoleApp" -- means dynamic linked lib
    language "C++"

    targetdir ("bin/" .. outputdir .. "%{prj.name}")
    objdir ("bin-intermediates/" .. outputdir .. "%{prj.name}")

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        resourcedir .. "/**",
    }

    includedirs {
        "Magnefu/src",
        "Magnefu/vendor",
        "Magnefu/vendor/spdlog/include",
        "%{prj.name}/src",
        "Dependencies/",
        "Dependencies/GLEW_2.1.0/include",
        "Dependencies/GLFW/include",
    }

    links {
        "Magnefu",
        "Maths"
    }

    filter "system:windows"
        cppdialect "C++20"
        staticruntime "On"
        systemversion "latest"

        defines {
            "MF_PLATFORM_WINDOWS",
        }

    postbuildcommands {
        "{COPYDIR}" .. resourcedir .. " " .. outputdir .. "/Sandbox"
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