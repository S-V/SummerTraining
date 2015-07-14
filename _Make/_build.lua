-- Based on bgfx's genie.lua:
-- Copyright 2010-2015 Branimir Karadzic. All rights reserved.
-- License: http://www.opensource.org/licenses/BSD-2-Clause
--

-----------------------------------------------------------------------
-- BGFX
-----------------------------------------------------------------------

newoption {
	trigger = "with-amalgamated",
	description = "Enable amalgamated build.",
}
newoption {
	trigger = "with-ovr",
	description = "Enable OculusVR integration.",
}
newoption {
	trigger = "with-sdl",
	description = "Enable SDL entry.",
}
newoption {
	trigger = "with-glfw",
	description = "Enable GLFW entry.",
}
newoption {
	trigger = "with-scintilla",
	description = "Enable building with Scintilla editor.",
}
newoption {
	trigger = "with-shared-lib",
	description = "Enable building shared library.",
}
newoption {
	trigger = "with-tools",
	description = "Enable building tools.",
}

-- e.g. "vs2010" or ""
local action = _ACTION or ""

G_ROOT_DIR = _WORKING_DIR
G_BUILD_DIR = "../.Build/";
G_DEMOS_DIR = "../_Demos/";
G_ENGINE_DIR = "../_Engine/";
G_BINARIES_DIR = "../_Binaries/";

G_SDL_DIR = path.join(G_ROOT_DIR, "SDL");

BGFX_DIR = path.getabsolute("../bgfx/")
BX_DIR = path.getabsolute(path.join(BGFX_DIR, "../bx"))

local BGFX_BUILD_DIR = path.join(BGFX_DIR, ".build")
local BGFX_THIRD_PARTY_DIR = path.join(BGFX_DIR, "3rdparty")

-- Create a new solution.
solution 'SummerTraining'

	-- Set the destination directory for the solution.
	--location ( G_SOLUTION_DIR )
	--printf("location: %s", location())

	configurations {
		"Debug",
		"Release",
	}

	if _ACTION == "xcode4" then
		platforms {
			"Universal",
		}
	else
		platforms {
			"x32",
			"x64",
			"Native", -- for targets where bitness is not specified
		}
	end

	language "C++"

-- needed for bgfx
function copyLib()
end

if _OPTIONS["with-sdl"] then
	if os.is("windows") then
		if not os.getenv("SDL2_DIR") then
			print("Set SDL2_DIR enviroment variable.")
		end
	end
end


