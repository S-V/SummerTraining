#include <Graphics/Graphics_PCH.h>
#pragma hdrstop
#include <algorithm>	// stable_sort()
#include <Core/Serialization.h>
#include <Graphics/Effects.h>
#include <TxTSupport/TxTReader.h>
#include <TxTSupport/TxTSerializers.h>

ERet ShaderSystem_RegisterClasses()
{
	{
		Assets::AssetMetaType& shaderType = Assets::gs_assetTypes[AssetTypes::SHADER];
		shaderType.loadData = &FxShader::LoadData;
		shaderType.finalize = &FxShader::Finalize;
		shaderType.bringOut = &FxShader::BringOut;
		shaderType.freeData = &FxShader::FreeData;
	}
	return ALL_OK;
}

ERet FxShader::LoadData( Assets::LoadContext2 & context )
{
	FxShader* shader = static_cast< FxShader* >( context.o );

	ShaderHeader	header;
	mxDO(context.Get(header));

#if mxUSE_BINARY_EFFECT_FILE_FORMAT
	mxDO(Serialization::LoadBinary( context, mxCLASS_OF(*shader), shader ));
#else
	ByteBuffer blob;
	context >> blob;
	MemoryReader reader(blob);
	mxDO(SON::Load( reader, *shader ));	
#endif

	return ALL_OK;
}
ERet FxShader::Finalize( Assets::LoadContext2 & context )
{
	Clump* clump = context.clump;
	FxShader* shader = static_cast< FxShader* >( context.o );

	CacheHeader_d	header;
	mxDO(context.Get( header ));

	// Create local constant buffers and shader resources.
	for( UINT32 iCB = 0; iCB < shader->localCBs.Num(); iCB++ )
	{
		FxCBuffer & ubo = shader->localCBs[ iCB ];
		ubo.handle = llgl::CreateBuffer( Buffer_Uniform, ubo.size, NULL );
	}

	//for( UINT32 iSR = 0; iSR < shader->localSRs.Num(); iSR++ )
	//{
	//	FxResource & resource = shader->localSRs[ iSR ];
	//	resource.texture;
	//	resource.sampler;
	//}

	// Create shader programs.
	FxShaderHandles	shaders;
	mxDO(ShaderCache_d::CreateShaders( header, context, shaders ));

	FxProgramHandles	programs;
	mxDO(ShaderCache_d::CreatePrograms( header, shaders, context, programs ));

	for( UINT32 i = 0; i < shader->programs.Num(); i++ )
	{
		const int programIndex = shader->programs[i].id;
		shader->programs[i] = programs.handles[programIndex];
	}

	return ALL_OK;
}
void FxShader::BringOut( Assets::LoadContext2 & context )
{
	//Clump* clump = static_cast< Clump* >( context.userData );
	//FxShader* shader = static_cast< FxShader* >( context.o );
}
void FxShader::FreeData( Assets::LoadContext2 & context )
{
	Clump* clump = context.clump;
	FxShader* shader = static_cast< FxShader* >( context.o );
	clump->Free( shader );
}

mxDEFINE_CLASS(FxRenderTargetSize);
mxBEGIN_REFLECTION(FxRenderTargetSize)
	mxMEMBER_FIELD(size),
	mxMEMBER_FIELD(relative),
mxEND_REFLECTION;

mxDEFINE_CLASS(FxRenderTargetBase);
mxBEGIN_REFLECTION(FxRenderTargetBase)
	mxMEMBER_FIELD(sizeX),
	mxMEMBER_FIELD(sizeY),
mxEND_REFLECTION;
FxRenderTargetBase::FxRenderTargetBase()
{
}

mxDEFINE_CLASS(FxColorTarget);
mxBEGIN_REFLECTION(FxColorTarget)
	mxMEMBER_FIELD(format),
mxEND_REFLECTION;
FxColorTarget::FxColorTarget()
{
	format = PixelFormat::Unknown;
	handle.SetNil();
}

mxDEFINE_CLASS(FxDepthTarget);
mxBEGIN_REFLECTION(FxDepthTarget)
	mxMEMBER_FIELD(format),
	mxMEMBER_FIELD(sample),
mxEND_REFLECTION;
FxDepthTarget::FxDepthTarget()
{
	format = DepthStencilFormat::D24S8;
	handle.SetNil();
	sample = false;
}

mxDEFINE_CLASS(FxDepthStencilState);
mxBEGIN_REFLECTION(FxDepthStencilState)
	//mxMEMBER_FIELD(info),
mxEND_REFLECTION;
FxDepthStencilState::FxDepthStencilState()
{
	handle.SetNil();
}

mxDEFINE_CLASS(FxRasterizerState);
mxBEGIN_REFLECTION(FxRasterizerState)
	//mxMEMBER_FIELD(info),
mxEND_REFLECTION;
FxRasterizerState::FxRasterizerState()
{
	handle.SetNil();
}

mxDEFINE_CLASS(FxSamplerState);
mxBEGIN_REFLECTION(FxSamplerState)
	//mxMEMBER_FIELD(info),
mxEND_REFLECTION;
FxSamplerState::FxSamplerState()
{
	handle.SetNil();
}

mxDEFINE_CLASS(FxBlendState);
mxBEGIN_REFLECTION(FxBlendState)
	//mxMEMBER_FIELD(info),
mxEND_REFLECTION;
FxBlendState::FxBlendState()
{
	handle.SetNil();
}

mxDEFINE_CLASS(FxStateBlock);
mxBEGIN_REFLECTION(FxStateBlock)
	mxMEMBER_FIELD(blendFactor),
	mxMEMBER_FIELD(sampleMask),
	mxMEMBER_FIELD(blendState),
	mxMEMBER_FIELD(rasterizerState),
	mxMEMBER_FIELD(depthStencilState),
	mxMEMBER_FIELD(stencilRef),	
mxEND_REFLECTION;
FxStateBlock::FxStateBlock()
{
	blendFactor = RGBAf::BLACK;
	blendState.SetNil();
	rasterizerState.SetNil();
	sampleMask = ~0;
	depthStencilState.SetNil();
	stencilRef = 0;
}

mxDEFINE_CLASS(FxRenderResources);
mxBEGIN_REFLECTION(FxRenderResources)
	mxMEMBER_FIELD(color_targets),
	mxMEMBER_FIELD(depth_targets),
	mxMEMBER_FIELD(depth_stencil_states),
	mxMEMBER_FIELD(rasterizer_states),
	mxMEMBER_FIELD(sampler_states),
	mxMEMBER_FIELD(blend_states),
	mxMEMBER_FIELD(state_blocks),
mxEND_REFLECTION;


mxDEFINE_CLASS(FxBackingStore);
mxBEGIN_REFLECTION(FxBackingStore)
	mxMEMBER_FIELD(data),
	mxMEMBER_FIELD(dirty),
