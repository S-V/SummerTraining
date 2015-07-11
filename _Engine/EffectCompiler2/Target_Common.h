#pragma once

#include <Core/Text/Preprocessor.h>
#include "Effect_Compiler.h"
#include <ShaderCompiler/ShaderCompiler.h>

using namespace Shaders;

//enum UIWidget
//{
//	UI_ComboBox,
//};

// high-level render state
struct FxStateBlockDescription : NamedObject
{
	RGBAf	blendFactorRGBA;
	UINT32	sampleMask;
	String	blendState;
	String	depthStencilState;
	UINT8	stencilRef;
	String	rasterizerState;
public:
	mxDECLARE_CLASS(FxStateBlockDescription, NamedObject);
	mxDECLARE_REFLECTION;
	FxStateBlockDescription();
};

struct FxColorTargetDescription : FxRenderTargetBase
{
	PixelFormatT		format;
public:
	mxDECLARE_CLASS(FxColorTargetDescription, FxRenderTargetBase);
	mxDECLARE_REFLECTION;
	FxColorTargetDescription();
};

struct FxDepthTargetDescription : FxRenderTargetBase
{
	DepthStencilFormatT	format;
	bool				sample;	// Can it be bound as a shader resource?
public:
	mxDECLARE_CLASS(FxDepthTargetDescription, FxRenderTargetBase);
	mxDECLARE_REFLECTION;
	FxDepthTargetDescription();
};

// static shader switches are set by the artist during development;
// they are used to create different material shader variations;
struct FxDefineDescription : public CStruct
{
	String32	name;		// name of the corresponding #define macro in the shader code
	String32	tooltip;	// description
	String32	UIName;		// editor name
	String32	UIWidget;	// editor widget
	String32	defaultValue;
public:
	mxDECLARE_CLASS( FxDefineDescription, CStruct );
	mxDECLARE_REFLECTION;
	FxDefineDescription();
};

struct FxShaderEntryD3D : public CStruct
{
	String	file;	// shader source file name (e.g. "cool_shader.hlsl")
	// shader function entry points
	String	vertex;
	String	hull;
	String	domain;
	String	geometry;
	String	pixel;
public:
	mxDECLARE_CLASS(FxShaderEntryD3D,CStruct);
	mxDECLARE_REFLECTION;
	const String& GetEntryFunction( EShaderType _type ) const;
};
struct FxShaderEntryOGL : public CStruct
{
	String	file;	// shader source file name (e.g. "cool_shader.glsl")
public:
	mxDECLARE_CLASS(FxShaderEntryOGL,CStruct);
	mxDECLARE_REFLECTION;
};

struct FxShaderDescription : public CStruct
{
	String				name;
	String				info;
	FxShaderEntryD3D	D3D;
	FxShaderEntryOGL	OGL;
	TArray< FxDefineDescription >	defines;
	TArray< VertexTypeT >	supportedVertexTypes;
	StringListT			mirrored_uniform_buffers;	// create backing store to allow setting shader constants one by one
public:
	mxDECLARE_CLASS(FxShaderDescription,CStruct);
	mxDECLARE_REFLECTION;
	FxShaderDescription();
};

struct FxResourceDescriptions : public CStruct
{
	// global graphics resources (declarations)
	TArray< FxColorTargetDescription >	color_targets;
	TArray< FxDepthTargetDescription >	depth_targets;

	// immutable render state blocks
	TArray< DepthStencilDescription >	depth_stencil_states;
	TArray< RasterizerDescription >		rasterizer_states;
	TArray< SamplerDescription >		sampler_states;
	TArray< BlendDescription >			blend_states;
	TArray< FxStateBlockDescription >	state_blocks;

public:
	mxDECLARE_CLASS( FxResourceDescriptions, CStruct );
	mxDECLARE_REFLECTION;
	FxResourceDescriptions();
};

// initially parsed data - platform-independent shader library/pipeline
// from which Direct3D- or OpenGL-specific implementations can be built.
struct FxLibraryDescription : public FxResourceDescriptions
{
	// shader programs (imperative code)
	TArray< FxShaderDescription >	shader_programs;

public:
	mxDECLARE_CLASS( FxLibraryDescription, FxResourceDescriptions );
	mxDECLARE_REFLECTION;
	FxLibraryDescription();
};

class IShaderBackend {
public:
};

ERet CreateResourceObjects(
						   const FxResourceDescriptions& library,
						   Clump &clump
						   );

ERet CreateBackingStore(
						const FxLibraryDescription& description,
						Clump& clump
						);

ERet CompileShaderLibrary(
						  const char* filepath,
						  const FxOptions& options,
						  Clump &clump, ShaderCache_d &cache
						  );
ERet CompileShaderLibrary(
						  const FxLibraryDescription& library,
						  const FxOptions& options,
						  AFileInclude& include,
						  Clump &clump, ShaderCache_d &cache
						  );

ERet FxCompileShader(
					 const FxShaderDescription& description,
					 const FxOptions& options,
					 AFileInclude* include,
					 FxShader &shader,
					 ShaderCache_d &ñache,
					 ShaderMetadata &metadata
					 );

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
