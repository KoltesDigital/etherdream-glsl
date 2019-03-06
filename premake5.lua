characterset "Unicode"

cppdialect "C++11"

defines {
	"_USE_MATH_DEFINES",
	"_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING",
}

flags {
	"FatalWarnings",
	"MultiProcessorCompile",
	"NoPCH",
}

configurations {
	"Debug",
	"Release",
}

platforms {
	"Linux32",
	"Linux64",
	"Win32",
	"Win64",
}

warnings "Extra"

filter "configurations:Debug"
	defines {
		"DEBUG",
	}
	runtime "Debug"
	symbols "On"
	targetsuffix "-d"

filter "configurations:Release"
	defines {
		"NDEBUG",
	}
	optimize "Speed"
	runtime "Release"

filter "platforms:Linux*"
	system "Linux"

filter "platforms:Linux32"
	architecture "x86"

filter "platforms:Linux64"
	architecture "x64"

filter "platforms:Win*"
	system "Windows"

filter "platforms:Win32"
	architecture "x86"

filter "platforms:Win64"
	architecture "x64"

workspace "etherdream-glsl"
	language "C++"
	location "build"
	startproject "etherdream-glsl"

project "etherdream-glsl"
	debugargs {
		"-c",
		"0",
	}
	files {
		"src/common/**",
	}
	includedirs {
		"src",
		"deps/include",
	}
	links {
		"opengl32",
		"glew32"
	}
	kind "ConsoleApp"

	filter "configurations:Debug"
		links {
		}

	filter "configurations:Release"
		links {
		}

	filter "platforms:Linux*"
		files {
			"src/linux/**",
		}
		links {
			"etherdream"
		}

	filter "platforms:Linux32"
		debugdir "deps/linux/bin32"
		libdirs {
			"deps/linux/lib32",
		}

	filter "platforms:Linux64"
		debugdir "deps/linux/bin64"
		libdirs {
			"deps/linux/lib64",
		}

	filter "platforms:Win*"
		files {
			"src/windows/**",
		}
		links {
			"EtherDream",
		}

	filter "platforms:Win32"
		debugdir "deps/windows/bin32"
		libdirs {
			"deps/windows/lib32",
		}

	filter "platforms:Win64"
		debugdir "deps/windows/bin64"
		libdirs {
			"deps/windows/lib64",
		}