mxEND_REFLECTION;
FxBackingStore::FxBackingStore()
{
	dirty = false;
}

mxDEFINE_CLASS(FxUniform);
mxBEGIN_REFLECTION(FxUniform)
	mxMEMBER_FIELD(offset),
	mxMEMBER_FIELD(size),
	mxMEMBER_FIELD(semantic),
mxEND_REFLECTION;
FxUniform::FxUniform()
{
	offset = ~0;
	size = 0;
	semantic = 0;
}

mxDEFINE_CLASS(FxCBuffer);
mxBEGIN_REFLECTION(FxCBuffer)
	mxMEMBER_FIELD(size),
	mxMEMBER_FIELD(handle),
	mxMEMBER_FIELD(uniforms),
	mxMEMBER_FIELD(backingStore),
mxEND_REFLECTION;
FxCBuffer::FxCBuffer()
{
	size = 0;
	handle.SetNil();
}

mxDEFINE_CLASS(FxResource);
mxBEGIN_REFLECTION(FxResource)
	mxMEMBER_FIELD(texture),
	mxMEMBER_FIELD(sampler),
mxEND_REFLECTION;
FxResource::FxResource()
{
	texture.SetNil();
	sampler.SetNil();
}

UINT32 CreateParameterBuffer( const FxCBuffer* uniformBuffer, ParameterBuffer *parameterBuffer )
{
	const UINT32 storedItemSize = sizeof( (*parameterBuffer)[0] );
	const UINT32 padding = uniformBuffer->size % storedItemSize;
	const UINT32 paddedLength = uniformBuffer->size + padding;
	const UINT32 numItems = paddedLength / storedItemSize;
	parameterBuffer->SetNum( numItems );
	mxTODO("set default values");
	parameterBuffer->ZeroOut();
	return paddedLength;
}

mxDEFINE_CLASS(FxCBufferBinding);
mxBEGIN_REFLECTION(FxCBufferBinding)
	//mxMEMBER_FIELD(handle),
	mxMEMBER_FIELD(slot),
	mxMEMBER_FIELD(id),
mxEND_REFLECTION;
FxCBufferBinding::FxCBufferBinding()
{
	//handle.SetNil();
	slot = ~0;
	id = ~0;
}

mxDEFINE_CLASS(FxTextureBinding);
mxBEGIN_REFLECTION(FxTextureBinding)
	//mxMEMBER_FIELD(texture),
	//mxMEMBER_FIELD(sampler),
	mxMEMBER_FIELD(slot),
	mxMEMBER_FIELD(id),
mxEND_REFLECTION;
FxTextureBinding::FxTextureBinding()
{
	//texture.SetNil();
	//sampler.SetNil();
	slot = ~0;
	id = ~0;
}

mxDEFINE_CLASS(FxDefine);
mxBEGIN_REFLECTION(FxDefine)
	mxMEMBER_FIELD(name),
	mxMEMBER_FIELD(value),
mxEND_REFLECTION;

mxDEFINE_CLASS(FxShaderPin);
mxBEGIN_REFLECTION(FxShaderPin)
	mxMEMBER_FIELD(mask),
	mxMEMBER_FIELD(enabled),
mxEND_REFLECTION;

mxDEFINE_CLASS(FxShader);
mxBEGIN_REFLECTION(FxShader)
	mxMEMBER_FIELD(programs),
	mxMEMBER_FIELD(permutations),
	mxMEMBER_FIELD(CBs),
	mxMEMBER_FIELD(SRs),
	mxMEMBER_FIELD(pins),
	mxMEMBER_FIELD(localCBs),
	mxMEMBER_FIELD(localSRs),
mxEND_REFLECTION;
FxShader::FxShader()
{
}

mxDEFINE_CLASS(FxShaderHandles);
mxBEGIN_REFLECTION(FxShaderHandles)
	mxMEMBER_FIELD(handles),
mxEND_REFLECTION;

mxDEFINE_CLASS(FxProgramHandles);
mxBEGIN_REFLECTION(FxProgramHandles)
	mxMEMBER_FIELD(handles),
mxEND_REFLECTION;

#if 0
AssetCallbacks	FxLibrary::s_callbacks;

mxDEFINE_CLASS(FxLibrary);
mxBEGIN_REFLECTION(FxLibrary)
	mxMEMBER_FIELD(m_colorTargets),
	mxMEMBER_FIELD(m_depthTargets),

	mxMEMBER_FIELD(m_depthStencilStates),
	mxMEMBER_FIELD(m_rasterizerStates),
	mxMEMBER_FIELD(m_samplerStates),
	mxMEMBER_FIELD(m_blendStates),
	mxMEMBER_FIELD(m_stateBlocks),

	mxMEMBER_FIELD(m_globalCBuffers),

	mxMEMBER_FIELD(m_shaders),

	//mxMEMBER_FIELD(m_shaders),
	//mxMEMBER_FIELD(m_programs),
	//mxMEMBER_FIELD(m_bindings),
mxEND_REFLECTION;

FxLibrary::FxLibrary()
{
}

ERet FxLibrary::Validate() const
{
	for( UINT32 iStateBlock = 0; iStateBlock < m_stateBlocks.Num(); iStateBlock++ )
	{
		const FxStateBlock& stateBlock = m_stateBlocks[ iStateBlock ];
		if( stateBlock.blendState.IsNull() ) {
			ptERROR("Invalid blend state!\n");
			return ERR_OBJECT_NOT_FOUND;
		}
		if( stateBlock.rasterizerState.IsNull() ) {
			ptERROR("Invalid rasterizer state!\n");
			return ERR_OBJECT_NOT_FOUND;
		}
		if( stateBlock.depthStencilState.IsNull() ) {
			ptERROR("Invalid depth-stencil state!\n");
			return ERR_OBJECT_NOT_FOUND;
		}
	}
	return ALL_OK;
}

