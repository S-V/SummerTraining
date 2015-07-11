#include <Base/Base.h>
#include <Core/Core.h>
#include <Driver/Driver.h>
#pragma hdrstop
#include "ImGUI_Renderer.h"

GUI_Renderer::GUI_Renderer()
{
}

// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
// - try adjusting ImGui::GetIO().PixelCenterOffset to 0.5f or 0.375f
static void ImImpl_RenderDrawLists(ImDrawList** const cmd_lists, int cmd_lists_count)
{
	GUI_Renderer::Get().RenderDrawLists( cmd_lists, cmd_lists_count );
}

ERet GUI_Renderer::Initialize( Clump* clump )
{
	m_clump = clump;

	// see ImDrawVert
	VertexDescription	vertexDesc;
	{
		vertexDesc.Begin();
		vertexDesc.Add(AttributeType::Float, 2, VertexAttribute::Position, false);
		vertexDesc.Add(AttributeType::Float, 2, VertexAttribute::TexCoord0, false);
		vertexDesc.Add(AttributeType::UByte, 4, VertexAttribute::Color0, true);
		vertexDesc.End();
	}
	m_vertexLayout = llgl::CreateInputLayout( vertexDesc, "GUIVertex" );

	const UINT32 maxVertices = 16384;
	m_bufferSize = maxVertices * sizeof(ImDrawVert);
	m_vertexBuffer = llgl::CreateBuffer( Buffer_Vertex, m_bufferSize );

	int screenWidth, screenHeight;
	WindowsDriver::GetWindowSize( &screenWidth, &screenHeight );

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2( screenWidth, screenHeight );	// Display size, in pixels. For clamping windows positions.
	io.DeltaTime = 1.0f/60.0f;	// Time elapsed since last frame, in seconds (in this sample app we'll override this every frame because our timestep is variable)
	// Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
	io.KeyMap[ImGuiKey_Tab] = KEY_Tab;
	io.KeyMap[ImGuiKey_LeftArrow] = KEY_Left;
	io.KeyMap[ImGuiKey_RightArrow] = KEY_Right;
	io.KeyMap[ImGuiKey_UpArrow] = KEY_Up;
	io.KeyMap[ImGuiKey_DownArrow] = KEY_Down;
	io.KeyMap[ImGuiKey_Home] = KEY_Home;
	io.KeyMap[ImGuiKey_End] = KEY_End;
	io.KeyMap[ImGuiKey_Delete] = KEY_Delete;
	io.KeyMap[ImGuiKey_Backspace] = KEY_Backspace;
	io.KeyMap[ImGuiKey_Enter] = KEY_Enter;
	io.KeyMap[ImGuiKey_Escape] = KEY_Escape;
	io.KeyMap[ImGuiKey_A] = KEY_A;
	io.KeyMap[ImGuiKey_C] = KEY_C;
	io.KeyMap[ImGuiKey_V] = KEY_V;
	io.KeyMap[ImGuiKey_X] = KEY_X;
	io.KeyMap[ImGuiKey_Y] = KEY_Y;
	io.KeyMap[ImGuiKey_Z] = KEY_Z;

	io.RenderDrawListsFn = ImImpl_RenderDrawLists;

	// Create the vertex buffer

	// Load font texture
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	Texture2DDescription	fontTextureDesc;
	IF_DEBUG fontTextureDesc.name.SetReference(Chars("FontTexture"));
	fontTextureDesc.format = PixelFormat::RGBA8;
	fontTextureDesc.width = width;
	fontTextureDesc.height = height;
	fontTextureDesc.numMips = 1;
	m_fontTexture = llgl::CreateTexture2D(fontTextureDesc, pixels);
	//m_fontTexture = llgl::CreateTexture(pixels, width*height*sizeof(UINT32));

	return ALL_OK;
}
void GUI_Renderer::Shutdown()
{
	ImGui::Shutdown();

	llgl::DeleteBuffer(m_vertexBuffer);
	m_vertexBuffer.SetNil();
	llgl::DeleteInputLayout(m_vertexLayout);
	m_vertexLayout.SetNil();
}
void GUI_Renderer::RenderDrawLists( ImDrawList** const cmd_lists, int cmd_lists_count )
{
	HContext renderContext = llgl::GetMainContext();
	gfxMARKER(ImGui);

	size_t total_vtx_count = 0;
	for (int n = 0; n < cmd_lists_count; n++)
		total_vtx_count += cmd_lists[n]->vtx_buffer.size();
	if (total_vtx_count == 0)
		return;

	// Copy and convert all vertices into a single contiguous buffer
	ImDrawVert* vtx_dst = (ImDrawVert*) llgl::MapBuffer( renderContext, m_vertexBuffer, m_bufferSize, Map_Write_Discard );
	for (int n = 0; n < cmd_lists_count; n++)
	{
		const ImDrawList* cmd_list = cmd_lists[n];
		const ImDrawVert* vtx_src = &cmd_list->vtx_buffer[0];
		const size_t vtx_count = cmd_list->vtx_buffer.size();
		memcpy(vtx_dst, vtx_src, vtx_count*sizeof(vtx_src[0]));
		vtx_dst += vtx_count;
	}
	llgl::UnmapBuffer( renderContext, m_vertexBuffer );


	{
		llgl::ViewState	viewState;
		viewState.Reset();
		viewState.colorTargets[0].SetDefault();
		viewState.targetCount = 1;
		viewState.flags = 0;
		llgl::SubmitView(renderContext, viewState);
	}


	// Setup render state: alpha-blending, no face culling, no depth testing
	FxDepthStencilState* pDS_GUI = FindByName<FxDepthStencilState>(*m_clump, "GUI");
	chkRET_IF_NIL(pDS_GUI);
	llgl::SetDepthStencilState(renderContext, pDS_GUI->handle);

	FxRasterizerState* pRS_GUI = FindByName<FxRasterizerState>(*m_clump, "GUI");
	chkRET_IF_NIL(pRS_GUI);
	llgl::SetRasterizerState(renderContext, pRS_GUI->handle);

	FxSamplerState* pSS_GUI = FindByName<FxSamplerState>(*m_clump, "GUI");
	chkRET_IF_NIL(pSS_GUI);

	FxBlendState* pBS_GUI = FindByName<FxBlendState>(*m_clump, "GUI");
	chkRET_IF_NIL(pBS_GUI);
	llgl::SetBlendState(renderContext, pBS_GUI->handle);


	// Setup orthographic projection matrix
	const ImVec2& screenSize = ImGui::GetIO().DisplaySize;
	//const Float4x4 projectionMatrix = Matrix_OrthographicD3D( 0.0f, screenSize.x, 0.0f, screenSize.y, -1.0f, +1.0f );
	//Float4x4 projectionMatrix = Matrix_OrthographicD3D( 0.0f, screenSize.x, screenSize.y, 0.0f, 0.0f, +1.0f );
	//projectionMatrix = Matrix_Identity();
	Float4x4 projectionMatrix = Matrix_OrthographicD3D( screenSize.x, screenSize.y, -1.0f, +1.0f );

	//Float4x4 projectionMatrix2 = Matrix_OrthographicD3D( 100, 100, 0.0f, 10.0f );
	//Float4 res = Matrix_Project(projectionMatrix2,Float4_Set(50,0,50,1));

	//D3DXMatrixOrthoLH();

	const float L = 0.0f;
	const float R = ImGui::GetIO().DisplaySize.x;
	const float B = ImGui::GetIO().DisplaySize.y;
	const float T = 0.0f;

	const float mvp[4][4] = 
	{
		{ 2.0f/(R-L),   0.0f,           0.0f,       0.0f},
		{ 0.0f,         2.0f/(T-B),     0.0f,       0.0f,},
		{ 0.0f,         0.0f,           0.5f,       0.0f },
		{ (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f },
	};
	memcpy(&projectionMatrix,mvp,64);


	FxShader* shader_GUI = FindByName<FxShader>(*m_clump, "GUI");
	chkRET_IF_NIL(shader_GUI);
	FxSlow_SetUniform(shader_GUI,"u_transform",&projectionMatrix);
	FxSlow_SetResource(shader_GUI, "t_texture", llgl::AsResource(m_fontTexture), pSS_GUI->handle);
	FxSlow_Commit(renderContext, shader_GUI);

	// Render command lists

	llgl::DrawCall	batch;
	batch.Clear();

	FxApplyShaderState(batch, *shader_GUI);

	batch.inputLayout = m_vertexLayout;
	batch.topology = Topology::TriangleList;

	batch.VB[0] = m_vertexBuffer;

	int vtx_offset = 0;
	for (int n = 0; n < cmd_lists_count; n++)
	{
		// Render command list
		const ImDrawList* cmd_list = cmd_lists[n];
		for (size_t cmd_i = 0; cmd_i < cmd_list->commands.size(); cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->commands[cmd_i];

			batch.baseVertex = vtx_offset;
			batch.vertexCount = pcmd->vtx_count;
			batch.startIndex = 0;
			batch.indexCount = 0;

			batch.scissor.left	= pcmd->clip_rect.x;
			batch.scissor.top	= pcmd->clip_rect.y;
			batch.scissor.right	= pcmd->clip_rect.z;
			batch.scissor.bottom= pcmd->clip_rect.w;

			llgl::Submit(renderContext, batch);

			vtx_offset += pcmd->vtx_count;
		}
	}
}

void GUI_Renderer::Update()
{
    ImGuiIO& io = ImGui::GetIO();

	io.DeltaTime = WindowsDriver::GetDeltaSeconds();

	if( io.DeltaTime < 1e-4f ) {
		io.DeltaTime = 1e-4f;
	}

    // Setup inputs

	int mouseX, mouseY;
	int mouseState = WindowsDriver::GetMouseState( &mouseX, &mouseY );
	io.MousePos = ImVec2( mouseX, mouseY );

	io.MouseDown[0] = (mouseState & BIT(MouseButton_Left)) != 0;
	io.MouseDown[1] = (mouseState & BIT(MouseButton_Right)) != 0;
	io.MouseDown[2] = (mouseState & BIT(MouseButton_Middle)) != 0;
	io.MouseDown[3] = (mouseState & BIT(MouseButton_X1)) != 0;
	io.MouseDown[4] = (mouseState & BIT(MouseButton_X2)) != 0;

	//io.MouseWheel = 

    const UINT8* keystate = WindowsDriver::GetKeyboardState();
	for( int i = 0; i < MAX_KEYS; i++ ) {
        io.KeysDown[i] = (keystate[i]) != 0;
	}
	const UINT16 modifiers = WindowsDriver::GetModifiers();
	io.KeyAlt = (modifiers & KeyModifier_Alt) != 0;
    io.KeyCtrl = (modifiers & KeyModifier_Ctrl) != 0;
    io.KeyShift = (modifiers & KeyModifier_Shift) != 0;
}

void GUI_Renderer::Start_Rendering()
{
	// Start the frame
    ImGui::NewFrame();
}

void GUI_Renderer::Finish_Rendering()
{
	ImGui::Render();
}

void HACK_ImGui_ProcessTextInput( const char* text )
{
	ImGuiIO& io = ImGui::GetIO();
	for( int i = 0; i < strlen(text); i++ )
	{
		io.AddInputCharacter((ImWchar)text[i]);
	}
}

void ImGui_DrawPropertyGrid( void * _memory, const mxType& _type, const char* name )
{
	switch( _type.m_kind )
	{
	case ETypeKind::Type_Integer :
		{
			INT32 value = GetInteger( _memory, _type.m_size );
			ImGui::InputInt( name, &value );
			PutInteger( _memory, _type.m_size, value );
		}
		break;

	case ETypeKind::Type_Float :
		{
			float value = GetDouble( _memory, _type.m_size );
			ImGui::InputFloat( name, &value );
			PutDouble( _memory, _type.m_size, value );
		}
		break;
	case ETypeKind::Type_Bool :
		{
			bool value = TPODCast<bool>::GetConst( _memory );
			ImGui::Checkbox( name, &value );
			*((bool*)_memory) = value;
		}
		break;

	case ETypeKind::Type_Enum :
		{
			//const mxEnumType& enumType = _type.UpCast< mxEnumType >();
			//const UINT32 enumValue = enumType.m_accessor.Get_Value( _memory );
			//const char* valueName = enumType.GetStringByValue( enumValue );
			//_log << valueName;
		}
		break;

	case ETypeKind::Type_Flags :
		{
			//const mxFlagsType& flagsType = _type.UpCast< mxFlagsType >();
			//String512 temp;
			//Dbg_FlagsToString( _memory, flagsType, temp );
			//_log << temp;
		}
		break;

	case ETypeKind::Type_String :
		{
			//const String & stringReference = TPODCast< String >::GetConst( _memory );
			//_log << stringReference;
		}
		break;

	case ETypeKind::Type_Class :
		{
			const mxClass& classType = _type.UpCast< mxClass >();
			const mxClass* parentType = classType.GetParent();
			while( parentType != nil )
			{
				if( parentType->GetLayout().numFields
					&& ImGui::TreeNode(parentType->GetTypeName()) )
				{
					ImGui_DrawPropertyGrid( _memory, *parentType, parentType->GetTypeName() );
					ImGui::TreePop();
				}
				parentType = parentType->GetParent();
			}
			if (ImGui::TreeNode(name))
			{
				const mxClassLayout& layout = classType.GetLayout();
				for( int fieldIndex = 0 ; fieldIndex < layout.numFields; fieldIndex++ )
				{
					const mxField& field = layout.fields[ fieldIndex ];
					void* memberVarPtr = mxAddByteOffset( _memory, field.offset );
					ImGui_DrawPropertyGrid( memberVarPtr, field.type, field.name );
				}
				ImGui::TreePop();
			}
		}
		break;

	case ETypeKind::Type_Pointer :
		{
			//_log << "(Pointers not impl)";
		}
		break;

	case ETypeKind::Type_AssetId :
		{
			const AssetID& assetId = *static_cast< const AssetID* >( _memory );
			//_log << AssetId_ToChars( assetId );
		}
		break;

	case ETypeKind::Type_ClassId :
		{
			//_log << "(ClassId not impl)";
		}
		break;

	case ETypeKind::Type_UserData :
		{
			//_log << "(UserData not impl)";
		}
		break;

	case ETypeKind::Type_Blob :
		{
			//_log << "(Blobs not impl)";
		}
		break;

	case ETypeKind::Type_Array :
		{
			//_log << "(Arrays not impl)";
		}
		break;

		mxNO_SWITCH_DEFAULT;
	}//switch
}

ImVec2 ImGui_GetWindowPosition( const Float3& worldPosition, const SceneView& sceneView )
{
	const Float4 posH = Matrix_Project( sceneView.viewProjectionMatrix, Float4_Set(worldPosition, 1.0f) );

	// [-1..+1] -> [0..W]
	float x = (posH.x + 1.0f) * (sceneView.viewportWidth * 0.5f);
	// [-1..+1] -> [H..0]
	float y = (1.0f - posH.y) * (sceneView.viewportHeight * 0.5f);

	return ImVec2( x, y );
}

void ImGui_Window_Text_Float3( const Float3& xyz )
{
	String128 buffer;
	StringStream writer(buffer);
	writer << xyz;

	ImGui::Text(writer.GetString().c_str());
}
