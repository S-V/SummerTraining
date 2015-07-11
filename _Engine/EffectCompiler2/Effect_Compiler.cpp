#include <Base/Base.h>
#pragma hdrstop
//#include <Base/Text/StringTools.h>
#include <Core/Text/Lexer.h>
#include <Core/Serialization.h>
#include <Graphics/Effects.h>
#include <TxTSupport/TxTSerializers.h>

#include "Effect_Compiler.h"
#include "Target_Direct3D_11.h"
#include "Target_OpenGL.h"

mxBEGIN_REFLECT_ENUM( ShaderTargetT )
	mxREFLECT_ENUM_ITEM( PC_Direct3D_11, EShaderTarget::PC_Direct3D_11 ),
	mxREFLECT_ENUM_ITEM( PC_OpenGL_4plus, EShaderTarget::PC_OpenGL_4plus ),
mxEND_REFLECT_ENUM

mxDEFINE_CLASS(FxOptions);
mxBEGIN_REFLECTION(FxOptions)
//	mxMEMBER_FIELD(target),

	mxMEMBER_FIELD(defines),

//	mxMEMBER_FIELD(pathToEffectFiles),
	mxMEMBER_FIELD(search_paths),
//	mxMEMBER_FIELD(pathToGenCppFiles),

//	mxMEMBER_FIELD(outputPath),
//	mxMEMBER_FIELD(dumpPreprocessedShaders),
	mxMEMBER_FIELD(debugDumpPath),

	mxMEMBER_FIELD(dumpPipeline),
	mxMEMBER_FIELD(dumpShaderCode),
	mxMEMBER_FIELD(optimizeShaders),
	//mxMEMBER_FIELD(dumpDisassembly),
	mxMEMBER_FIELD(stripReflection),

	mxMEMBER_FIELD(stripSymbolNames),
mxEND_REFLECTION;

FxOptions::FxOptions()
{
	//if( LLGL_Driver_Is_Direct3D ) {
	//	target = PC_Direct3D_11;
	//} else {
	//	target = PC_OpenGL_4plus;
	//}

	//dumpPreprocessedShaders = false;

	dumpPipeline = false;
	dumpShaderCode = false;
	optimizeShaders = true;
	//dumpDisassembly = false;
	stripReflection = false;
	stripSymbolNames = false;
	generateCppHeaders = false;	
}
#if 0

ERet CreateBackingStore(
							   const FxLibraryDescription& description,
							   Clump& clump
							   )
{
	TObjectIterator< FxShader >	shaderIt( clump );
	while( shaderIt.IsValid() )
	{
		FxShader& shader = shaderIt.Value();

		const FxShaderDescription* shaderDescription =
			FindByName( description.shader_programs, shader.name.ToPtr() );
		if( shaderDescription )
		{
			for( int i = 0; i < shaderDescription->mirrored_uniform_buffers.Num(); i++ )
			{
				const String& uniformBuffer = shaderDescription->mirrored_uniform_buffers[i];
				FxCBufferBinding* bufferBinding = FindByName( shader.CBs, uniformBuffer.ToPtr() );
				if( bufferBinding )
				{
					FxCBuffer* pCB = bufferBinding->resource;
					if( pCB )
					{
						FxBackingStore* backingStore = clump.New< FxBackingStore >();
						if( !backingStore ) {
							return ERR_OUT_OF_MEMORY;
						}
						CreateParameterBuffer( pCB, &backingStore->data );
						backingStore->dirty = false;
						pCB->backingStore = backingStore;
					}
				}
				else//if( !Str::Equal( uniformBuffer, s_GLOBAL_UBO_ID ) )
				{
					ptERROR("Cannot find mirrored buffer '%s'", uniformBuffer.SafeGetPtr());
				}
			}
		}

		shaderIt.MoveToNext();
	}
	return ALL_OK;
}
#endif
ERet CompileShaderLibrary(
						  const char* filepath,
						  const FxOptions& options,
						  Clump &clump,
						  ShaderCache_d &cache
						  )
{
#if 0
	// By default, create a mirror copy in RAM for the global constant buffer.
	{
		String globalCBufferID;
		globalCBufferID.SetReference( s_GLOBAL_UBO_ID );
		for( UINT32 iShader = 0; iShader < description.shader_programs.Num(); iShader++ )
		{
			FxShaderDescription& shader = description.shader_programs[ iShader ];
			shader.mirrored_uniform_buffers.AddUnique( globalCBufferID );
		}
	}
#endif

	FxLibraryDescription	description;
	mxDO(SON::LoadFromFile(filepath, description));

	MultiFileInclude	include;
	include.AddSearchPath(filepath);
	for( UINT i = 0; i < options.search_paths.Num(); i++ ) {
		include.AddSearchPath(options.search_paths[i].ToPtr());
	}

	mxDO(CompileShaderLibrary(description, options, include, clump, cache));

	return ALL_OK;
}