function exampleProject(_name)

	project ("example-" .. _name)
		uuid (os.uuid("example-" .. _name))
		kind "WindowedApp"

	configuration {}

	debugdir (path.join(BGFX_DIR, "examples/runtime"))

	includedirs {
		path.join(BX_DIR,   "include"),
		path.join(BGFX_DIR, "include"),
		path.join(BGFX_DIR, "3rdparty"),
		path.join(BGFX_DIR, "examples/common"),
	}

	files {
		path.join(BGFX_DIR, "examples", _name, "**.c"),
		path.join(BGFX_DIR, "examples", _name, "**.cpp"),
		path.join(BGFX_DIR, "examples", _name, "**.h"),
	}

	removefiles {
		path.join(BGFX_DIR, "examples", _name, "**.bin.h"),
	}

	links {
		"bgfx",
		"example-common",
	}

	if _OPTIONS["with-sdl"] then
		defines { "ENTRY_CONFIG_USE_SDL=1" }
		links   { "SDL2" }

		configuration { "x32", "windows" }
			libdirs { "$(SDL2_DIR)/lib/x86" }

		configuration { "x64", "windows" }
			libdirs { "$(SDL2_DIR)/lib/x64" }

		configuration {}
	end

	if _OPTIONS["with-glfw"] then
		defines { "ENTRY_CONFIG_USE_GLFW=1" }
		links   {
			"glfw3"
		}

		configuration { "linux or freebsd" }
			links {
				"Xrandr",
				"Xinerama",
				"Xi",
				"Xxf86vm",
				"Xcursor",
			}

		configuration { "osx" }
			linkoptions {
				"-framework CoreVideo",
				"-framework IOKit",
			}

		configuration {}
	end

	configuration { "vs*" }
		linkoptions {
			"/ignore:4199", -- LNK4199: /DELAYLOAD:*.dll ignored; no imports found from *.dll
		}
		links { -- this is needed only for testing with GLES2/3 on Windows with VS2008
			"DelayImp",
		}

	configuration { "vs201*" }
		linkoptions { -- this is needed only for testing with GLES2/3 on Windows with VS201x
			"/DELAYLOAD:\"libEGL.dll\"",
			"/DELAYLOAD:\"libGLESv2.dll\"",
		}

	configuration { "mingw*" }
		targetextension ".exe"

	configuration { "vs20* or mingw*" }
		links {
			"gdi32",
			"psapi",
		}

	configuration { "winphone8* or winstore8*" }
		removelinks {
			"DelayImp",
			"gdi32",
			"psapi"
		}
		links {
			"d3d11",
			"dxgi"
		}
		linkoptions {
			"/ignore:4264" -- LNK4264: archiving object file compiled with /ZW into a static library; note that when authoring Windows Runtime types it is not recommended to link with a static library that contains Windows Runtime metadata
		}

	-- WinRT targets need their own output directories or build files stomp over each other
	configuration { "x32", "winphone8* or winstore8*" }
		targetdir (path.join(BGFX_BUILD_DIR, "win32_" .. _ACTION, "bin", _name))
		objdir (path.join(BGFX_BUILD_DIR, "win32_" .. _ACTION, "obj", _name))

	configuration { "x64", "winphone8* or winstore8*" }
		targetdir (path.join(BGFX_BUILD_DIR, "win64_" .. _ACTION, "bin", _name))
		objdir (path.join(BGFX_BUILD_DIR, "win64_" .. _ACTION, "obj", _name))

	configuration { "ARM", "winphone8* or winstore8*" }
		targetdir (path.join(BGFX_BUILD_DIR, "arm_" .. _ACTION, "bin", _name))
		objdir (path.join(BGFX_BUILD_DIR, "arm_" .. _ACTION, "obj", _name))

	configuration { "mingw-clang" }
		kind "ConsoleApp"

	configuration { "android*" }
		kind "ConsoleApp"
		targetextension ".so"
		linkoptions {
			"-shared",
		}
		links {
			"EGL",
			"GLESv2",
		}

	configuration { "nacl*" }
		kind "ConsoleApp"
		targetextension ".nexe"
		links {
			"ppapi",
			"ppapi_gles2",
			"pthread",
		}

	configuration { "pnacl" }
		kind "ConsoleApp"
		targetextension ".pexe"
		links {
			"ppapi",
			"ppapi_gles2",
			"pthread",
		}

	configuration { "asmjs" }
		kind "ConsoleApp"
		targetextension ".bc"

	configuration { "linux-* or freebsd" }
		links {
			"X11",
			"GL",
			"pthread",
		}

	configuration { "rpi" }
		links {
			"X11",
			"GLESv2",
			"EGL",
			"bcm_host",
			"vcos",
			"vchiq_arm",
			"pthread",
		}

	configuration { "osx" }
		files {
			path.join(BGFX_DIR, "examples/common/**.mm"),
		}
		links {
			"Cocoa.framework",
			"OpenGL.framework",
		}

	configuration { "ios*" }
		kind "ConsoleApp"
		files {
			path.join(BGFX_DIR, "examples/common/**.mm"),
		}
		linkoptions {
			"-framework CoreFoundation",
			"-framework Foundation",
			"-framework OpenGLES",
			"-framework UIKit",
			"-framework QuartzCore",
		}

	configuration { "xcode4", "ios" }
		kind "WindowedApp"
		files {
			path.join(BGFX_DIR, "examples/runtime/iOS-Info.plist"),
		}

	configuration { "qnx*" }
		targetextension ""
		links {
			"EGL",
			"GLESv2",
		}

	configuration {}

	strip()
end








-- strip() function is needed for bgfx example projects
dofile (path.join(BX_DIR, "scripts/toolchain.lua"))
if not toolchain(G_BUILD_DIR, BGFX_THIRD_PARTY_DIR) then
	return -- no action specified
end

dofile "_util.lua"

dofile (path.join(BGFX_DIR, "scripts/bgfx.lua"))

group "examples"
dofile (path.join(BGFX_DIR, "scripts/example-common.lua"))

