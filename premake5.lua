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
	"x32",
	"x64",
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

filter "system:macosx"
	defines {
		"SYSTEM_MACOSX",
	}
	system "macosx"

filter "system:linux"
	defines {
		"SYSTEM_LINUX",
	}
	system "linux"

filter "system:windows"
	defines {
		"SYSTEM_WINDOWS",
	}
	system "windows"

filter "platforms:x32"
	architecture "x86"

filter "platforms:x64"
	architecture "x64"

workspace "etherdream-glsl"
	language "C++"
	location "build"
	startproject "etherdream-glsl"

project "etherdream-glsl"
	debugargs {
		"-s",
		"../../../example.frag",
	}
	files {
		"src/common/**",
	}
	includedirs {
		"src",
		"deps/include",
	}
	links {
		"glew32",
		"opengl32",
	}
	kind "ConsoleApp"

	filter "configurations:Debug"
		defines {
			"DEBUG",
		}
		links {
			"efsw-debug",
		}
		targetsuffix "-d"
		symbols "On"

	filter "configurations:Release"
		defines {
			"NDEBUG",
		}
		optimize "On"
		links {
			"efsw",
		}

	filter "system:linux"
		files {
			"src/linux/**",
		}
		links {
			"etherdream"
		}

	filter { "system:linux", "platforms:x32" }
		debugdir "deps/linux/bin32"
		libdirs {
			"deps/linux/lib32",
		}

	filter { "system:linux", "platforms:x64" }
		debugdir "deps/linux/bin64"
		libdirs {
			"deps/linux/lib64",
		}

	filter "system:macosx"
		files {
			"src/macosx/**",
		}
		links {
			"etherdream",
		}

	filter { "system:macosx", "platforms:x32" }
		debugdir "deps/macosx/bin32"
		libdirs {
			"deps/macosx/lib32",
		}

	filter { "system:macosx", "platforms:x64" }
		debugdir "deps/macosx/bin64"
		libdirs {
			"deps/macosx/lib64",
		}

	filter "system:windows"
		files {
			"src/windows/**",
		}
		links {
			"EtherDream",
		}

	filter { "system:windows", "platforms:x32" }
		debugdir "deps/windows/bin32"
		libdirs {
			"deps/windows/lib32",
		}

	filter { "system:windows", "platforms:x64" }
		debugdir "deps/windows/bin64"
		libdirs {
			"deps/windows/lib64",
		}