ERet FxLibrary::Load( AStreamReader& stream )
{
	FxHeader	header;
	mxDO(stream.Get(header));

	if( header.fourCC == FX_Library_MAGIC ) {
		mxDO(Serialization::LoadBinary(stream, FxLibrary::MetaClass(), this));
	}

	// Initialization order:
	// 1) Render targets
	// 2) State objects
	// 3) Shaders
	// 4) Input layouts
	// 5) Everything else

	// render targets should be sorted by size
	// shader resources should be sorted by size
	// constant buffers should be sorted by size

	// Ideally, render states should be sorted by state deltas (so that changes between adjacent states are minimized)

	//CreateRenderTargets(resolution, m_colorTargets, m_depthTargets, false);

	for( UINT32 iSamplerState = 0; iSamplerState < m_samplerStates.Num(); iSamplerState++ )
	{
		FxSamplerState& samplerState = m_samplerStates[ iSamplerState ];
		samplerState.handle = llgl::CreateSamplerState( samplerState );
	}
	for( UINT32 iDepthStencilState = 0; iDepthStencilState < m_depthStencilStates.Num(); iDepthStencilState++ )
	{
		FxDepthStencilState& depthStencilState = m_depthStencilStates[ iDepthStencilState ];
		depthStencilState.handle = llgl::CreateDepthStencilState( depthStencilState );
	}
	for( UINT32 iRasterizerState = 0; iRasterizerState < m_rasterizerStates.Num(); iRasterizerState++ )
	{
		FxRasterizerState& rasterizerState = m_rasterizerStates[ iRasterizerState ];
		rasterizerState.handle = llgl::CreateRasterizerState( rasterizerState );
	}
	for( UINT32 iBlendState = 0; iBlendState < m_blendStates.Num(); iBlendState++ )
	{
		FxBlendState& blendState = m_blendStates[ iBlendState ];
		blendState.handle = llgl::CreateBlendState( blendState );
	}
	for( UINT32 iStateBlock = 0; iStateBlock < m_stateBlocks.Num(); iStateBlock++ )
	{
		FxStateBlock& stateBlock = m_stateBlocks[ iStateBlock ];
		stateBlock.blendState		= m_blendStates[ stateBlock.blendState.id ].handle;
		stateBlock.rasterizerState	= m_rasterizerStates[ stateBlock.rasterizerState.id ].handle;
		stateBlock.depthStencilState= m_depthStencilStates[ stateBlock.depthStencilState.id ].handle;
	}

	// create constant buffers and shader resources
	for( UINT32 iCB = 0; iCB < m_globalCBuffers.Num(); iCB++ )
	{
		FxCBuffer& rCB = m_globalCBuffers[ iCB ];
		rCB.handle = llgl::CreateBuffer( Buffer_Uniform, rCB.size, NULL );
	}
	//for( UINT32 iSR = 0; iSR < shaderResources.Num(); iSR++ )
	//{
	//	//XShaderResource& rSR = shaderResources[ iSR ];
	//	UNDONE;
	//}

	// Load shaders and create shader programs.

	for( UINT32 shaderType = 0; shaderType < ShaderTypeCount; shaderType++ )
	{
		for( UINT32 shaderIndex = 0; shaderIndex < header.numShaders[shaderType]; shaderIndex++ )
		{
			UINT32 codeSize;
			stream >> codeSize;

			ScopedStackAlloc	codeAlloc( gCore.frameAlloc );
			void* codeBuffer = codeAlloc.Alloc( codeSize );

			mxDO(stream.Read( codeBuffer, codeSize ));

			const UINT32 alignedOffset = ALIGN_VALUE(codeSize, 4);
			const UINT32 sizeOfPadding = alignedOffset - codeSize;
			Skip_N_bytes( stream, sizeOfPadding );

			const HShader shaderHandle = llgl::CreateShader( (EShaderType)shaderType, codeBuffer, codeSize );
			m_shaders[shaderType][shaderIndex] = shaderHandle;

			//DBGOUT("Created '%s' (handle: %u, %u bytes)\n",
			//	EShaderTypeToChars((EShaderType)shaderType), shaderHandle.id, codeSize);
		}
	}



	// Create programs.
	for( UINT32 iProgram = 0; iProgram < m_programs.Num(); iProgram++ )
	{
		FxProgram& program = m_programs[ iProgram ];

		ProgramDescription	pd;
		for( UINT32 shaderType = 0; shaderType < ShaderTypeCount; shaderType++ )
		{
			if( program.shaders[shaderType] != UINT16(~0) )
			{
				pd.shaders[shaderType] = m_shaders[shaderType][program.shaders[shaderType]];
			}
		}
		if(LLGL_Driver_Is_OpenGL)
		{
			pd.bindings = &m_bindings[ program.bindings ];
		}

		program.handle = llgl::CreateProgram( pd );
	}

	// Fixup shader techniques.
	for( UINT32 iTechnique = 0; iTechnique < m_techniques.Num(); iTechnique++ )
	{
		FxShader& technique = m_techniques[ iTechnique ];

		DBGOUT("Loading technique '%s'...\n", technique.name.ToPtr());

		// Create local shader resources.
		{
			// Create local constant buffers.
			for( UINT32 iCB = 0; iCB < technique.locals.Num(); iCB++ )
			{
				FxCBuffer& rCB = technique.locals[ iCB ];
				rCB.handle = llgl::CreateBuffer( Buffer_Uniform, rCB.size, NULL );
			}
		}

		// Fixup pointers to all shader inputs used by the technique.
		//technique.Link(*this);
		{
			for( UINT32 bufferIndex = 0; bufferIndex < technique.inputs.cbuffers.Num(); bufferIndex++ )
			{
				FxCBufferRef& bufferReference = technique.inputs.cbuffers[ bufferIndex ];
				UINT16 relativeIndex = bufferReference.handle.id;
				if( GetUpperBit( relativeIndex ) )
				{
					ClearUpperBit( relativeIndex );
					bufferReference.handle = technique.locals[ relativeIndex ].handle;
					mxASSERT(bufferReference.handle.IsValid());
				}
				else
				{
					UNDONE;
				}
			}
			for( UINT32 samplerIndex = 0; samplerIndex < technique.inputs.samplers.Num(); samplerIndex++ )
			{

			}
		}

		for( UINT32 iPass = 0; iPass < technique.passes.Num(); iPass++ )
		{
			FxPass& pass = technique.passes[ iPass ];

			for( UINT32 iProgram = 0; iProgram < pass.programs.Num(); iProgram++ )
			{
				mxASSERT( pass.programs[iProgram].id != UINT16(~0) );
				pass.programs[iProgram] = m_programs[ pass.programs[iProgram].id ].handle;
			}
		}
	}

	//DBGOUT("Loading %u shaders: %u VS, %u GS, %u PS\n",
	//	totalShaderCount,
	//	header.numShaders[ShaderVertex],
	//	header.numShaders[ShaderGeometry],
	//	header.numShaders[ShaderFragment]);

	return ALL_OK;
}