group "libs"
-- bgfx library
bgfxProject("", "StaticLib", {})
-- bgfx example projects
group "examples"
exampleProject("00-helloworld")
exampleProject("01-cubes")
exampleProject("02-metaballs")
exampleProject("03-raymarch")
exampleProject("04-mesh")
exampleProject("05-instancing")
exampleProject("06-bump")
exampleProject("07-callback")
exampleProject("08-update")
exampleProject("09-hdr")
exampleProject("10-font")
exampleProject("11-fontsdf")
exampleProject("12-lod")
exampleProject("13-stencil")
exampleProject("14-shadowvolumes")
exampleProject("15-shadowmaps-simple")
exampleProject("16-shadowmaps")
exampleProject("17-drawstress")
exampleProject("18-ibl")
exampleProject("19-oit")
exampleProject("20-nanovg")
exampleProject("21-deferred")
exampleProject("22-windows")
exampleProject("23-vectordisplay")
exampleProject("24-nbody")
--[[
if _OPTIONS["with-tools"] then
	group "tools"
	dofile (path.join(BGFX_DIR, "scripts/makedisttex.lua"))
	dofile (path.join(BGFX_DIR, "scripts/shaderc.lua"))
	dofile (path.join(BGFX_DIR, "scripts/texturec.lua"))
	dofile (path.join(BGFX_DIR, "scripts/geometryc.lua"))
end
]]

-- SDL library
--MakeSDL("SDL", {})

-- engine library
--CreateProject("Engine", "StaticLib", {})


function DemoProject(_name)

	project (_name)
		uuid (os.uuid(_name))
		kind "WindowedApp"

	configuration {}

	includedirs {
		path.join(BX_DIR,   "include"),
		path.join(BGFX_DIR, "include"),
		path.join(BGFX_DIR, "3rdparty"),
		path.join(BGFX_DIR, "examples/common"),
		G_ENGINE_DIR,
	}

	files {
		path.join(G_DEMOS_DIR, _name, "**.c"),
		path.join(G_DEMOS_DIR, _name, "**.cpp"),
		path.join(G_DEMOS_DIR, _name, "**.h"),
		path.join(G_DEMOS_DIR, _name, "**.sc"),
	}

	removefiles {
		path.join(G_DEMOS_DIR, _name, "**.bin.h"),
	}

	links {
		"bgfx",
		"example-common",
	}

	configuration { "vs*" }
		--[[linkoptions {
			"/ignore:4199", -- LNK4199: /DELAYLOAD:*.dll ignored; no imports found from *.dll
		}
		links { -- this is needed only for testing with GLES2/3 on Windows with VS2008
			"DelayImp",
		}
		]]

	configuration { "vs201*" }
		linkoptions { -- this is needed only for testing with GLES2/3 on Windows with VS201x
			"/DELAYLOAD:\"libEGL.dll\"",
			"/DELAYLOAD:\"libGLESv2.dll\"",
		}

	configuration { "vs20* or mingw*" }
		links {
			"gdi32",
			"psapi",
		}

	-- WinRT targets need their own output directories or build files stomp over each other
	configuration { "x32" }
		--targetdir (path.join(BGFX_BUILD_DIR, "win32_" .. _ACTION, "bin", _name))
		--objdir (path.join(BGFX_BUILD_DIR, "win32_" .. _ACTION, "obj", _name))
		targetdir (path.join(G_BUILD_DIR, ".bin"))
		objdir (path.join(G_BUILD_DIR, ".bin"))

	configuration { "x64" }
		--targetdir (path.join(BGFX_BUILD_DIR, "win64_" .. _ACTION, "bin", _name))
		--objdir (path.join(BGFX_BUILD_DIR, "win64_" .. _ACTION, "obj", _name))
		targetdir (path.join(G_BUILD_DIR, ".bin"))
		objdir (path.join(G_BUILD_DIR, ".bin"))

	configuration {}

	strip()
end

configuration {} -- reset configuration

group "libs"
	