ERet CompileShaderLibrary(
						  const FxLibraryDescription& library,
						  const FxOptions& options,
						  AFileInclude& include,
						  Clump &clump, ShaderCache_d &cache
						  )
{
	//if( options.target == PC_Direct3D_11 )
	if( LLGL_Driver_Is_Direct3D )
	{
		mxTRY(CompileLibraryD3D11(
			library,
			options,
			&include,
			clump,
			cache
			));
	}
	//else if( options.target == PC_OpenGL_4plus )
	else if( LLGL_Driver_Is_OpenGL )
	{
		UNDONE;
		//mxTRY(CompileLibraryOpenGL(
		//	library,
		//	options,
		//	&include,
		//	clump,
		//	cache
		//	));
	}
	else {
		return ERR_INVALID_PARAMETER;
	}

	//mxTRY(CreateBackingStore( library, clump ));

	return ALL_OK;
}

ERet FxCompileEffectFromFile(const char* filename,
							 ByteArrayT &effectBlob,
							 const FxOptions& options)
{
	Clump			clump;
	ShaderCache_d	cache;

	mxDO(CompileShaderLibrary( filename, options, clump, cache ));

	effectBlob.Empty();
	ByteArrayWriter	writer(effectBlob);

	FxLibraryHeader_d	header;
	mxZERO_OUT(header);
	header.magicNumber = '41XF';

	mxDO(writer.Put(header));

	// We compile the source asset file into two parts:
	// 1) the memory-resident part (a clump containing effect library structures);
	// 2) the temporary data that is used only for initializing run-time structures and exists only during loading (compiled shader code);

	// save the memory-resident part to enable initial loading and hot-reloading
#if mxUSE_BINARY_EFFECT_FILE_FORMAT
	//mxTRY(Serialization::SaveBinary(&O, mxCLASS_OF(O), writer));
	mxTRY(Serialization::SaveClumpImage( clump, writer ));
#else
	mxTRY(SON::SaveClump( clump, writer ));
#endif

	{
		UINT32 currentOffset = effectBlob.GetDataSize();
		UINT32 alignedOffset = AlignUp( currentOffset, 4 );
		UINT32 paddingAmount = alignedOffset - currentOffset;
		if( paddingAmount ) {
			UINT32 padding = 0;
			mxDO(writer.Write( &padding, paddingAmount ));
		}
	}

	{
		header.runtimeSize = effectBlob.GetDataSize() - sizeof(FxLibraryHeader_d);
		((FxLibraryHeader_d*)effectBlob.ToPtr())->runtimeSize = header.runtimeSize;
	}

	// save compiled shader bytecode
	mxTRY(cache.SaveToStream(writer));

	//DBGOUT("Saving shaders: %u VS, %u GS, %u PS\n",
	//	m_shaders[ShaderVertex].Num(),
	//	m_shaders[ShaderGeometry].Num(),
	//	m_shaders[ShaderFragment].Num());

	return ALL_OK;
}

ERet FxCompileAndSaveEffect(const char* sourceFile,
							const char* destination,
							const FxOptions& options)
{
	ByteArrayT	effectBlob;
	mxTRY(FxCompileEffectFromFile(sourceFile, effectBlob, options));
	mxTRY(Util_SaveDataToFile(effectBlob.ToPtr(), effectBlob.Num(), destination));
	return ALL_OK;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
