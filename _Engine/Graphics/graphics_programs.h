/*
=============================================================================
	Shader/Effect system.
=============================================================================
*/
#pragma once

#include <Core/Asset.h>
#include <Graphics/Device.h>

#define mxUSE_BINARY_EFFECT_FILE_FORMAT	(1)

ERet ShaderSystem_RegisterClasses();

// uniform handle (for use with FxShader)
mxDECLARE_16BIT_HANDLE(HUniform);

/*
=====================================================================
    SHADERS
=====================================================================
*/
#if 0
enum EShaderQuality
{
	SQ_Low,
	SQ_Medium,
	SQ_Highest,
};
// from lowest to highest
struct UsageFrequency {
	enum Enum {
		PerFrame,
		PerScene,	// view
		PerObject,	// instance
		PerBatch,	// primitive group
		PerPrimitive,
		PerVertex,
		PerPixel,
	};
};
#endif

typedef TBuffer< Float4 >	ParameterBuffer;

// allows setting shader constants one by one
// at the cost of additional RAM and CPU usage
struct FxBackingStore : CStruct {
	ParameterBuffer	data;
	bool			dirty;
public:
	FxBackingStore();
	mxDECLARE_CLASS(FxBackingStore, CStruct);
	mxDECLARE_REFLECTION;
};

// describes a uniform parameter in a shader constant buffer
struct FxUniform : NamedObject {
	UINT32	offset;
	UINT16	size;
	UINT16	semantic;
public:
	mxDECLARE_CLASS(FxUniform, NamedObject);
	mxDECLARE_REFLECTION;
	FxUniform();
};

// represents a shader constant buffer resource
struct FxCBuffer : NamedObject {
	UINT32					size;			// size of this buffer in bytes
	HBuffer					handle;			// id of the low-level device object
	TBuffer< FxUniform >	uniforms;		// basic reflection for member fields
	FxBackingStore 			backingStore;	// optional (empty if this is a global UBO)
public:
	mxDECLARE_CLASS(FxCBuffer, NamedObject);
	mxDECLARE_REFLECTION;
	FxCBuffer();
};

struct FxResource : NamedObject {
	HResource		texture;
	HSamplerState	sampler;
public:
	mxDECLARE_CLASS(FxResource, NamedObject);
	mxDECLARE_REFLECTION;
	FxResource();
};

UINT32 CreateParameterBuffer( const FxCBuffer* uniformBuffer, ParameterBuffer *parameterBuffer );

// contains an index of an input shader resource and its binding point
struct FxCBufferBinding : CStruct {
	UINT8			slot;	// input slot (register index)
	UINT8			id;		// index into array of constant buffers
public:
	mxDECLARE_CLASS(FxCBufferBinding, CStruct);
	mxDECLARE_REFLECTION;
	FxCBufferBinding();
};

struct FxTextureBinding : CStruct {
	UINT8			slot;	// input slot (register index)
	UINT8			id;		// index into array of shader resources
public:
	mxDECLARE_CLASS(FxTextureBinding, CStruct);
	mxDECLARE_REFLECTION;
	FxTextureBinding();
};

struct FxDefine : CStruct
{
	String64	name;
	String64	value;
public:
	mxDECLARE_CLASS( FxDefine, CStruct );
	mxDECLARE_REFLECTION;
};

struct FxShaderPin : NamedObject {
	UINT32	mask;
	bool	enabled;
public:
	mxDECLARE_CLASS(FxShaderPin, NamedObject);
	mxDECLARE_REFLECTION;
};

// Represents a shader program.
struct FxShader : public NamedObject
{
	// shader program instances
	TBuffer< HProgram >		programs;

	// indices into the above array
	TBuffer< UINT32 >		permutations;

	// shader resource bindings - they are the same for all shader instances
	TBuffer< FxCBufferBinding >	CBs;	// constant buffer bindings
	TBuffer< FxTextureBinding >	SRs;	// shader resource bindings

	TBuffer< FxShaderPin >	pins;

	TBuffer< FxCBuffer >	localCBs;
	TBuffer< FxResource >	localSRs;

public:
	mxDECLARE_CLASS(FxShader,NamedObject);
	mxDECLARE_REFLECTION;
	FxShader();

	static AssetTypeT GetAssetType() { return AssetTypes::SHADER; }

	static ERet LoadData( Assets::LoadContext2 & context );
	static ERet Finalize( Assets::LoadContext2 & context );
	static void BringOut( Assets::LoadContext2 & context );
	static void FreeData( Assets::LoadContext2 & context );
};

