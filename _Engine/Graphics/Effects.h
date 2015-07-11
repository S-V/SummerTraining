/*
=============================================================================
	Shader/Effect system.
=============================================================================
*/
#pragma once

#include <Base/Util/Color.h>
#include <Core/Asset.h>
#include <Graphics/Device.h>
#include <Graphics/graphics_programs.h>

/*
=====================================================================
    RENDER TARGETS
=====================================================================
*/
//union FloatOrUInt
//{
//	FLOAT32	float_value;
//	UINT32	uint_value;
//};
// the size of a render target in one dimension (X or Y)
struct FxRenderTargetSize : public CStruct
{
	float		size;		// either absolute or relative
	bool		relative;	// size mode (default = false)
public:
	mxDECLARE_CLASS( FxRenderTargetSize, CStruct );
	mxDECLARE_REFLECTION;
	FxRenderTargetSize()
	{
		size = 0.0f;
		relative = false;
	}
	void SetAbsoluteSize( int absoluteSize )
	{
		size = absoluteSize;
		relative = false;	// dimensions are constant values
	}
	void SetRelativeSize( float relativeSize )
	{
		size = relativeSize;
		relative = true;	// dimensions depend on back buffer size
	}
};

struct FxRenderTargetBase : NamedObject
{
	FxRenderTargetSize	sizeX;	// width
	FxRenderTargetSize	sizeY;	// height
	//UINT8				depth;
public:
	mxDECLARE_CLASS(FxRenderTargetBase, NamedObject);
	mxDECLARE_REFLECTION;
	FxRenderTargetBase();
};

struct FxColorTarget : FxRenderTargetBase
{
	PixelFormatT		format;
	HColorTarget		handle;
public:
	mxDECLARE_CLASS(FxColorTarget, FxRenderTargetBase);
	mxDECLARE_REFLECTION;
	FxColorTarget();
};

struct FxDepthTarget : FxRenderTargetBase
{
	DepthStencilFormatT	format;
	HDepthTarget		handle;
	bool				sample;	// Can it be bound as a shader resource?
public:
	mxDECLARE_CLASS(FxDepthTarget, FxRenderTargetBase);
	mxDECLARE_REFLECTION;
	FxDepthTarget();
};

/*
=====================================================================
    RENDER STATES
=====================================================================
*/
struct FxDepthStencilState : DepthStencilDescription
{
	HDepthStencilState	handle;
public:
	mxDECLARE_CLASS(FxDepthStencilState, DepthStencilDescription);
	mxDECLARE_REFLECTION;
	FxDepthStencilState();
};

struct FxRasterizerState : RasterizerDescription
{
	HRasterizerState	handle;
public:
	mxDECLARE_CLASS(FxRasterizerState, RasterizerDescription);
	mxDECLARE_REFLECTION;
	FxRasterizerState();
};

struct FxSamplerState : SamplerDescription
{
	HSamplerState	handle;
public:
	mxDECLARE_CLASS(FxSamplerState, SamplerDescription);
	mxDECLARE_REFLECTION;
	FxSamplerState();
};

struct FxBlendState : BlendDescription
{
	HBlendState		handle;
public:
	mxDECLARE_CLASS(FxBlendState, BlendDescription);
	mxDECLARE_REFLECTION;
	FxBlendState();
};

struct FxStateBlock : NamedObject
{
	RGBAf				blendFactor;
	UINT32				sampleMask;
	HBlendState			blendState;

	HRasterizerState	rasterizerState;

	HDepthStencilState	depthStencilState;
	UINT8				stencilRef;
public:
	mxDECLARE_CLASS(FxStateBlock, NamedObject);
	mxDECLARE_REFLECTION;
	FxStateBlock();
};

struct FxRenderResources : CStruct
{
	TArray< FxColorTarget* >	color_targets;	// sorted by name
	TArray< FxDepthTarget* >	depth_targets;	// sorted by name
	TArray< FxDepthStencilState* >	depth_stencil_states;	// sorted by name
	TArray< FxRasterizerState* >	rasterizer_states;	// sorted by name
	TArray< FxSamplerState* >		sampler_states;	// sorted by name
	TArray< FxBlendState* >			blend_states;	// sorted by name
	TArray< FxStateBlock* >			state_blocks;	// sorted by name
public:
	mxDECLARE_CLASS(FxRenderResources, CStruct);
	mxDECLARE_REFLECTION;
};

//=====================================================================
//	UTILITY FUNCTIONS (SHOULD BE AVOIDED IN PERFORMANCE CRITICAL CODE)
//=====================================================================

ERet FxUtil_LoadShaderLibrary( AStreamReader& stream, Clump &clump, const Resolution& screen );

FxCBufferBinding* FxSlow_FindCBufferBinding( FxShader* shader, const char* name );
FxTextureBinding* FxSlow_FindTextureBinding( FxShader* shader, const char* name );

ERet FxSlow_UpdateUBO(HContext _context, FxShader* shader, const char* UBO, const void* data, int size);

// similar to glGetUniformLocation()
HUniform FxGetUniform( FxShader* shader, const char* name );
ERet FxSetUniform( FxShader* shader, HUniform handle, const void* data );

ERet FxSlow_SetResource( FxShader* shader, const char* name, HResource source, HSamplerState sampler );
ERet FxSlow_SetUniform( FxShader* shader, const char* name, const void* data );
// flushes all changed constant buffers to the GPU
ERet FxSlow_Commit( HContext _context, FxShader* shader );

ERet FxSlow_SetDepthStencilState( HContext _context, const Clump& clump, const char* name, UINT8 stencilRef = 0 );
ERet FxSlow_SetRasterizerState( HContext _context, const Clump& clump, const char* name );
ERet FxSlow_SetBlendState( HContext _context, const Clump& clump, const char* name, const float* blendFactor = NULL, UINT32 sampleMask = ~0 );
ERet FxSlow_SetRenderState( HContext _context, const Clump& clump, const char* name );

void FxSetRenderState( HContext _context, const FxStateBlock& _state );

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
