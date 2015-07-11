#pragma once

#include <ImGui/imgui.h>
#if MX_AUTOLINK
#pragma comment( lib, "ImGui.lib" )
#endif //MX_AUTOLINK

#include <Core/ObjectModel.h>
#include <Graphics/Device.h>
#include <Graphics/Effects.h>
#include <Graphics/Utils.h>
#include <Renderer/Renderer.h>

class GUI_Renderer : public TGlobal< GUI_Renderer >
{
	HBuffer			m_vertexBuffer;
	HInputLayout	m_vertexLayout;
	UINT32			m_bufferSize;
	HTexture		m_fontTexture;
	TPtr< Clump >	m_clump;
public:
	GUI_Renderer();
	ERet Initialize( Clump* clump );
	void Shutdown();

	void Update();

	// begin GUI rendering
	void Start_Rendering();

	// all of GUI rendering code should be here...

	// flush
	void Finish_Rendering();

public_internal:
	void RenderDrawLists( ImDrawList** const cmd_lists, int cmd_lists_count );
};

void ImGui_DrawPropertyGrid( void * _memory, const mxType& _type, const char* name );

ImVec2 ImGui_GetWindowPosition( const Float3& worldPosition, const SceneView& sceneView );

void ImGui_Window_Text_Float3( const Float3& xyz );