void FxLibrary::Shutdown()
{
	for( UINT32 iProgram = 0; iProgram < m_programs.Num(); iProgram++ )
	{
		FxProgram& program = m_programs[ iProgram ];
		llgl::DeleteProgram(program.handle);
		program.handle.SetNil();
	}

	for( UINT32 shaderType = 0; shaderType < mxCOUNT_OF(m_shaders); shaderType++ )
	{
		for( UINT32 iShader = 0; iShader < m_shaders[shaderType].Num(); iShader++ )
		{
			llgl::DeleteShader( m_shaders[shaderType][iShader] );
			m_shaders[shaderType][iShader].SetNil();
		}
	}

	for( UINT32 iTechnique = 0; iTechnique < m_techniques.Num(); iTechnique++ )
	{
		FxShader& technique = m_techniques[ iTechnique ];
		for( UINT32 iCB = 0; iCB < technique.locals.Num(); iCB++ )
		{
			FxCBuffer& rCB = technique.locals[ iCB ];
			llgl::DeleteBuffer( rCB.handle );
			rCB.handle.SetNil();
		}
	}

	for( UINT32 iCB = 0; iCB < m_globalCBuffers.Num(); iCB++ )
	{
		FxCBuffer& rCB = m_globalCBuffers[ iCB ];
		llgl::DeleteBuffer( rCB.handle );
		rCB.handle.SetNil();
	}
	//for( UINT32 iSR = 0; iSR < shaderResources.Num(); iSR++ )
	//{
	//	//XShaderResource& rSR = shaderResources[ iSR ];
	//	UNDONE;
	//}

	for( UINT32 iStateBlock = 0; iStateBlock < m_stateBlocks.Num(); iStateBlock++ )
	{
		FxStateBlock& stateBlock = m_stateBlocks[ iStateBlock ];
		stateBlock.rasterizerState.SetNil();
		stateBlock.depthStencilState.SetNil();
		stateBlock.blendState.SetNil();
	}
	m_stateBlocks.DestroyAndEmpty();

	for( UINT32 iBlendState = 0; iBlendState < m_blendStates.Num(); iBlendState++ )
	{
		FxBlendState& blendState = m_blendStates[ iBlendState ];
		llgl::DeleteBlendState( blendState.handle );
		blendState.handle.SetNil();
	}
	m_blendStates.DestroyAndEmpty();

	for( UINT32 iRasterizerState = 0; iRasterizerState < m_rasterizerStates.Num(); iRasterizerState++ )
	{
		FxRasterizerState& rasterizerState = m_rasterizerStates[ iRasterizerState ];
		llgl::DeleteRasterizerState( rasterizerState.handle );
		rasterizerState.handle.SetNil();
	}
	m_rasterizerStates.DestroyAndEmpty();

	for( UINT32 iDepthStencilState = 0; iDepthStencilState < m_depthStencilStates.Num(); iDepthStencilState++ )
	{
		FxDepthStencilState& depthStencilState = m_depthStencilStates[ iDepthStencilState ];
		llgl::DeleteDepthStencilState( depthStencilState.handle );
		depthStencilState.handle.SetNil();
	}
	m_depthStencilStates.DestroyAndEmpty();

	for( UINT32 iSamplerState = 0; iSamplerState < m_samplerStates.Num(); iSamplerState++ )
	{
		FxSamplerState& samplerState = m_samplerStates[ iSamplerState ];
		llgl::DeleteSamplerState( samplerState.handle );
		samplerState.handle.SetNil();
	}
	m_samplerStates.DestroyAndEmpty();

	for( UINT32 iColorTarget = 0; iColorTarget < m_colorTargets.Num(); iColorTarget++ )
	{
		FxColorTarget& colorTarget = m_colorTargets[ iColorTarget ];
		llgl::DeleteColorTarget( colorTarget.handle );
		colorTarget.handle.SetNil();
	}
	m_colorTargets.DestroyAndEmpty();

	for( UINT32 iDepthTarget = 0; iDepthTarget < m_depthTargets.Num(); iDepthTarget++ )
	{
		FxDepthTarget& depthTarget = m_depthTargets[ iDepthTarget ];
		llgl::DeleteDepthTarget( depthTarget.handle );
		depthTarget.handle.SetNil();
	}
	m_depthTargets.DestroyAndEmpty();
}

void FxLibrary::ReleaseResourcesDependentOnBackBuffer()
{
	UNDONE;
#if 0
	for( UINT32 iColorTarget = 0; iColorTarget < m_colorTargets.Num(); iColorTarget++ )
	{
		FxColorTarget& colorTarget = m_colorTargets[ iColorTarget ];
		if( DependsOnBackBufferSize( colorTarget ) ) {
			llgl::DeleteColorTarget( colorTarget.handle );
			colorTarget.handle.SetNil();
		}	
	}
	for( UINT32 iDepthTarget = 0; iDepthTarget < m_depthTargets.Num(); iDepthTarget++ )
	{
		FxDepthTarget& depthTarget = m_depthTargets[ iDepthTarget ];
		if( DependsOnBackBufferSize( depthTarget ) ) {
			llgl::DeleteDepthTarget( depthTarget.handle );
			depthTarget.handle.SetNil();
		}	
	}
#endif
}

void FxLibrary::RecreateResourcesDependentOnBackBuffer()
{
	UNDONE;
//	const Resolution resolution = llgl::GetCurrentResolution();
//	CreateRenderTargets(resolution, m_colorTargets, m_depthTargets, true);
}

ERet FxLibrary::LoadInstance( AssetLoadContext& context )
{
	AssetReader	reader( context.package, &context.stream );
	FxLibrary* library = static_cast< FxLibrary* >( context.instance );
	library->Load(reader);
	return ALL_OK;
}
#endif
mxDEFINE_CLASS(CachedShader_d);
mxBEGIN_REFLECTION(CachedShader_d)
	mxMEMBER_FIELD(code),
mxEND_REFLECTION;

mxDEFINE_CLASS(CachedProgram_d);
mxBEGIN_REFLECTION(CachedProgram_d)
	mxMEMBER_FIELD(pd),
	mxMEMBER_FIELD(gl),
mxEND_REFLECTION;

mxDEFINE_CLASS(ShaderCache_d);
mxBEGIN_REFLECTION(ShaderCache_d)
	mxMEMBER_FIELD(m_shaders),
	//mxMEMBER_FIELD(m_stats),
mxEND_REFLECTION;

ShaderCache_d::ShaderCache_d()
{
	mxZERO_OUT(m_stats);
}

UINT32 ShaderCache_d::AddShaderCode( EShaderType type, const void* code, UINT32 size )
{
	for( UINT32 i = 0; i< m_shaders[ type ].Num(); i++ )
	{
		const CachedShader_d& shader = m_shaders[ type ][ i ];
		if( shader.code.Num() != size ) {
			continue;
		}
		if( memcmp( shader.code.ToPtr(), code, size ) == 0 ) {
			m_stats.numDuplicates[ type ]++;
			return i;
		}
	}
	//DBGOUT("Adding '%s' (%u bytes)\n",EShaderTypeToChars((EShaderType)type),size);
	const UINT32 newCodeIndex = m_shaders[ type ].Num();
	CachedShader_d& newCode = m_shaders[ type ].Add();
	newCode.code.SetNum( size );
	memcpy( newCode.code.ToPtr(), code, size );
	return newCodeIndex;
}

