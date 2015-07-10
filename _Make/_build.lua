-- Based on bgfx's genie.lua:
-- Copyright 2010-2015 Branimir Karadzic. All rights reserved.
-- License: http://www.opensource.org/licenses/BSD-2-Clause
--

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
G_BINARIES_DIR = "../._Binaries/";
--G_SOLUTION_DIR = G_BUILD_DIR .. action

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
	startproject "example-00-helloworld"

--targetsuffix [Debug]

BGFX_DIR = path.getabsolute("../bgfx/")
BX_DIR = path.getabsolute(path.join(BGFX_DIR, "../bx"))

local BGFX_BUILD_DIR = path.join(BGFX_DIR, ".build")
local BGFX_THIRD_PARTY_DIR = path.join(BGFX_DIR, "3rdparty")


--targetdir(path.join(G_SOLUTION_DIR, "bin"))
--targetdir(path.join(G_BINARIES_DIR, "bin"))
--objdir (path.join(G_SOLUTION_DIR, "obj"))

--printf("BGFX_DIR=%s, BGFX_BUILD_DIR=%s, BGFX_THIRD_PARTY_DIR=%s", BGFX_DIR, BGFX_BUILD_DIR, BGFX_THIRD_PARTY_DIR)

--printf("BGFX_DIR=%s, BX_DIR=%s, BGFX_BUILD_DIR=%s, BGFX_THIRD_PARTY_DIR=%s",BGFX_DIR,BX_DIR, BGFX_BUILD_DIR, BGFX_THIRD_PARTY_DIR)









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

	if _OPTIONS["with-ovr"] then
		links   {
			"winmm",
			"ws2_32",
		}

		-- Check for LibOVR 5.0+
		if os.isdir(path.join(os.getenv("OVR_DIR"), "LibOVR/Lib/Windows/Win32/Debug/VS2012")) then

			configuration { "x32", "Debug" }
				libdirs { path.join("$(OVR_DIR)/LibOVR/Lib/Windows/Win32/Debug", _ACTION) }

			configuration { "x32", "Release" }
				libdirs { path.join("$(OVR_DIR)/LibOVR/Lib/Windows/Win32/Release", _ACTION) }

			configuration { "x64", "Debug" }
				libdirs { path.join("$(OVR_DIR)/LibOVR/Lib/Windows/x64/Debug", _ACTION) }

			configuration { "x64", "Release" }
				libdirs { path.join("$(OVR_DIR)/LibOVR/Lib/Windows/x64/Release", _ACTION) }

			configuration { "x32 or x64" }
				links { "libovr" }
		else
			configuration { "x32" }
				libdirs { path.join("$(OVR_DIR)/LibOVR/Lib/Win32", _ACTION) }

			configuration { "x64" }
				libdirs { path.join("$(OVR_DIR)/LibOVR/Lib/x64", _ACTION) }

			configuration { "x32", "Debug" }
				links { "libovrd" }

			configuration { "x32", "Release" }
				links { "libovr" }

			configuration { "x64", "Debug" }
				links { "libovr64d" }

			configuration { "x64", "Release" }
				links { "libovr64" }
		end

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

dofile (path.join(BGFX_DIR, "scripts/bgfx.lua"))

group "examples"
dofile (path.join(BGFX_DIR, "scripts/example-common.lua"))

group "libs"
bgfxProject("", "StaticLib", {})

--[[
group "examples"
bgfxSampleProject(BGFX_DIR, "00-helloworld")
bgfxSampleProject(BGFX_DIR, "01-cubes")
bgfxSampleProject(BGFX_DIR, "02-metaballs")
bgfxSampleProject(BGFX_DIR, "03-raymarch")
bgfxSampleProject(BGFX_DIR, "04-mesh")
bgfxSampleProject(BGFX_DIR, "05-instancing")
bgfxSampleProject(BGFX_DIR, "06-bump")
bgfxSampleProject(BGFX_DIR, "07-callback")
bgfxSampleProject(BGFX_DIR, "08-update")
bgfxSampleProject(BGFX_DIR, "09-hdr")
bgfxSampleProject(BGFX_DIR, "10-font")
bgfxSampleProject(BGFX_DIR, "11-fontsdf")
bgfxSampleProject(BGFX_DIR, "12-lod")
bgfxSampleProject(BGFX_DIR, "13-stencil")
bgfxSampleProject(BGFX_DIR, "14-shadowvolumes")
bgfxSampleProject(BGFX_DIR, "15-shadowmaps-simple")
bgfxSampleProject(BGFX_DIR, "16-shadowmaps")
bgfxSampleProject(BGFX_DIR, "17-drawstress")
bgfxSampleProject(BGFX_DIR, "18-ibl")
bgfxSampleProject(BGFX_DIR, "19-oit")
bgfxSampleProject(BGFX_DIR, "20-nanovg")
bgfxSampleProject(BGFX_DIR, "21-deferred")
bgfxSampleProject(BGFX_DIR, "22-windows")
bgfxSampleProject(BGFX_DIR, "23-vectordisplay")
bgfxSampleProject(BGFX_DIR, "24-nbody")
--]]

if _OPTIONS["with-tools"] then
	group "tools"
	dofile (path.join(BGFX_DIR, "scripts/makedisttex.lua"))
	dofile (path.join(BGFX_DIR, "scripts/shaderc.lua"))
	dofile (path.join(BGFX_DIR, "scripts/texturec.lua"))
	dofile (path.join(BGFX_DIR, "scripts/geometryc.lua"))
end