project ("Engine")

	uuid (os.uuid("Engine"))
	kind "StaticLib"

	defines {
		"WIN32", "_WIN32",
		"_HAS_EXCEPTIONS=0",
		"_HAS_ITERATOR_DEBUGGING=0",
		"_SCL_SECURE=0", "_SECURE_SCL=0",
		"_SCL_SECURE_NO_WARNINGS",
		"_CRT_SECURE_NO_WARNINGS",
		"_CRT_SECURE_NO_DEPRECATE",
		"MX_AUTOLINK=0",
	}

	files {
		path.join(G_ENGINE_DIR, "**.h"),
		path.join(G_ENGINE_DIR, "**.cpp"),
	}
	removefiles {
		path.join(G_ENGINE_DIR, "ImGui/**.cpp"),
	}

	includedirs {
		G_ENGINE_DIR,
		--path.join(G_ROOT_DIR, "stb"),
	}

	links {
		"gdi32",
		"psapi",
		"winmm",	-- timeGetTime()
		"Dbghelp",	-- MiniDumpWriteDump()
		"comctl32", "Imm32",

		"DxErr", "dxgi", "dxguid", "d3d11", "d3dcompiler",
	}

	configuration { "vs*" }
		linkoptions {
			"/ignore:4199", -- LNK4199: /DELAYLOAD:*.dll ignored; no imports found from *.dll
			"/ignore:4221", -- LNK4221: This object file does not define any previously undefined public symbols, so it will not be used by any link operation that consumes this library
		}
	
	configuration { "Debug" }
		links {
			"d3dx11d",
		}
		targetsuffix "-Debug"

	configuration { "Release" }
		flags {
			"OptimizeSpeed",
		}
		links {
			"d3dx11",
		}
		targetsuffix "-Release"

	configuration { "x32", "vs*" }
		targetdir (path.join(G_BUILD_DIR, "win32_" .. _ACTION, "bin"))
		objdir (path.join(G_BUILD_DIR, "win32_" .. _ACTION, "obj"))
		libdirs {
			"$(DXSDK_DIR)/lib/x86",
			--path.join(G_SDL_DIR, "libs/x86"),
		}

	configuration { "x64", "vs*" }
		--defines { "_WIN64" }
		targetdir (path.join(G_BUILD_DIR, "win64_" .. _ACTION, "bin"))
		objdir (path.join(G_BUILD_DIR, "win64_" .. _ACTION, "obj"))
		libdirs {
			"$(DXSDK_DIR)/lib/x64",
			--path.join(G_SDL_DIR, "libs/x64"),
		}

	configuration {} -- reset configuration



group "demos"

project ("BSP-CSG")

	uuid (os.uuid("BSP-CSG"))
	kind "WindowedApp"

	defines {
		"WIN32", "_WIN32",
		"_HAS_EXCEPTIONS=0",
		"_HAS_ITERATOR_DEBUGGING=0",
		"_SCL_SECURE=0", "_SECURE_SCL=0",
		"_SCL_SECURE_NO_WARNINGS",
		"_CRT_SECURE_NO_WARNINGS",
		"_CRT_SECURE_NO_DEPRECATE",
		"MX_AUTOLINK=0",
	}

	files {
		path.join(G_ROOT_DIR, "_Demos/**.**"),
	}

	includedirs {
		path.join(BX_DIR,   "include"),
		path.join(BGFX_DIR, "include"),
		path.join(BGFX_DIR, "3rdparty"),
		path.join(BGFX_DIR, "examples/common"),
		--path.join(BGFX_DIR, G_SDL_DIR, "include"),
		G_ENGINE_DIR,
	}

	links {
		-- Windows
		"gdi32",
		"psapi",
		"winmm",	-- timeGetTime()
		"Dbghelp",	-- MiniDumpWriteDump()
		"comctl32", "Imm32",

		--"SDL2",

		-- bgfx
		"bgfx",
		"example-common",

		-- Direct3D
		"DxErr", "dxgi", "dxguid", "d3d11", "d3dcompiler",

		-- Engine
		"Engine",
	}
	
	debugdir (path.join(G_DEMOS_DIR, "runtime"))

	configuration { "vs*" }
		linkoptions {
			"/ignore:4199", -- LNK4199: /DELAYLOAD:*.dll ignored; no imports found from *.dll
			"/ignore:4221", -- LNK4221: This object file does not define any previously undefined public symbols, so it will not be used by any link operation that consumes this library
		}
	
	configuration { "Debug" }
		defines {
			"BGFX_CONFIG_DEBUG=1",
		}
		libdirs {
		}
		links {
			"d3dx11d",
		}
		targetsuffix "-Debug"

	configuration { "Release" }
		flags {
			"OptimizeSpeed",
		}
		links {
			"d3dx11",
		}
		targetsuffix "-Release"

	configuration { "x32", "vs*" }
		targetdir (path.join(G_BINARIES_DIR, "x86"))
		objdir (path.join(G_BUILD_DIR, "win32_" .. _ACTION, "obj"))
		libdirs {
			path.join(G_BUILD_DIR, "win32_" .. _ACTION, "bin"),
			"$(DXSDK_DIR)/lib/x86",
		}

	configuration { "x64", "vs*" }
		--defines { "_WIN64" }
		targetdir (path.join(G_BINARIES_DIR, "x64"))
		objdir (path.join(G_BUILD_DIR, "win64_" .. _ACTION, "obj"))
		libdirs {
			path.join(G_BUILD_DIR, "win64_" .. _ACTION, "bin"),
			"$(DXSDK_DIR)/lib/x64",
		}

	configuration {} -- reset configuration


startproject "BSP-CSG"