#pragma pack (push,1)
CacheHeader_d::CacheHeader_d()
{
	fourCC = MCHAR4('S','H','I','T');//'XXXX'
	if( LLGL_Driver_Is_Direct3D ) {
		target = FX_Direct3D_MAGIC;
	} else {
		target = FX_OpenGL_MAGIC;
	}
	//timeStamp = FileTimeT::CurrentTime();
	//checksum = 0;
	//flags = 0;
	bufferSize = 0;
	mxZERO_OUT(numShaders);
	numPrograms = 0;
}
#pragma pack (pop)

ERet ShaderCache_d::SaveToStream( AStreamWriter &stream ) const
{
	// save file header
	CacheHeader_d	header;
	{
		// serialize number of shaders for each shader type
		for( UINT32 i = 0; i < ShaderTypeCount; i++ ) {
			header.numShaders[i] = m_shaders[i].Num();
		}
		header.numPrograms = m_programs.Num();
	}

	DBGOUT("Saving shaders: %u VS, %u GS, %u PS\n",
		m_shaders[ShaderVertex].Num(),
		m_shaders[ShaderGeometry].Num(),
		m_shaders[ShaderFragment].Num());

	stream.Put(header);

	// serialize compiled shader byte code
	for( UINT32 shaderType = 0; shaderType < ShaderTypeCount; shaderType++ )
	{
		for( UINT32 i = 0; i < m_shaders[shaderType].Num(); i++ )
		{
			const CachedShader_d& shader = m_shaders[shaderType][i];
			const UINT32 codeSize = shader.code.GetDataSize();
			mxDO(stream.Put( codeSize ));
			mxDO(stream.Write( shader.code.ToPtr(), codeSize ));
			header.bufferSize = Max(header.bufferSize, codeSize);
		}
	}

	ByteArrayT	memoryBlob;
	for( UINT32 i = 0; i < m_programs.Num(); i++ )
	{
		const CachedProgram_d& program = m_programs[i];

		ByteArrayWriter	blobWriter( memoryBlob );
		mxDO(Serialization::SaveImage( program, blobWriter ));
		stream << memoryBlob;
		header.bufferSize = Max(header.bufferSize, memoryBlob.Num());
		memoryBlob.Empty();
	}

	return ALL_OK;
}

void ShaderCache_d::Clear()
{
	for( UINT32 shaderType = 0; shaderType < mxCOUNT_OF(m_shaders); shaderType++ )
	{
		m_shaders[shaderType].Empty();
	}
	mxZERO_OUT(m_stats);
}

UINT32 ShaderCache_d::NumShaders() const
{
	UINT32 numShaders = 0;
	for( UINT32 shaderType = 0; shaderType < mxCOUNT_OF(m_shaders); shaderType++ )
	{
		numShaders += m_shaders[ shaderType ].Num();
	}
	return numShaders;
}

ERet ShaderCache_d::CreateShaders(
								  const CacheHeader_d& header,
								  AStreamReader& stream,
								  FxShaderHandles &shaders
								  )
{
	UINT32 totalShaderCount = 0;
	for( UINT32 shaderType = 0; shaderType < ShaderTypeCount; shaderType++ )
	{
		totalShaderCount += header.numShaders[ shaderType ];
		shaders.handles[shaderType].SetNum( header.numShaders[shaderType] );
	}

	DBGOUT("Loading %u shaders: %u VS, %u GS, %u PS\n",
		totalShaderCount,
		header.numShaders[ShaderVertex],
		header.numShaders[ShaderGeometry],
		header.numShaders[ShaderFragment]);

	for( UINT32 shaderType = 0; shaderType < ShaderTypeCount; shaderType++ )
	{
		for( UINT32 shaderIndex = 0; shaderIndex < header.numShaders[shaderType]; shaderIndex++ )
		{
			UINT32 codeSize;
			mxDO(stream.Get( codeSize ));

			ScopedStackAlloc	codeAlloc( gCore.frameAlloc );
			void* codeBuffer = codeAlloc.Alloc( codeSize );

			mxDO(stream.Read( codeBuffer, codeSize ));

			const UINT32 alignedOffset = ALIGN_VALUE(codeSize, 4);
			const UINT32 sizeOfPadding = alignedOffset - codeSize;
			Skip_N_bytes( stream, sizeOfPadding );

			//DBGOUT("Creating '%s' (%u bytes)\n",EShaderTypeToChars((EShaderType)shaderType),codeSize);

			const HShader shaderHandle = llgl::CreateShader( (EShaderType)shaderType, codeBuffer, codeSize );
			shaders.handles[shaderType][shaderIndex] = shaderHandle;
		}
	}

	return ALL_OK;
}

ERet ShaderCache_d::CreatePrograms(
	const CacheHeader_d& header,
	const FxShaderHandles& shaders,
	AStreamReader& stream,
	FxProgramHandles &programs
)
{
	programs.handles.SetNum( header.numPrograms );

	ScopedStackAlloc	programAlloc( gCore.frameAlloc );
	char *	tempBuffer = programAlloc.Alloc( header.bufferSize );
	ByteArrayT	programBlob( tempBuffer, header.bufferSize );

	for( UINT32 programIndex = 0; programIndex < header.numPrograms; programIndex++ )
	{
		stream >> programBlob;

		CachedProgram_d *	program;
		mxDO(Serialization::LoadInPlace( programBlob.ToPtr(), programBlob.Num(), program ));

		HShader* shaderIds = program->pd.shaders;
		for( UINT32 shaderType = 0; shaderType < ShaderTypeCount; shaderType++ )
		{
			const UINT16 shaderIndex = shaderIds[shaderType].id;
			if( shaderIndex != (UINT16)~0 ) {
				shaderIds[shaderType] = shaders.handles[shaderType][shaderIndex];
			}
		}

		const HProgram programHandle = llgl::CreateProgram( program->pd );

		programs.handles[programIndex] = programHandle;
	}

	return ALL_OK;
}

//String512 FxShaderCache::ComposePathToShaderCache( const char* name )
//{
//	return ComposePathToShaderCache( gCore.config->GetString("PathToShaderCache"), name );
//}
//
//String512 FxShaderCache::ComposePathToShaderCache( const char* folder, const char* name )
//{
//	String512	pathToShaderCache;
//	str::Copy(pathToShaderCache, Chars(folder));
//	str::NormalizePath(pathToShaderCache);
//
//	String512	result;
//	str::SPrintF(result, "%s%s.sc", pathToShaderCache.ToPtr(), name);
//	return result;
//}

