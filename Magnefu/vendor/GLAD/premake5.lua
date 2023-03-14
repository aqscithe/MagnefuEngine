project "GLAD"
	kind "StaticLib"
	language "C"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-intermediates/" .. outputdir .. "/%{prj.name}")

	files {
		"include/GLAD/glad.h",
		"include/KHR/khrplatform.h",
		"src/glad.c"
	}

	includedirs {
		"include"
	}

	filter "system:linux"
		pic "On"

		systemversion "latest"
		staticruntime "On"

		files {
			"src/glad.c"
		}

		defines {}

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

		files {
			"src/glad.c"
		}

		defines { 
			"_CRT_SECURE_NO_WARNINGS"
		}

	filter "configurations:Debug"
		runtime "Debug"
		buildoptions "/MDd"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		buildoptions "/MD"
		optimize "on"