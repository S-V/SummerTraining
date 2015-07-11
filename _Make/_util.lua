-- Useful functions for creating library and executable projects.

function CreateProject(_name, _kind, _defines)

	project (_name)
		uuid (os.uuid(_name))
		kind (_kind)

		includedirs {
			path.join(G_SDL_DIR, "include"),
			path.join(BGFX_DIR, "3rdparty"),
			path.join(BGFX_DIR, "../bx/include"),
			G_ENGINE_DIR,
		}

		defines {
			_defines,
		}

		configuration { "Debug" }
			defines {
				"BGFX_CONFIG_DEBUG=1",
			}
			links {
				"sdldebug",
			}

		configuration { "Release" }
			links {
				"sdlrelease",
			}

		configuration { "x32", "vs*" }
			targetdir (path.join(G_BUILD_DIR, "win32_" .. _ACTION, "bin"))
			objdir (path.join(G_BUILD_DIR, "win32_" .. _ACTION, "obj"))
			debugdir (targetdir())
			libdirs {
				targetdir(),
				"$(DXSDK_DIR)/lib/x86",
			}

		configuration { "x64", "vs*" }
			defines { "_WIN64" }
			targetdir (path.join(G_BUILD_DIR, "win64_" .. _ACTION, "bin"))
			objdir (path.join(G_BUILD_DIR, "win64_" .. _ACTION, "obj"))
			--debugdir (targetdir())
			debugdir (path.join(G_BUILD_DIR, "debug"))
			libdirs {
				targetdir(),
				"$(DXSDK_DIR)/lib/x64",
			}
			
		configuration { "vs2008" }
			includedirs {
				"$(DXSDK_DIR)/include",
			}

		configuration {}

		includedirs {
			path.join(BGFX_DIR, "include"),
			path.join(BGFX_DIR, "include"),
		}

		files {
			path.join(G_ENGINE_DIR, "**.h"),
			path.join(G_ENGINE_DIR, "**.cpp"),
			--path.join(G_ENGINE_DIR, "/**.h"),
		}

		removefiles {
			path.join(G_ENGINE_DIR, "Graphics/**.cpp"),
			path.join(G_ENGINE_DIR, "Renderer/**.h"),
			path.join(G_ENGINE_DIR, "Renderer/**.cpp"),
		}

		configuration {}

		copyLib()
end

function MakeSDL(_name, _defines)

	project (_name)
		uuid (os.uuid(_name))
		kind ("StaticLib")

		includedirs {
			G_SDL_DIR,
		}

		defines {
			_defines,
		}

		configuration { "Debug" }
			defines {
				"_DEBUG=1",
			}

		configuration { "vs2008" }
			includedirs {
				"$(DXSDK_DIR)/include",
			}

		configuration {}

		includedirs {
			G_SDL_DIR,
			path.join(G_SDL_DIR, "include"),
		}

		files {
			path.join(G_SDL_DIR, "**.h"),
			path.join(G_SDL_DIR, "**.c"),
			path.join(G_SDL_DIR, "**.cpp"),
		}

		removefiles {
			path.join(G_SDL_DIR, "src/core/winrt/*.cpp"),
			path.join(G_SDL_DIR, "src/main/winrt/*.cpp"),
			
			path.join(G_SDL_DIR, "src/thread/stdcpp/*.cpp"),

			path.join(G_SDL_DIR, "src/audio/**.c"), path.join(G_SDL_DIR, "src/audio/**.cpp"),
			path.join(G_SDL_DIR, "src/joystick/**.c"),
			path.join(G_SDL_DIR, "src/video/**.c"),

			path.join(G_SDL_DIR, "src/audio/psp/**.c"),
			path.join(G_SDL_DIR, "src/main/psp/**.c"),
			path.join(G_SDL_DIR, "src/timer/psp/**.c"),
			path.join(G_SDL_DIR, "src/thread/psp/**.c"),
			
			path.join(G_SDL_DIR, "src/thread/pthread/**.c"),
		}

		configuration {}

		copyLib()
end