//ERet FxCreateLibrary( AStreamReader& stream, FxLibrary **library )
//{
//	Resolution	resolution;
//	UINT32		videoFlags;
//	mxDO(llgl::GetVideoMode(resolution, videoFlags));
//
//	FxLibraryHeader	header;
//	stream >> header;
//
//	if( (LLGL_Driver_Is_Direct3D && (header.magic != FX_Direct3D_MAGIC))
//		|| (LLGL_Driver_Is_OpenGL && (header.magic != FX_OpenGL_MAGIC)) )
//	{
//		ptERROR("Couldn't load Shader library: header mismatch!\n");
//		return ERR_INCOMPATIBLE_VERSION;
//	}
//
//	MemoryImageLoader	loader( stream );
//
//	const UINT32 bufferSize = loader.GetRequiredBufferSize();
//	FxLibrary* objectBuffer = (FxLibrary*) mxAlloc(bufferSize);
//
//	mxDO(loader.LoadToBuffer( FxLibrary::MetaClass(), objectBuffer, bufferSize ));
//
//	objectBuffer->Load(resolution, stream);
//
//	*library = objectBuffer;
//
//	return ALL_OK;
//}
//
//void FxReleaseLibrary( FxLibrary **library )
//{
//	if( library && *library )
//	{
//		(*library)->Shutdown();
//		mxFree(*library);
//		*library = NULL;
//	}
//}
#if 0

ERet FxUtil_SaveLibraryToFile(const FxLibrary& library, const char* path)
{
	mxDO(Serialization::SaveBinaryToFile(&library, mxCLASS_OF(library), path));
	return ALL_OK;
}

ERet FxUtil_SaveShadersToFile(const FxShaderCache& code, const char* path)
{
	FileWriter	stream;
	mxDO(stream.Open(path, FileWrite_NoErrors));
	mxDO(code.SaveShaderCode(stream));
	return ALL_OK;
}

ERet FxUtil_SetStateBlock(const HContext _context,
						  const FxLibrary& effect,
						  const char* stateBlock)
{
	const FxStateBlock* pStateBlock = FindByName( effect.m_stateBlocks, stateBlock );
	if( pStateBlock ) {
		llgl::SetRasterizerState( _context, pStateBlock->rasterizerState );
		llgl::SetDepthStencilState( _context, pStateBlock->depthStencilState, pStateBlock->stencilRef );
		llgl::SetBlendState( _context, pStateBlock->blendState, pStateBlock->blendFactor.ToPtr(), pStateBlock->sampleMask );
		return ALL_OK;
	}
	return ERR_OBJECT_NOT_FOUND;
}
#endif


static bool DependsOnBackBufferSize( const FxRenderTargetBase& renderTarget )
{
	return renderTarget.sizeX.relative || renderTarget.sizeY.relative;
}

static UINT32 CalculateRenderTargetDimension( INT32 screenSize, const FxRenderTargetSize& textureSize )
{
	float sizeValue = textureSize.size;
	if( textureSize.relative ) {
		sizeValue *= screenSize;
	}
	return (UINT32) sizeValue;
}

static UINT32 CalculateRenderTargetArea( const Resolution& screen, const FxRenderTargetBase& renderTarget )
{
	UINT32 sizeX = CalculateRenderTargetDimension( screen.width, renderTarget.sizeX );
	UINT32 sizeY = CalculateRenderTargetDimension( screen.height, renderTarget.sizeY );
	return sizeX * sizeY;
}

struct SortItem
{
	void *	o;	// FxColorTarget or FxDepthTarget
	UINT32	size;
	bool	isColor;
};

void CreateRenderTargets( const Resolution& screen,
						 Clump &clump,
						 bool createOnlyThoseDependentOnBackBuffer )
{
	// create largest render targets first
	SortItem			sortedItemsStorage[96];
	TArray< SortItem >	sortedItems( sortedItemsStorage, mxCOUNT_OF(sortedItemsStorage) );

	{
		TObjectIterator< FxColorTarget >	colorTargetIt( clump );
		while( colorTargetIt.IsValid() )
		{
			FxColorTarget& colorTarget = colorTargetIt.Value();
			if( createOnlyThoseDependentOnBackBuffer && !DependsOnBackBufferSize( colorTarget ) ) {
				continue;
			}
			SortItem& sortItem = sortedItems.Add();
			sortItem.o = &colorTarget;
			sortItem.size = CalculateRenderTargetArea( screen, colorTarget ) * PixelFormat::BitsPerPixel(colorTarget.format);
			sortItem.isColor = true;
			colorTargetIt.MoveToNext();
		}
	}
	{
		TObjectIterator< FxDepthTarget >	depthTargetIt( clump );
		while( depthTargetIt.IsValid() )
		{
			FxDepthTarget& depthTarget = depthTargetIt.Value();
			if( createOnlyThoseDependentOnBackBuffer && !DependsOnBackBufferSize( depthTarget ) ) {
				continue;
			}
			SortItem& sortItem = sortedItems.Add();
			sortItem.o = &depthTarget;
			sortItem.size = CalculateRenderTargetArea( screen, depthTarget ) * DepthStencilFormat::BitsPerPixel(depthTarget.format);
			sortItem.isColor = false;
			depthTargetIt.MoveToNext();
		}
	}

	struct CompareRenderTargetsByArea {
		inline bool operator () ( const SortItem& a, const SortItem& b ) const { return a.size > b.size; }
	};
	std::stable_sort( sortedItems.ToPtr(), sortedItems.ToPtr() + sortedItems.Num(), CompareRenderTargetsByArea() );

	ColorTargetDescription	colorTargetDescription;
	DepthTargetDescription	depthTargetDescription;
	for( UINT32 iRenderTarget = 0; iRenderTarget < sortedItems.Num(); iRenderTarget++ )
	{
		SortItem& item = sortedItems[ iRenderTarget ];
		if( item.isColor )
		{
			FxColorTarget& colorTarget = *static_cast< FxColorTarget* >( item.o );

			colorTargetDescription.name.SetReference( colorTarget.name );
			colorTargetDescription.format = colorTarget.format;
			colorTargetDescription.width = CalculateRenderTargetDimension( screen.width, colorTarget.sizeX );
			colorTargetDescription.height = CalculateRenderTargetDimension( screen.height, colorTarget.sizeY );

			colorTarget.handle = llgl::CreateColorTarget( colorTargetDescription );
		}
		else
		{
			FxDepthTarget& depthTarget = *static_cast< FxDepthTarget* >( item.o );

			depthTargetDescription.name.SetReference( depthTarget.name );
			depthTargetDescription.format = depthTarget.format;
			depthTargetDescription.width = CalculateRenderTargetDimension( screen.width, depthTarget.sizeX );
			depthTargetDescription.height = CalculateRenderTargetDimension( screen.height, depthTarget.sizeY );

			depthTarget.handle = llgl::CreateDepthTarget( depthTargetDescription );
		}
	}
}

