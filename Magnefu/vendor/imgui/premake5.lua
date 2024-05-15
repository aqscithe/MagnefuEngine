project "ImGui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "On"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "imconfig.h",
        "imgui.h",
        "imgui.cpp",
        "imgui_draw.cpp",
        "imgui_internal.h",
        "imgui_widgets.cpp",
        "imstb_rectpack.h",
        "imstb_textedit.h",
        "imstb_truetype.h",
        "imgui_demo.cpp",
        "backends/imgui_impl_glfw.h",
        "backends/imgui_impl_glfw.cpp",
        "backends/imgui_impl_opengl3.h",
        "backends/imgui_impl_opengl3.cpp",
        "backends/imgui_impl_vulkan.h",
        "backends/imgui_impl_vulkan.cpp"
    }

    includedirs {
        "%{wks.location}/Magnefu/src",
        "%{wks.location}/Magnefu/src/Maths",
        "%{wks.location}/Magnefu/vendor",
        "%{wks.location}/Magnefu/vendor/imgui",
        "%{wks.location}/Magnefu/vendor/GLFW/include",
        "%{wks.location}/Magnefu/vendor/vulkan/include"
    }

    filter "system:windows"
        systemversion "latest"

        defines {
            "MF_PLATFORM_WINDOWS",
            "_CRT_SECURE_NO_WARNINGS"
        }

    filter "system:linux"
        pic "On"
        systemversion "latest"

    filter "system:macosx"
        systemversion "latest"

        defines {
            "MF_PLATFORM_MACOS"
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