// arrays of all created shaders and programs for garbage collection
struct FxShaderHandles : public CStruct
{
	TArray< HShader >	handles[ ShaderTypeCount ];
public:
	mxDECLARE_CLASS(FxShaderHandles,CStruct);
	mxDECLARE_REFLECTION;
};
struct FxProgramHandles : public CStruct
{
	TArray< HProgram >	handles;
public:
	mxDECLARE_CLASS(FxProgramHandles,CStruct);
	mxDECLARE_REFLECTION;
};

struct ShaderHeader
{
	UINT32	fourCC;
};

/*
=====================================================================
    SHADER CACHE
=====================================================================
*/

// the _d prefix means that this struct is primarily meant for storing on disk
// and only temporarily being loaded into memory during initialization
struct CachedShader_d : CStruct
{
	ByteBuffer32	code;	// compiled shader code
public:
	mxDECLARE_CLASS(CachedShader_d, CStruct);
	mxDECLARE_REFLECTION;
};
struct CachedProgram_d : CStruct
{
	ProgramDescription	pd;	// (-1) - null shader
	ProgramBindingsOGL	gl;	// OpenGL only
public:
	mxDECLARE_CLASS(CachedProgram_d, CStruct);
	mxDECLARE_REFLECTION;
};

struct FxShaderStats
{
	UINT16	numDuplicates[ ShaderTypeCount ];
};

enum { SHADER_CACHE_ALIGN = 4 };

#pragma pack (push,1)
struct CacheHeader_d
{
	UINT32		fourCC;		//4 "SHIT"
	UINT32		target;		//4 "D3D " or "OGL "
	//FileTimeT	timeStamp;
	//UINT32		checksum;
	//UINT32		flags;
	UINT32		bufferSize;	//4 maximum size of buffer for temporary allocations
	UINT16		numShaders[ ShaderTypeCount ];	//10
	UINT16		numPrograms;//2
	UINT8		_pad32[8];
public:
	CacheHeader_d();
};
ASSERT_SIZEOF(CacheHeader_d, 32);

struct FxLibraryHeader_d
{
	UINT32	magicNumber;	// FX_Library_MAGIC
	UINT32	runtimeSize;
	UINT32	_pad0;
	UINT32	_pad1;
};
ASSERT_SIZEOF(FxLibraryHeader_d, 16);
#pragma pack (pop)

struct ShaderCache_d : CStruct
{
	TArray< CachedShader_d >	m_shaders[ ShaderTypeCount ];
	TArray< CachedProgram_d >	m_programs;
	FxShaderStats				m_stats;
public:
	mxDECLARE_CLASS(ShaderCache_d, CStruct);
	mxDECLARE_REFLECTION;
	ShaderCache_d();

	UINT32 AddShaderCode( EShaderType type, const void* data, UINT32 size );
	ERet SaveToStream( AStreamWriter &stream ) const;
	void Clear();

	UINT32 NumShaders() const;

	// to create shaders, we don't need to instantiate ShaderCache
	static ERet CreateShaders(
		const CacheHeader_d& header,
		AStreamReader& stream,
		FxShaderHandles &shaders
	);
	static ERet CreatePrograms(
		const CacheHeader_d& header,
		const FxShaderHandles& shaders,
		AStreamReader& stream,
		FxProgramHandles &programs
	);
};

enum {
	FX_Library_MAGIC	= '41XF',	// effect system
	FX_Shaders_MAGIC	= 'TIHS',	// shader cache
	FX_OpenGL_MAGIC		= ' LGO',
	FX_Direct3D_MAGIC	= ' D3D',
};

/*
=====================================================================
    FUNCTIONS
=====================================================================
*/

inline void FxApplyShaderState( llgl::DrawCall &batch, const FxShader& shader, UINT32 shaderIndex = 0 )
{
	// apply shader state, bind default shader resources
	const UINT32 numCBs = shader.CBs.Num();
	for( UINT32 iCB = 0; iCB < numCBs; iCB++ )
	{
		const FxCBufferBinding& binding = shader.CBs[ iCB ];
		const FxCBuffer& cbuffer = shader.localCBs[ binding.id ];
		//batch.CBs[ binding.slot ] = binding.handle;
		batch.CBs[ binding.slot ] = cbuffer.handle;
	}
	const UINT32 numTSs = shader.SRs.Num();
	for( UINT32 iTS = 0; iTS < numTSs; iTS++ )
	{
		const FxTextureBinding& binding = shader.SRs[ iTS ];
		const FxResource& resource = shader.localSRs[ binding.id ];
		//batch.SRs[ binding.slot ] = binding.texture;
		//batch.SSs[ binding.slot ] = binding.sampler;
		batch.SRs[ binding.slot ] = resource.texture;
		batch.SSs[ binding.slot ] = resource.sampler;
	}
	batch.program = shader.programs[ shaderIndex ];
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
