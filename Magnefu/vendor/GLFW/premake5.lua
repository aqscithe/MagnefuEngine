project "GLFW"
    kind "StaticLib"
    language "C"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "include/GLFW/glfw3.h",
        "include/GLFW/glfw3native.h",
        "src/glfw_config.h",
        "src/context.c",
        "src/init.c",
        "src/input.c",
        "src/monitor.c",
        "src/vulkan.c",
        "src/window.c"
    }

    includedirs {
        "%{wks.location}/Magnefu/vendor/vulkan/include"
    }

    libdirs {
        "%{wks.location}/Magnefu/vendor/vulkan/lib"
    }

    links {
        "vulkan-1"
    }

    filter "system:linux"
        pic "on"

        systemversion "latest"
        staticruntime "on"

        files
        {
            "src/x11_init.c",
            "src/x11_monitor.c",
            "src/x11_window.c",
            "src/xkb_unicode.c",
            "src/posix_time.c",
            "src/posix_thread.c",
            "src/glx_context.c",
            "src/egl_context.c",
            "src/osmesa_context.c",
            "src/linux_joystick.c"
        }

        defines
        {
            "_GLFW_X11"
        }

    filter "system:windows"
        systemversion "latest"
        staticruntime "on"

        files
        {
            "src/win32_init.c",
            "src/win32_joystick.c",
            "src/win32_monitor.c",
            "src/win32_time.c",
            "src/win32_thread.c",
            "src/win32_window.c",
            "src/wgl_context.c",
            "src/egl_context.c",
            "src/osmesa_context.c"
        }

        defines 
        { 
            "_GLFW_WIN32",
            "_CRT_SECURE_NO_WARNINGS"
        }

    filter "system:macosx"
        systemversion "11.0" -- Earliest MacOS with m1 support (ARM Architecture)
        architecture "ARM64"
        staticruntime "on"

        files
        {
            "src/cocoa_init.m",
            "src/cocoa_joystick.m",
            "src/cocoa_monitor.m",
            "src/cocoa_window.m",
            "src/cocoa_time.c",
            "src/nsgl_context.m",
            "src/posix_thread.c",
            "src/egl_context.c",
            "src/osmesa_context.c"
        }

        defines
        {
            "MF_PLATFORM_MACOS",
            "MF_ARCH_ARM",
            "_GLFW_COCOA",
            "ENABLE_TESTABILITY"
        }

        buildoptions {
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

        links
        {
            "Cocoa.framework",
            "IOKit.framework",
            "CoreVideo.framework"
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
