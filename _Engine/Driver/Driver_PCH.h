// This is a precompiled header.  Include a bunch of common stuff.
// This is kind of ugly in that it adds a bunch of dependency where it isn't needed.
// But on balance, the compile time is much lower (even incrementally) once the precompiled
// headers contain these headers.

#ifndef MX_DRIVER_PCH
#define MX_DRIVER_PCH

#include <Base/Base.h>

#include <SDL.h>
#include <SDL_syswm.h>
#if MX_AUTOLINK
#pragma comment( lib, "SDL2.lib" )
#endif //MX_AUTOLINK

#endif // MX_DRIVER_PCH