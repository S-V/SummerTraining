#include "Graphics/Graphics_PCH.h"
#pragma hdrstop
#include <Psapi.h>
#include <External/RenderDoc/app/renderdoc_app.h>

#include <Graphics/RenderDoc.h>

#define BX_TRACE(...)	mxNOOP

namespace RenderDoc
{
	bool findModule(const char* _name)
	{
		HANDLE process = GetCurrentProcess();
		DWORD size;
		BOOL result = ::EnumProcessModules(process
			, NULL
			, 0
			, &size
			);
		if (0 != result)
		{
			HMODULE* modules = (HMODULE*)alloca(size);
			result = ::EnumProcessModules(process
				, modules
				, size
				, &size
				);
			if (0 != result)
			{
				char moduleName[MAX_PATH];
				for (uint32_t ii = 0, num = uint32_t(size/sizeof(HMODULE) ); ii < num; ++ii)
				{
					result = ::GetModuleBaseNameA(process
						, modules[ii]
					, moduleName
						, mxCOUNT_OF(moduleName)
						);
					if (0 != result
						&& 0 == stricmp(_name, moduleName) )
					{
						return true;
					}
				}
			}
		}
		return false;
	}
#define RENDERDOC_IMPORT \
	RENDERDOC_IMPORT_FUNC(RENDERDOC_SetLogFile); \
	RENDERDOC_IMPORT_FUNC(RENDERDOC_GetCapture); \
	RENDERDOC_IMPORT_FUNC(RENDERDOC_SetCaptureOptions); \
	RENDERDOC_IMPORT_FUNC(RENDERDOC_SetActiveWindow); \
	RENDERDOC_IMPORT_FUNC(RENDERDOC_TriggerCapture); \
	RENDERDOC_IMPORT_FUNC(RENDERDOC_StartFrameCapture); \
	RENDERDOC_IMPORT_FUNC(RENDERDOC_EndFrameCapture); \
	RENDERDOC_IMPORT_FUNC(RENDERDOC_GetOverlayBits); \
	RENDERDOC_IMPORT_FUNC(RENDERDOC_MaskOverlayBits); \
	RENDERDOC_IMPORT_FUNC(RENDERDOC_SetFocusToggleKeys); \
	RENDERDOC_IMPORT_FUNC(RENDERDOC_SetCaptureKeys); \
	RENDERDOC_IMPORT_FUNC(RENDERDOC_InitRemoteAccess);
#define RENDERDOC_IMPORT_FUNC(_func) p##_func _func
	RENDERDOC_IMPORT
#undef RENDERDOC_IMPORT_FUNC
		pRENDERDOC_GetAPIVersion RENDERDOC_GetAPIVersion;
	void* loadRenderDoc()
	{
		// Skip loading RenderDoc when IntelGPA is present to avoid RenderDoc crash.
		if (findModule((mxARCH_TYPE == mxARCH_32BIT) ? "shimloader32.dll" : "shimloader64.dll") )
		{
			return NULL;
		}
		HMODULE renderdocdll = ::LoadLibraryA("renderdoc.dll");
		if (NULL != renderdocdll)
		{
			RENDERDOC_GetAPIVersion = (pRENDERDOC_GetAPIVersion)::GetProcAddress(renderdocdll, "RENDERDOC_GetAPIVersion");
			if (NULL != RENDERDOC_GetAPIVersion
				&& RENDERDOC_API_VERSION == RENDERDOC_GetAPIVersion() )
			{
#define RENDERDOC_IMPORT_FUNC(_func) \
	_func = (p##_func)::GetProcAddress(renderdocdll, #_func); \
	BX_TRACE("%p " #_func, _func);
				RENDERDOC_IMPORT
#undef RENDERDOC_IMPORT_FUNC
					RENDERDOC_SetLogFile("renderdoc/");
				RENDERDOC_SetFocusToggleKeys(NULL, 0);
				KeyButton captureKey = eKey_F11;
				RENDERDOC_SetCaptureKeys(&captureKey, 1);
				CaptureOptions opt;
				memset(&opt, 0, sizeof(opt) );
				opt.AllowVSync = 1;
				opt.SaveAllInitials = 1;
				RENDERDOC_SetCaptureOptions(&opt);
				uint32_t ident = 0;
				RENDERDOC_InitRemoteAccess(&ident);
				RENDERDOC_MaskOverlayBits(eOverlay_None, eOverlay_None);
			}
			else
			{
				::FreeLibrary(renderdocdll);
				renderdocdll = NULL;
			}
		}
		return renderdocdll;
	}

	void unloadRenderDoc(void* _renderdocdll)
	{
		if (NULL != _renderdocdll)
		{
			::FreeLibrary((HMODULE)_renderdocdll);
		}
	}

}//namespace RenderDoc

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
