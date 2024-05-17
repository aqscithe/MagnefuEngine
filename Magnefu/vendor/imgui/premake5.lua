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
        systemversion "11.0" -- Earliest MacOS with m1 support (ARM Architecture)
        architecture "ARM64"

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

        filter { "system:macosx", "configurations:Debug" }
            buildoptions { "-fobjc-weak", "-fprofile-arcs", "-ftest-coverage" }
            defines { "ENABLE_TESTABILITY" }

        filter { "system:macosx", "configurations:Release" }
            buildoptions { "-fobjc-weak" }


    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