ERet FxUtil_LoadShaderLibrary( AStreamReader& stream, Clump &clump, const Resolution& screen )
{
	FxLibraryHeader_d	libraryHeader;
	mxDO(stream.Get(libraryHeader));

	// Load shader library structures.

#if mxUSE_BINARY_EFFECT_FILE_FORMAT
	UNDONE;
	//mxTRY(Serialization::LoadClumpImage(stream, clump));
#else

	ByteBuffer32	buffer;
	buffer.SetNum(libraryHeader.runtimeSize);
	stream.Read(buffer.ToPtr(), buffer.Num());

	SON::Parser		parser;
	parser.buffer = (char*)buffer.ToPtr();
	parser.length = libraryHeader.runtimeSize;
	parser.line = 1;
	parser.file = "";

	SON::Allocator	allocator;
	SON::Node* root = SON::ParseBuffer(parser, allocator);
	chkRET_X_IF_NIL(root, ERR_FAILED_TO_PARSE_DATA);

	mxTRY(SON::LoadClump( root, clump ));
#endif

	// Initialization order:
	// 1) Render targets
	// 2) State objects
	// 3) Shaders
	// 4) Input layouts
	// 5) Everything else

	// render targets should be sorted by size
	// shader resources should be sorted by size
	// constant buffers should be sorted by size

	CreateRenderTargets(screen, clump, false);

	// Ideally, render states should be sorted by state deltas (so that changes between adjacent states are minimized).

	{
		TObjectIterator< FxSamplerState >	samplerStateIt( clump );
		while( samplerStateIt.IsValid() )
		{
			FxSamplerState& samplerState = samplerStateIt.Value();
			samplerState.handle = llgl::CreateSamplerState( samplerState );
			samplerStateIt.MoveToNext();
		}
	}
	{
		TObjectIterator< FxDepthStencilState >	depthStencilStateIt( clump );
		while( depthStencilStateIt.IsValid() )
		{
			FxDepthStencilState& depthStencilState = depthStencilStateIt.Value();
			depthStencilState.handle = llgl::CreateDepthStencilState( depthStencilState );
			depthStencilStateIt.MoveToNext();
		}
	}
	{
		TObjectIterator< FxRasterizerState >	rasterizerStateIt( clump );
		while( rasterizerStateIt.IsValid() )
		{
			FxRasterizerState& rasterizerState = rasterizerStateIt.Value();
			rasterizerState.handle = llgl::CreateRasterizerState( rasterizerState );
			rasterizerStateIt.MoveToNext();
		}
	}
	{
		TObjectIterator< FxBlendState >	blendStateIt( clump );
		while( blendStateIt.IsValid() )
		{
			FxBlendState& blendState = blendStateIt.Value();
			blendState.handle = llgl::CreateBlendState( blendState );
			blendStateIt.MoveToNext();
		}
	}

	FxRenderResources* renderStates = FindSingleInstance<FxRenderResources>(clump);
	if(renderStates)
	{
		for( UINT32 iStateBlock = 0; iStateBlock < renderStates->state_blocks.Num(); iStateBlock++ )
		{
			FxStateBlock* stateBlock = renderStates->state_blocks[ iStateBlock ];
			stateBlock->blendState		= renderStates->blend_states[ stateBlock->blendState.id ]->handle;
			stateBlock->rasterizerState	= renderStates->rasterizer_states[ stateBlock->rasterizerState.id ]->handle;
			stateBlock->depthStencilState= renderStates->depth_stencil_states[ stateBlock->depthStencilState.id ]->handle;
		}
	}

	// Create constant buffers and shader resources.
	{
		TObjectIterator< FxCBuffer >	cbufferIt( clump );
		while( cbufferIt.IsValid() )
		{
			FxCBuffer& ubo = cbufferIt.Value();
			ubo.handle = llgl::CreateBuffer( Buffer_Uniform, ubo.size, NULL );
			cbufferIt.MoveToNext();
		}
	}

	CacheHeader_d	cacheHeader;
	mxDO(stream.Get(cacheHeader));

	// Create shaders.
	FxShaderHandles	shaders;
	ScopedStackAlloc	tempAlloc( gCore.frameAlloc );
	for( UINT32 shaderType = 0; shaderType < ShaderTypeCount; shaderType++ )
	{
		const UINT32 shaderCount = cacheHeader.numShaders[shaderType];
		if( shaderCount ) {
			HShader* shaderHandlesStorage = tempAlloc.AllocMany< HShader >( shaderCount );
			shaders.handles[shaderType].SetExternalStorage( shaderHandlesStorage, shaderCount );
			shaders.handles[shaderType].SetNum( shaderCount );
		}
	}
	mxDO(ShaderCache_d::CreateShaders( cacheHeader, stream, shaders ));

	// Create programs.
	TArray< HProgram >	programHandles;
	{
		const UINT32 programsCount = cacheHeader.numPrograms;
		HProgram* programHandlesStorage = tempAlloc.AllocMany< HProgram >( programsCount );
		programHandles.SetExternalStorage( programHandlesStorage, programsCount );
		programHandles.SetNum( programsCount );

		ScopedStackAlloc	programAlloc( gCore.frameAlloc );
		char *	tempBuffer = programAlloc.Alloc( cacheHeader.bufferSize );
		ByteArrayT	programBlob( tempBuffer, cacheHeader.bufferSize );

		for( UINT32 programIndex = 0; programIndex < programsCount; programIndex++ )
		{
			stream >> programBlob;

			CachedProgram_d *	program;
			mxDO(Serialization::LoadInPlace( programBlob.ToPtr(), programBlob.Num(), program ));

			HShader* shaderIds = program->pd.shaders;
			for( UINT32 shaderType = 0; shaderType < ShaderTypeCount; shaderType++ )
			{
				const UINT16 shaderIndex = shaderIds[shaderType].id;
				if( shaderIndex != (UINT16)~0 ) {
					shaderIds[shaderType] = shaders.handles[shaderType][shaderIndex];
				}
			}

			const HProgram programHandle = llgl::CreateProgram( program->pd );
			programHandles[programIndex] = programHandle;
		}
	}

	{
		TObjectIterator< FxShader >	shaderIt( clump );
		while( shaderIt.IsValid() )
		{
			FxShader& shader = shaderIt.Value();

			for( UINT32 i = 0; i < shader.programs.Num(); i++ )
			{
				int programIndex = shader.programs[i].id;
				shader.programs[i] = programHandles[programIndex];
			}

			shaderIt.MoveToNext();
		}
	}


	return ALL_OK;
}

