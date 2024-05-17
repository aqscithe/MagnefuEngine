project "GLAD"
    kind "StaticLib"
    language "C"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "include/GLAD/glad.h",
        "include/KHR/khrplatform.h",
        "src/glad.c"
    }

    includedirs {
        "include"
    }

    filter "system:linux"
        pic "on"
        systemversion "latest"
        
        files {
            "src/glad.c"
        }

        defines {}

    filter "system:windows"
        systemversion "latest"

        files {
            "src/glad.c"
        }

        defines { 
            "_CRT_SECURE_NO_WARNINGS"
        }

    filter "system:macosx"
        systemversion "11.0" -- Earliest MacOS with m1 support (ARM Architecture)
        architecture "ARM64"
        
        files {
            "src/glad.c"
        }

        defines {
            "MF_PLATFORM_MACOS",
            "MF_ARCH_ARM",
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