FxCBufferBinding* FxSlow_FindCBufferBinding(FxShader* shader, const char* name)
{
	for( UINT32 iCB = 0; iCB < shader->CBs.Num(); iCB++ )
	{
		FxCBufferBinding& binding = shader->CBs[ iCB ];
		const FxCBuffer& cbuffer = shader->localCBs[ binding.id ];
		if( Str::EqualS( cbuffer.name, name ) ) {
			return &binding;
		}		
	}
	return NULL;
}
FxTextureBinding* FxSlow_FindTextureBinding(FxShader* shader, const char* name)
{
	for( UINT32 iTS = 0; iTS < shader->SRs.Num(); iTS++ )
	{
		FxTextureBinding& binding = shader->SRs[ iTS ];
		const FxResource& resource = shader->localSRs[ binding.id ];
		if( Str::EqualS( resource.name, name ) ) {
			return &binding;
		}		
	}
	return NULL;
}

ERet FxSlow_UpdateUBO(HContext _context, FxShader* shader, const char* UBO, const void* data, int size)
{
	const FxCBufferBinding* binding = FxSlow_FindCBufferBinding(shader, UBO);
	if( binding ) {
		const FxCBuffer& cbuffer = shader->localCBs[ binding->id ];
		llgl::UpdateBuffer( _context, cbuffer.handle, size, data );
		return ALL_OK;
	}
	return ERR_OBJECT_NOT_FOUND;
}

HUniform FxGetUniform( FxShader* shader, const char* name )
{
	mxASSERT_PTR(shader);
	HUniform uniform;
	uniform.SetNil();
	for( UINT iCB = 0; iCB < shader->localCBs.Num(); iCB++ )
	{
		FxCBuffer& cbuffer = shader->localCBs[ iCB ];
		if( !cbuffer.backingStore.data.Num() ) {
			continue;
		}
		int uniformIndex = FindIndexByName( cbuffer.uniforms, name );
		if( uniformIndex != -1 )
		{
			uniform.id = (iCB << 12U) | uniformIndex;	// 4 upper bits = UBO slot
			break;
		}
	}
	return uniform;
}
ERet FxSetUniform( FxShader* shader, HUniform handle, const void* data )
{
	mxASSERT_PTR(shader);
	UINT bufferIndex = (handle.id & 0xF000) >> 12U;
	UINT uniformIndex = (handle.id & 0x0FFF);
	FxCBuffer& cbuffer = shader->localCBs[ bufferIndex ];
	FxUniform& uniform = cbuffer.uniforms[ uniformIndex ];

	mxASSERT(cbuffer.backingStore.data.Num());
	void* pDestination = cbuffer.backingStore.data.ToPtr();
	pDestination = mxAddByteOffset( pDestination, uniform.offset );
	memcpy( pDestination, data, uniform.size );
	cbuffer.backingStore.dirty = true;

	return ALL_OK;
}

ERet FxSlow_SetResource( FxShader* shader, const char* name, HResource source, HSamplerState sampler )
{
	mxASSERT_PTR(shader);
	FxTextureBinding* binding = FxSlow_FindTextureBinding(shader, name);
	if( !binding ) {
		return ERR_OBJECT_NOT_FOUND;
	}
	FxResource& resource = shader->localSRs[ binding->id ];
	resource.texture = source;
	resource.sampler = sampler;
	return ALL_OK;
}
ERet FxSlow_SetUniform( FxShader* shader, const char* name, const void* data )
{
	mxASSERT_PTR(shader);
	mxASSERT_PTR(name);
	mxASSERT_PTR(data);
	for( int iCB = 0; iCB < shader->localCBs.Num(); iCB++ )
	{
		FxCBuffer& cbuffer = shader->localCBs[ iCB ];
		const FxUniform* pUniform = FindByName( cbuffer.uniforms, name );
		if( !pUniform ) {
			return ERR_OBJECT_NOT_FOUND;
		}
		FxBackingStore& backingStore = cbuffer.backingStore;
		if( !backingStore.data.Num() ) {
			return ERR_INVALID_FUNCTION_CALL;
		}
		void* pDestination = backingStore.data.ToPtr();
		pDestination = mxAddByteOffset( pDestination, pUniform->offset );
		memcpy( pDestination, data, pUniform->size );
		backingStore.dirty = true;
	}
	return ALL_OK;
}
ERet FxSlow_Commit( HContext _context, FxShader* shader )
{
	mxASSERT_PTR(shader);
	for( int iCB = 0; iCB < shader->localCBs.Num(); iCB++ )
	{
		FxCBuffer& cbuffer = shader->localCBs[ iCB ];
		FxBackingStore& backingStore = cbuffer.backingStore;
		if( backingStore.data.Num() && backingStore.dirty ) {
			const void* data = backingStore.data.ToPtr();
			llgl::UpdateBuffer( _context, cbuffer.handle, cbuffer.size, data );
			backingStore.dirty = false;
		}
	}
	return ALL_OK;
}

ERet FxSlow_SetDepthStencilState( HContext _context, const Clump& clump, const char* name, UINT8 stencilRef )
{
	FxDepthStencilState* pDS = FindByName<FxDepthStencilState>( clump, name );
	if( pDS ) {
		llgl::SetDepthStencilState(_context, pDS->handle, stencilRef);
		return ALL_OK;
	}
	return ERR_OBJECT_NOT_FOUND;
}
ERet FxSlow_SetRasterizerState( HContext _context, const Clump& clump, const char* name )
{
	FxRasterizerState* pRS = FindByName<FxRasterizerState>( clump, name );
	if( pRS ) {
		llgl::SetRasterizerState(_context, pRS->handle);
		return ALL_OK;
	}
	return ERR_OBJECT_NOT_FOUND;
}
ERet FxSlow_SetBlendState( HContext _context, const Clump& clump, const char* name, const float* blendFactor /*= NULL*/, UINT32 sampleMask /*= ~0*/ )
{
	FxBlendState* pBS = FindByName<FxBlendState>( clump, name );
	if( pBS ) {
		llgl::SetBlendState(_context, pBS->handle);
		return ALL_OK;
	}
	return ERR_OBJECT_NOT_FOUND;
}
ERet FxSlow_SetRenderState( HContext _context, const Clump& clump, const char* name )
{
	FxStateBlock* pState = FindByName<FxStateBlock>( clump, name );
	if( pState ) {
		FxSetRenderState(_context, *pState);
		return ALL_OK;
	}
	ptERROR("Couldn't find state block: '%s'",name);
	return ERR_OBJECT_NOT_FOUND;
}
void FxSetRenderState( HContext _context, const FxStateBlock& _state )
{
	llgl::SetDepthStencilState(_context, _state.depthStencilState, _state.stencilRef);
	llgl::SetRasterizerState(_context, _state.rasterizerState);
	llgl::SetBlendState(_context, _state.blendState, _state.blendFactor.ToPtr(), _state.sampleMask);
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
