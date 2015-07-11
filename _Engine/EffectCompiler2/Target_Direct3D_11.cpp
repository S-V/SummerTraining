/*
=============================================================================
	File:	PC_D3D11.cpp
	Desc:	
=============================================================================
*/
#include <Base/Base.h>
#pragma hdrstop

#include <D3DX11.h>
#include <D3Dcompiler.h>

#include <Core/Text/Token.h>
#include <Core/Serialization.h>
#include <Graphics/Device.h>
#include <Graphics/Effects.h>
#include <ShaderCompiler/ShaderCompiler.h>

#include "Target_Direct3D_11.h"

#if USE_D3D_SHADER_COMPILER

using namespace Shaders;

struct ProgramD3D
{
	String64		name;	// for debugging only
	UINT16			shaders[ShaderTypeCount];
	ShaderMetadata	metadata;
};
mxSTATIC_ASSERT(sizeof(HShader) == sizeof(UINT16));

static ERet CompileShaderD3D(
	const char* source,
	size_t sourceLength,
	const String& file,
	const String& entry,
	AFileInclude* include,
	const EShaderType type,
	const FxOptions& options,
	ShaderCache_d& shaderCache,
	UINT16 &shaderByteCodeIndex,
	ShaderMetadata &shaderMetadata
	)
{
	if( entry.IsEmpty() ) {
		return ALL_OK;
	}

	TArray< Shaders::Macro > defines;
	defines.SetNum( options.defines.Num() );
	for( int i = 0; i < options.defines.Num(); i++ )
	{
		defines[i].name = options.defines[i].name.c_str();
		defines[i].value = options.defines[i].value.c_str();
	}

	Shaders::Options	compilerOptions;
	compilerOptions.defines = defines.ToPtr();
	compilerOptions.numDefines = defines.Num();
	compilerOptions.include = include;
	if( options.optimizeShaders ) {
		compilerOptions.flags |= Shaders::Compile_Optimize;
	}

	Shaders::ByteCode	compiledCode;

	mxTRY(Shaders::CompileShaderD3D(
		compiledCode,
		source,
		sourceLength,
		entry.ToPtr(),
		type,
		&compilerOptions,
		file.ToPtr()
	));

	void* compiledCodeData = Shaders::GetBufferData( compiledCode );
	size_t compiledCodeSize = Shaders::GetBufferSize( compiledCode );

	if( !options.debugDumpPath.IsEmpty() )
	{
		String64	pureFileName(file);
		Str::StripPath(pureFileName);
		Str::StripFileExtension(pureFileName);

		String512	dumpFileName;
		Str::SPrintF(dumpFileName, "%s%s_%s.html", options.debugDumpPath.ToPtr(), pureFileName.ToPtr(), entry.ToPtr());
		UINT32 disassembleFlags = Shaders::DISASM_WriteAsHTML;
		const char* disasmComments = dumpFileName.ToPtr();
		Shaders::ByteCode disassembly = Shaders::DisassembleShaderD3D(
			compiledCodeData,
			compiledCodeSize,
			disassembleFlags,
			disasmComments
			);
		Util_SaveDataToFile(Shaders::GetBufferData(disassembly), Shaders::GetBufferSize(disassembly), dumpFileName.ToPtr());
		Shaders::ReleaseBuffer( disassembly );
	}

	Shaders::ShaderMetadata	reflectionMetadata;
	mxTRY(Shaders::ReflectShaderD3D( compiledCodeData, compiledCodeSize, reflectionMetadata ));
	//DBGOUT("%s '%s' uses %u CBs, %u SSs, %u SRs\n",
	//	shaderTypeString, program.name.ToPtr(), shaderMetadata.numUsedCBs, shaderMetadata.numUsedSRs, shaderMetadata.numUsedSSs);

	mxTRY(MergeMetadataD3D(shaderMetadata, reflectionMetadata));


	Shaders::ByteCode strippedCode = nil;
	if( options.stripSymbolNames )
	{
		strippedCode = Shaders::StripShaderD3D( compiledCodeData, compiledCodeSize );
		if( strippedCode != nil )
		{
			compiledCodeData = Shaders::GetBufferData( strippedCode );
			compiledCodeSize = Shaders::GetBufferSize( strippedCode );
		}
	}

	shaderByteCodeIndex = shaderCache.AddShaderCode( type, compiledCodeData, compiledCodeSize );

	DBGOUT("Compiled '%s' in '%s' (%u bytes at [%u])\n", entry.ToPtr(), file.ToPtr(), compiledCodeSize, shaderByteCodeIndex);

	Shaders::ReleaseBuffer( compiledCode );
	Shaders::ReleaseBuffer( strippedCode );

	return ALL_OK;
}

static ERet CompileProgramD3D(
	ProgramD3D &program,
	AFileInclude* include,
	const FxOptions& options,
	ShaderCache_d& shaderCache,
	const FxShaderEntryD3D& shader
	)
{
	memset(program.shaders, -1, sizeof(program.shaders));

	char *	fileData;
	UINT32	fileSize;
	if( !include->OpenFile(shader.file.c_str(), &fileData, &fileSize) )
	{
		ptERROR("Failed to open '%s'.\n", shader.file.ToPtr());
		return ERR_FAILED_TO_OPEN_FILE;
	}

	String sourceFile;
	sourceFile.SetReference(Chars(include->CurrentFilePath()));

	mxTRY(CompileShaderD3D(
		fileData,
		fileSize,
		sourceFile,
		shader.vertex,
		include,
		ShaderVertex,
		options,
		shaderCache,
		program.shaders[ShaderVertex],
		program.metadata
	));

	mxTRY(CompileShaderD3D(
		fileData,
		fileSize,
		sourceFile,
		shader.geometry,
		include,
		ShaderGeometry,
		options,
		shaderCache,
		program.shaders[ShaderGeometry],
		program.metadata
	));

	mxTRY(CompileShaderD3D(
		fileData,
		fileSize,
		sourceFile,
		shader.pixel,
		include,
		ShaderFragment,
		options,
		shaderCache,
		program.shaders[ShaderFragment],
		program.metadata
	));

	return ALL_OK;
}

ERet CompileLibraryD3D11(
	const FxLibraryDescription& library,
	const FxOptions& options,
	AFileInclude* include,
	Clump &clump,
	ShaderCache_d &ñache
	)
{
	mxDO(CreateResourceObjects(library, clump));
	
	const UINT32 numShaders = library.shader_programs.Num();
	if( numShaders > 0 ) {
		clump.CreateObjectList< FxShader >( numShaders );
	}

	TArray< ProgramD3D >	programs;

	for( UINT32 iShader = 0; iShader < numShaders; iShader++ )
	{
		const FxShaderDescription& shaderDesc = library.shader_programs[ iShader ];
		const FxShaderEntryD3D& entry = shaderDesc.D3D;
		FxShader* shader = clump.New< FxShader >();

		shader->name = shaderDesc.name;
		shader->UpdateNameHash();

		ShaderMetadata	metadata;	// merged from all shader combinations/permutations/variations

		const UINT32 numPermutations = 1;
		shader->programs.SetNum(numPermutations);
		shader->permutations.SetNum(numPermutations);
		for( UINT32 iProgram = 0; iProgram < numPermutations; iProgram++ )
		{
			const UINT32 programIndex = programs.Num();
			shader->programs[iProgram].id = programIndex;
			ProgramD3D& program = programs.Add();
			mxTRY(CompileProgramD3D(program, include, options, ñache, entry));
			mxTRY(MergeMetadataD3D(metadata, program.metadata));
			shader->permutations[iProgram] = iProgram;
		}

		const UINT32 numCBuffers = metadata.cbuffers.Num();
		shader->CBs.SetNum( numCBuffers );

		const UINT32 numSamplers = metadata.samplers.Num();
		shader->SRs.SetNum( numSamplers );

		for( UINT32 iCB = 0; iCB < numCBuffers; iCB++ )
		{
			const ShaderCBuffer& buffer = metadata.cbuffers[ iCB ];
			FxCBuffer* pResource = clump.New< FxCBuffer >( 64 );

			pResource->name = buffer.name;
			pResource->UpdateNameHash();

			pResource->size = buffer.size;
			pResource->handle.SetNil();

			const UINT32 numFields = buffer.fields.Num();
			pResource->uniforms.SetNum(numFields);
			for( UINT32 iUniform = 0; iUniform < numFields; iUniform++ )
			{
				const Field& field = buffer.fields[ iUniform ];
				FxUniform& uniform = pResource->uniforms[ iUniform ];
				uniform.name = field.name;
				uniform.UpdateNameHash();
				uniform.offset = field.offset;
				uniform.size = field.size;
			}

			UNDONE;
			//FxCBufferBinding& cbufferBinding = shader->CBs[ iCB ];
			//cbufferBinding.resource = pResource;
			//cbufferBinding.slot = buffer.slot;
		}

		for( UINT32 iTS = 0; iTS < numSamplers; iTS++ )
		{
			const ShaderSampler& sampler = metadata.samplers[ iTS ];
			FxResource* pResource = clump.New< FxResource >( 64 );

			pResource->name = sampler.name;
			pResource->UpdateNameHash();

			pResource->texture.SetNil();
			pResource->sampler.SetNil();

			UNDONE;
			//FxTextureBinding& samplerBinding = shader->SRs[ iTS ];
			//samplerBinding.resource = pResource;
			//samplerBinding.slot = sampler.slot;
		}
	}

	ñache.m_programs.SetNum(programs.Num());
	for( UINT32 iProgram = 0; iProgram < programs.Num(); iProgram++ )
	{
		const ProgramD3D& program = programs[iProgram];
		CachedProgram_d& cachedProgram = ñache.m_programs[iProgram];
		cachedProgram.pd.name = program.name;
		cachedProgram.pd.UpdateNameHash();
		memcpy(cachedProgram.pd.shaders, program.shaders, sizeof(HShader)*ShaderTypeCount);
		ExtractBindings( &cachedProgram.gl, program.metadata );
		cachedProgram.pd.bindings = &cachedProgram.gl;
	}

	return ALL_OK;
}

#if 0
using namespace ShaderCompiler;

struct PassDataD3D
{
	ShaderMetadata			metadata;
	TArray< ProgramD3D >	programs;
};

static ERet CompileShaderD3D(
	const char* source,
	size_t sourceLength,
	const String& file,
	const String& entry,
	AFileInclude* include,
	const EShaderType type,
	const FxOptions& options,
	ShaderCache_d& shaderCache,
	UINT16 &shaderByteCodeIndex,
	ShaderMetadata &shaderMetadata
	)
{
	if( entry.IsEmpty() ) {
		return ALL_OK;
	}

#if USE_D3D_SHADER_COMPILER

	ShaderCompiler::Options		compilerOptions;
	compilerOptions.include = include;
	if( options.optimizeShaders ) {
		compilerOptions.flags |= ShaderCompiler::Compile_Optimize;
	}

	ShaderCompiler::ByteCode	compiledCode;

	mxTRY(ShaderCompiler::CompileShaderD3D(
		compiledCode,
		source,
		sourceLength,
		entry.ToPtr(),
		type,
		&compilerOptions,
		file.ToPtr()
	));

	void* compiledCodeData = ShaderCompiler::GetBufferData( compiledCode );
	size_t compiledCodeSize = ShaderCompiler::GetBufferSize( compiledCode );

	//if( options.dumpDisassembly )
	//{
	//	String512	dumpFileName;
	//	str::SPrintF( dumpFileName, "%s%s_%s.asm",
	//		options.outputPath.ToPtr(), program.name.ToPtr(), ShaderToExtension(type) );
	//	UINT32 disassembleFlags = 0;
	//	ShaderCompiler::ByteCode disassembly = ShaderCompiler::DisassembleShader(
	//		compiledCodeData,
	//		compiledCodeSize,
	//		disassembleFlags,
	//		disasmComments
	//		);
	//	Util_SaveDataToFile(dumpFileName.ToPtr(), ShaderCompiler::GetBufferData(disassembly), ShaderCompiler::GetBufferSize(disassembly));
	//	ShaderCompiler::ReleaseBuffer( disassembly );
	//}

	ShaderCompiler::ShaderMetadata	reflectionMetadata;
	mxTRY(ShaderCompiler::ReflectShaderD3D( compiledCodeData, compiledCodeSize, reflectionMetadata ));
	//DBGOUT("%s '%s' uses %u CBs, %u SSs, %u SRs\n",
	//	shaderTypeString, program.name.ToPtr(), shaderMetadata.numUsedCBs, shaderMetadata.numUsedSRs, shaderMetadata.numUsedSSs);

	mxTRY(MergeMetadataD3D(shaderMetadata, reflectionMetadata));


	ShaderCompiler::ByteCode strippedCode = nil;
	if( options.stripSymbolNames )
	{
		strippedCode = ShaderCompiler::StripShaderD3D( compiledCodeData, compiledCodeSize );
		if( strippedCode != nil )
		{
			compiledCodeData = ShaderCompiler::GetBufferData( strippedCode );
			compiledCodeSize = ShaderCompiler::GetBufferSize( strippedCode );
		}
	}

	shaderByteCodeIndex = shaderCache.AddShaderCode( type, compiledCodeData, compiledCodeSize );

	//DBGOUT("Compiled '%s' in '%s' (%u bytes at [%u])\n",
	//	mxGET_ENUM_TYPE(EShaderType).GetStringByInteger(type), program.name.ToPtr(), compiledCodeSize, shaderCodeIndex);

	ShaderCompiler::ReleaseBuffer( compiledCode );
	ShaderCompiler::ReleaseBuffer( strippedCode );

	return ALL_OK;

#else

	return ERR_UNSUPPORTED_FEATURE;

#endif

}

static ERet CompileProgramD3D(
	ProgramD3D &program,
	AFileInclude* include,
	const FxOptions& options,
	ShaderCache_d& shaderCache,
	const FxShaderEntryD3D& shader
	)
{
	memset(program.shaders, -1, sizeof(program.shaders));

	char *	fileData;
	UINT32	fileSize;
	if( !include->OpenFile(shader.file.ToPtr(), &fileData, &fileSize) )
	{
		ptERROR("Failed to include '%s'.\n", shader.file.ToPtr());
		return ERR_FAILED_TO_OPEN_FILE;
	}

	String sourceFile;
	sourceFile.SetReference(Chars(include->CurrentFilePath()));

	mxTRY(CompileShaderD3D(
		fileData,
		fileSize,
		sourceFile,
		shader.vertex,
		include,
		ShaderVertex,		
		options,
		shaderCache,
		program.shaders[ShaderVertex],
		program.metadata
	));

	mxTRY(CompileShaderD3D(
		fileData,
		fileSize,
		sourceFile,
		shader.geometry,
		include,
		ShaderGeometry,		
		options,
		shaderCache,
		program.shaders[ShaderGeometry],
		program.metadata
	));

	mxTRY(CompileShaderD3D(
		fileData,
		fileSize,
		sourceFile,
		shader.pixel,
		include,
		ShaderFragment,		
		options,
		shaderCache,
		program.shaders[ShaderFragment],
		program.metadata
	));

	return ALL_OK;
}

ERet CompileLibraryD3D(
					   const FxLibraryDescription& library,
					   const FxOptions& options,
					   AFileInclude* include,
					   FxLibrary &library,
					   ShaderCache_d &shaderCache
					   )
{
	mxASSERT(options.target == PC_Direct3D_11);

	mxTRY(CreateResourceObjects(library, library));
	mxTRY(CopyRenderStates(library, library));

	library.m_techniques.SetNum(library.effects.Num());
	for( UINT32 iTechnique = 0; iTechnique < library.effects.Num(); iTechnique++ )
	{
		const FxEffectDescription& techniqueDesc = library.effects[ iTechnique ];
		FxShader& technique = library.m_techniques[ iTechnique ];

		technique.name = techniqueDesc.name;
		technique.UpdateNameHash();

		const UINT32 techniqueBindingsIndex = library.m_bindings.Num();
		ProgramBindingsOGL& techniqueBindings = library.m_bindings.Add();

		// temporary storage for technique data merged from all passes
		ShaderMetadata	techniqueMetadata;

		// temporary storage for pass data
		TArray< PassDataD3D >	passDataArray;

		const UINT32 numPasses = techniqueDesc.passes.Num();

		passDataArray.SetNum(numPasses);

		technique.passes.SetNum(numPasses);
		for( UINT32 iPass = 0; iPass < numPasses; iPass++ )
		{
			const FxPassDescription& passDesc = techniqueDesc.passes[ iPass ];
			PassDataD3D& passData = passDataArray[ iPass ];
			FxPass& pass = technique.passes[ iPass ];

			pass.name = passDesc.name;
			pass.UpdateNameHash();

			const FxShaderEntryD3D& entryPoints = passDesc.D3D;

			const UINT32 numPrograms = 1;
			passData.programs.SetNum(numPrograms);
			for( UINT32 iProgram = 0; iProgram < numPrograms; iProgram++ )
			{
				ProgramD3D& program = passData.programs[ iProgram ];
				mxTRY(CompileProgramD3D(program, include, options, shaderCache, entryPoints));
				mxTRY(MergeMetadataD3D(passData.metadata, program.metadata));
			}

			mxTRY(MergeMetadataD3D(techniqueMetadata, passData.metadata));
		}//For each pass

		ExtractBindings(techniqueBindings, techniqueMetadata);

		technique.defaultId = 0;

		{
			const UINT32 numLocalCBs = techniqueMetadata.cbuffers.Num();
			technique.locals.SetNum(numLocalCBs);
			for( UINT32 iCB = 0; iCB < numLocalCBs; iCB++ )
			{
				const ShaderCBuffer& buffer = techniqueMetadata.cbuffers[ iCB ];
				FxCBuffer& localCB = technique.locals[ iCB ];

				localCB.name = buffer.name;
				localCB.UpdateNameHash();

				localCB.size = buffer.size;
				localCB.handle.SetNil();

				const UINT32 numFields = buffer.fields.Num();
				localCB.uniforms.SetNum(numFields);
				for( UINT32 iUniform = 0; iUniform < numFields; iUniform++ )
				{
					const Field& field = buffer.fields[ iUniform ];
					FxUniform& uniform = localCB.uniforms[ iUniform ];
					uniform.name = field.name;
					uniform.UpdateNameHash();
					uniform.offset = field.offset;
					uniform.size = field.size;
				}
			}

			const UINT32 numSamplers = techniqueBindings.samplers.Num();
			technique.inputs.cbuffers.SetNum(numLocalCBs);
			technique.inputs.samplers.SetNum(numSamplers);

			for( UINT32 iCB = 0; iCB < numLocalCBs; iCB++ )
			{
				const CBufferBindingOGL& binding = techniqueBindings.cbuffers[ iCB ];
				FxCBufferRef& rCB = technique.inputs.cbuffers[ iCB ];

				rCB.name = binding.name;
				rCB.UpdateNameHash();

				const int nCBIndex = FindIndexByName(technique.locals, rCB.name);
				chkRET_X_IF_NOT(nCBIndex != -1, ERR_OBJECT_NOT_FOUND);
				rCB.handle.id = nCBIndex;
				SetUpperBit(rCB.handle.id);
			}
			for( UINT32 iTS = 0; iTS < numSamplers; iTS++ )
			{
				const SamplerBindingOGL& binding = techniqueBindings.samplers[ iTS ];
				FxSamplerRef& rSS = technique.inputs.samplers[ iTS ];

				rSS.name = binding.name;
				rSS.UpdateNameHash();

				rSS.texture.SetNil();
				rSS.sampler.SetNil();
			}
		}

		for( UINT32 iPass = 0; iPass < techniqueDesc.passes.Num(); iPass++ )
		{
			const PassDataD3D& passData = passDataArray[ iPass ];
			FxPass& pass = technique.passes[ iPass ];

			const UINT32 numPrograms = passData.programs.Num();
			pass.programs.SetNum(numPrograms);
			for( UINT32 iProgram = 0; iProgram < numPrograms; iProgram++ )
			{
				const ProgramD3D& program = passData.programs[ iProgram ];
				mxUNUSED(program);

				const UINT32 programIndex = library.m_programs.Num();
				pass.programs[ iProgram ].id = programIndex;

				FxProgram& newProgram = library.m_programs.Add();
				memset(newProgram.shaders, ~0, sizeof(newProgram.shaders));
				newProgram.shaders[ShaderVertex]	= program.shaders[ShaderVertex];
				newProgram.shaders[ShaderGeometry]	= program.shaders[ShaderGeometry];
				newProgram.shaders[ShaderFragment]	= program.shaders[ShaderFragment];
				newProgram.bindings = techniqueBindingsIndex;
			}

			const ShaderMetadata& passBindings = passData.metadata;

			pass.CBs.SetNum(passBindings.cbuffers.Num());
			for( UINT32 iCB = 0; iCB < passBindings.cbuffers.Num(); iCB++ )
			{
				const ShaderCBuffer& buffer = passBindings.cbuffers[ iCB ];
				FxBinding& binding = pass.CBs[ iCB ];
				binding.index = FindIndexByName(technique.inputs.cbuffers, buffer.name);
				chkRET_X_IF_NOT(binding.index != -1, ERR_OBJECT_NOT_FOUND);
				binding.slot = techniqueBindings.cbuffers[ binding.index ].slot;
			}

			pass.TSs.SetNum(passBindings.samplers.Num());
			for( UINT32 iTS = 0; iTS < passBindings.samplers.Num(); iTS++ )
			{
				const ShaderSampler& sampler = passBindings.samplers[ iTS ];
				FxBinding& binding = pass.TSs[ iTS ];
				binding.index = FindIndexByName(technique.inputs.samplers, sampler.name);
				chkRET_X_IF_NOT(binding.index != -1, ERR_OBJECT_NOT_FOUND);
				binding.slot = techniqueBindings.samplers[ binding.index ].slot;
			}

		}//For each pass

	}//For each technique



	for( UINT32 shaderType = 0; shaderType < ShaderTypeCount; shaderType++ )
	{
		library.m_shaders[shaderType].SetNum(shaderCache.m_shaders[shaderType].Num());
	}

	// we don't need bindings in DirectX mode
	library.m_bindings.Empty();

	return ALL_OK;
}


#if 0
	static inline void ID3D11Blob_To_ByteArray( ID3DBlob* blob, TBuffer< BYTE > &buffer )
	{
		buffer.SetNum(blob->GetBufferSize());
		memcpy(buffer.ToPtr(), blob->GetBufferPointer(), buffer.Num());
	}

	static void CompileShader(
		const ByteBuffer& source,
		const FxOptions& options,
		const FxShaderDescription& library,
		const EShaderType shaderType,		
		IR_Program& program,
		ShaderCache_d& compiledShaderCache,		
		const char* disasmComments = nil
		)
	{
		const char* shaderTypeString = EShaderTypeToChars( shaderType );
		chkRET_IF_NIL(shaderTypeString);

		const char* sEntryPoint = library.GetShaderEntryPoint( shaderType );
		chkRET_IF_NIL(sEntryPoint);

		if( !IsValidShaderFunctionName( sEntryPoint ) ) {
			program.codeIndices[shaderType] = -1;
			return;
		}

		ShaderCompiler::Options		compilerOptions;
		if( options.optimizeShaders ) {
			compilerOptions.flags |= ShaderCompiler::Compile_Optimize;
		}

		ShaderCompiler::ByteCode	compiledCode;

		if(mxSUCCEDED(ShaderCompiler::CompileShader(
			source.ToPtr(),
			source.GetDataSize(),
			sEntryPoint,
			shaderType,
			compilerOptions,
			compiledCode
			)))
		{
			void* compiledCodeData = ShaderCompiler::GetBufferData( compiledCode );
			size_t compiledCodeSize = ShaderCompiler::GetBufferSize( compiledCode );

			if( options.dumpDisassembly )
			{
				String512	dumpFileName;
				str::SPrintF( dumpFileName, "%s%s_%s.asm",
					options.outputPath.ToPtr(), program.name.ToPtr(), ShaderToExtension(shaderType) );
				UINT32 disassembleFlags = 0;
				ShaderCompiler::ByteCode disassembly = ShaderCompiler::DisassembleShader(
					compiledCodeData,
					compiledCodeSize,
					disassembleFlags,
					disasmComments
					);
				Util_SaveDataToFile(dumpFileName.ToPtr(), ShaderCompiler::GetBufferData(disassembly), ShaderCompiler::GetBufferSize(disassembly));
				ShaderCompiler::ReleaseBuffer( disassembly );
			}

			ShaderCompiler::ReflectionInfo	reflection;
			if(mxSUCCEDED(ShaderCompiler::ReflectShader( compiledCodeData, compiledCodeSize, reflection )))
			{
				DBGOUT("%s '%s' uses %u CBs, %u SSs, %u SRs\n",
					shaderTypeString, program.name.ToPtr(), reflection.numUsedCBs, reflection.numUsedSRs, reflection.numUsedSSs);

				if( reflection.numUsedCBs ) {
					program.CBMask |= (1 << shaderType);
				}
				if( reflection.numUsedSRs ) {
					program.SRMask |= (1 << shaderType);
				}
				if( reflection.numUsedSSs ) {
					program.SSMask |= (1 << shaderType);
				}
			}

			ShaderCompiler::ByteCode strippedCode = nil;
			if( options.stripSymbolNames )
			{
				strippedCode = ShaderCompiler::StripShader( compiledCodeData, compiledCodeSize );
				if( strippedCode != nil )
				{
					compiledCodeData = ShaderCompiler::GetBufferData( strippedCode );
					compiledCodeSize = ShaderCompiler::GetBufferSize( strippedCode );
				}
			}

			const UINT32 shaderCodeIndex = compiledShaderCache.AddShaderCode( shaderType, compiledCodeData, compiledCodeSize );

			DBGOUT("Compiled '%s' in '%s' (%u bytes at [%u])\n",
				mxGET_ENUM_TYPE(EShaderType).GetStringByInteger(shaderType), program.name.ToPtr(), compiledCodeSize, shaderCodeIndex);

			program.codeIndices[shaderType] = shaderCodeIndex;

			ShaderCompiler::ReleaseBuffer( compiledCode );
			ShaderCompiler::ReleaseBuffer( strippedCode );
		}
	}

	/*
	-----------------------------------------------------------------------------
	ShaderBackendD3D11
	-----------------------------------------------------------------------------
	*/
	ShaderBackendD3D11::ShaderBackendD3D11()
	{
	}

	void ShaderBackendD3D11::GetDefinitions(
		FxDefineSet &definitions
		) const
	{
		FxDefine& newMacro = definitions.Add();
		newMacro.name.SetReference(Chars("PC_D3D11"));
		newMacro.value.SetReference(Chars("1"));
	}

	void ShaderBackendD3D11::GetSpecs(
		ShaderProfileSpecs& specs
		) const
	{
		specs.nUniformBufferSlots = 14;
		specs.nShaderSamplerSlots = 16;
		specs.nShaderTextureSlots = 32;
		specs.nMaximumUniformBufferSize = 64*mxKIBIBYTE;
	}

	ERet ShaderBackendD3D11::CompileProgramD3D(
		const FxShaderDescription& library,
		const FxOptions& options,
		IR_Program & program,
		ShaderCache_d& cache
		) const
	{
		ByteBuffer			shaderSourceCode;
		MemoryBlobWriter	shaderCodeStream( shaderSourceCode );
		TextWriter		shaderCodeStreamWriter( shaderCodeStream );

		HLSLWriter_D3D11	hlslWriter( shaderCodeStreamWriter, program.source.declarations );
		RET_X_IF_NOT(hlslWriter.WriteShaderSourceCode(), ERR_UNKNOWN_ERROR);

		if( options.dumpShaderCode )
		{
			String512	dumpFileName;
			str::Copy(dumpFileName, options.outputPath);
			str::Append(dumpFileName, program.name);
			str::SetFileExtension( dumpFileName, "hlsl" );
			Util_SaveBlobToFile( dumpFileName.ToPtr(), shaderSourceCode, FileWrite_NoErrors );
		}

		// compile the resulting shader program
		if( options.optimizeShaders ) {
			program.CBMask = 0;
			program.SRMask = 0;
			program.SSMask = 0;
		} else {
			program.CBMask = BITS_ALL;
			program.SRMask = BITS_ALL;
			program.SSMask = BITS_ALL;
		}

		for( int shaderTypeIndex = 0; shaderTypeIndex < ShaderTypeCount; shaderTypeIndex++ )
		{
			const EShaderType shaderType = (EShaderType)shaderTypeIndex;
			CompileShader( shaderSourceCode, options, library, shaderType, program, cache );
		}

		return ALL_OK;
	}

	static void BuildConstantBufferData( const UniformBuffer& src, CBufferD3D11 &dst )
	{
		dst.name = src.name;
		dst.UpdateHash();

		dst.size = src.sizeInBytes;
		dst.handle.SetNil();

		dst.uniforms.SetNum(src.fields.Num());
		for( UINT32 i = 0; i < src.fields.Num(); i++ )
		{
			const Uniform& field = src.fields[i];
			UniformD3D11& newUniform = dst.uniforms[ i ];
			newUniform.name = field.name;
			newUniform.UpdateHash();
			newUniform.offset = field.offset;
		}
	}

	ERet ShaderBackendD3D11::CompilePipeline(
		const FxLibraryDescription& pipeline,
		const ShaderCache_d& cache,
		const FxOptions& options,
		ByteBuffer &outputBuffer
		) const
	{
		const UINT32 numTechniques = pipeline.techniques.Num();

		TArray< const UniformBuffer* >	sharedCBs;
		for( UINT32 iTechnique = 0; iTechnique < numTechniques; iTechnique++ )
		{
			const FxTechniqueDescription& technique = pipeline.techniques[ iTechnique ];

			for( UINT32 i = 0; i < technique.data->sharedCBs.Num(); i++ )
			{
				const UniformBuffer* pCB = technique.data->sharedCBs[ i ];
				if( !ContainsNamedItem(sharedCBs, pCB->name) ) {
					sharedCBs.Add( pCB );
				}
			}
		}


		PipelineD3D11	pipeline11;

		pipeline11.sharedVars.SetNum(sharedCBs.Num());
		for( UINT32 iCB = 0; iCB < sharedCBs.Num(); iCB++ )
		{
			const UniformBuffer* pCB = sharedCBs[ iCB ];
			CBufferD3D11& rCB = pipeline11.sharedVars[ iCB ];
			BuildConstantBufferData(*pCB, rCB);
		}

		pipeline11.techniques.SetNum(numTechniques);
		for( UINT32 iTechnique = 0; iTechnique < numTechniques; iTechnique++ )
		{
			const FxTechniqueDescription& technique = pipeline.techniques[ iTechnique ];
			const UINT32 numPasses = technique.passes.Num();

			TechniqueD3D11& tech11 = pipeline11.techniques[ iTechnique ];
			tech11.passes.SetNum(numPasses);
		
			tech11.defaultId = 0;

			tech11.uniforms.SetNum(technique.data->localCBs.Num());
			for( UINT32 iCB = 0; iCB < technique.data->localCBs.Num(); iCB++ )
			{
				const UniformBuffer* pCB = technique.data->localCBs[ iCB ];
				CBufferD3D11& rCB = tech11.uniforms[ iCB ];
				BuildConstantBufferData(*pCB, rCB);
			}

			{
				// initialize references to all constant buffers
				tech11.inputs.cbuffers.SetNum( technique.data->allCBs.Num() );
				for( UINT32 iCB = 0; iCB < technique.data->allCBs.Num(); iCB++ )
				{
					const UniformBuffer* pCB = technique.data->allCBs[ iCB ];
					CBufferRef& rCBReference = tech11.inputs.cbuffers[ iCB ];
					rCBReference.name = pCB->name;
					rCBReference.UpdateHash();
					int bufferIndex = -1;
					if( pCB->isShared ) {
						bufferIndex = FindIndexByName(sharedCBs, pCB->name);
					} else {
						bufferIndex = FindIndexByName(tech11.uniforms, pCB->name);
					}
					mxASSERT(bufferIndex != -1);
					rCBReference.handle.id = bufferIndex;
					if( pCB->isShared ) {
						SetUpperBit(rCBReference.handle.id);
					}
				}

				// initialize external references to all samplers
				tech11.inputs.samplers.SetNum( technique.data->allSSs.Num() );
				for( UINT32 iSS = 0; iSS < technique.data->allSSs.Num(); iSS++ )
				{
					const ShaderSampler* pSS = technique.data->allSSs[ iSS ];
					SamplerRef& rSSReference = tech11.inputs.samplers[ iSS ];
					rSSReference.name = pSS->initializer;
					rSSReference.UpdateHash();
					rSSReference.initializer = pSS->initializer;
					rSSReference.handle.SetNil();
				}

				// initialize external references to all shader resources
				tech11.inputs.textures.SetNum( technique.data->allSRs.Num() );
				for( UINT32 iSR = 0; iSR < technique.data->allSRs.Num(); iSR++ )
				{
					const ShaderResource* pSR = technique.data->allSRs[ iSR ];
					TextureRef& rSRReference = tech11.inputs.textures[ iSR ];
					rSRReference.name = pSR->name;
					rSRReference.UpdateHash();
					rSRReference.initializer = pSR->initializer;
					rSRReference.handle.SetNil();
				}
			}

			for( UINT32 passIndex = 0; passIndex < numPasses; passIndex++ )
			{
				const FxPassDescription& passDesc = technique.passes[passIndex];
				const IR_Pass& pass = technique.data->passes[passIndex];
				PassD3D11 &pass11 = tech11.passes[passIndex];

				const UINT32 numPermutations = pass.programs.Num();
				pass11.programs.SetNum(numPermutations);

				for( UINT32 permutationIndex = 0; permutationIndex < numPermutations; permutationIndex++ )
				{
					const IR_Program& program = pass.programs[permutationIndex];
					ProgramD3D11 &program11 = pass11.programs[permutationIndex];

					program11.VS = program.codeIndices[ShaderVertex];
					program11.HS = program.codeIndices[ShaderHull];
					program11.DS = program.codeIndices[ShaderDomain];
					program11.GS = program.codeIndices[ShaderGeometry];
					program11.PS = program.codeIndices[ShaderFragment];

					program11.CBMask = BITS_ALL;
					program11.SRMask = BITS_ALL;
					program11.SSMask = BITS_ALL;
				}

				pass11.CBs.SetNum(pass.CBs.Num());
				for( UINT32 iCB = 0; iCB < pass.CBs.Num(); iCB++ )
				{
					const UniformBuffer& rCB = pass.CBs[ iCB ];
					Binding& binding = pass11.CBs[ iCB ];
					binding.index = FindIndexByName( tech11.inputs.cbuffers, rCB.name );
					binding.slot = rCB.inputSlot;
				}
				pass11.SRs.SetNum(pass.SRs.Num());
				for( UINT32 iSR = 0; iSR < pass.SRs.Num(); iSR++ )
				{
					const ShaderResource& rSR = pass.SRs[ iSR ];
					Binding& binding = pass11.SRs[ iSR ];
					binding.index = FindIndexByName( tech11.inputs.textures, rSR.name );
					binding.slot = rSR.inputSlot;
				}
				pass11.SSs.SetNum(pass.SSs.Num());
				for( UINT32 iSS = 0; iSS < pass.SSs.Num(); iSS++ )
				{
					const ShaderSampler& rSS = pass.SSs[ iSS ];
					Binding& binding = pass11.SSs[ iSS ];
					if( rSS.initializer.NonEmpty() ) {
						binding.index = FindIndexByName( tech11.inputs.samplers, rSS.initializer );
					} else {
						binding.index = -1;
					}
					binding.slot = rSS.inputSlot;
				}
			}
		}
	
		if( options.dumpPipeline )
		{
			String512	dumpFileName;
			str::Copy(dumpFileName, options.outputPath);
			str::AppendS(dumpFileName, "_pipeline.json");
			JSON::SaveObjectToFile(pipeline11, dumpFileName.ToPtr());
		}

		return ALL_OK;
	}
#endif
#endif
#if 1
ERet FxCompileShader(
	const FxShaderDescription& description,
	const FxOptions& options,
	AFileInclude* include,
	FxShader &shader,
	ShaderCache_d &ñache,
	ShaderMetadata &metadata	// merged from all shader combinations/permutations/variations
	)
{
	shader.name = description.name;
	shader.UpdateNameHash();

	const FxShaderEntryD3D& entry = description.D3D;

	TArray< ProgramD3D >	programs;

	const UINT32 numPermutations = 1;
	shader.programs.SetNum(numPermutations);
	shader.permutations.SetNum(numPermutations);
	for( UINT32 iProgram = 0; iProgram < numPermutations; iProgram++ )
	{
		const UINT32 programIndex = programs.Num();
		shader.programs[iProgram].id = programIndex;

		ProgramD3D& program = programs.Add();

		//Str::Copy(program.name, shader.file);
		Str::Copy(program.name, description.name);

		mxTRY(CompileProgramD3D(program, include, options, ñache, entry));
		mxTRY(MergeMetadataD3D(metadata, program.metadata));

		shader.permutations[iProgram] = iProgram;
	}

	{
		// remove global constant buffers (which are managed by the engine)

		TArray< const ShaderCBuffer* >	localCBuffers;

		for( UINT32 iCB = 0; iCB < metadata.cbuffers.Num(); iCB++ )
		{
			const ShaderCBuffer& buffer = metadata.cbuffers[ iCB ];
			if( !buffer.IsGlobal() )
			{
				localCBuffers.Add( &buffer );
			}
		}

		// Setup local constant buffers and shader resources.

		const UINT32 numCBuffers = localCBuffers.Num();
		shader.CBs.SetNum( numCBuffers );
		shader.localCBs.SetNum( numCBuffers );

		for( UINT32 iCB = 0; iCB < numCBuffers; iCB++ )
		{
			const ShaderCBuffer& buffer = *localCBuffers[ iCB ];
			FxCBuffer* pResource = &shader.localCBs[ iCB ];

			pResource->name = buffer.name;
			pResource->UpdateNameHash();

			pResource->size = buffer.size;
			pResource->handle.SetNil();

			const UINT32 numFields = buffer.fields.Num();
			pResource->uniforms.SetNum(numFields);
			for( UINT32 iUniform = 0; iUniform < numFields; iUniform++ )
			{
				const Field& field = buffer.fields[ iUniform ];
				FxUniform& uniform = pResource->uniforms[ iUniform ];
				uniform.name = field.name;
				uniform.UpdateNameHash();
				uniform.offset = field.offset;
				uniform.size = field.size;
			}

			{
				// allocate buffer for uniform shader parameters
				FxBackingStore& backingStore = pResource->backingStore;
				CreateParameterBuffer( pResource, &backingStore.data );
				backingStore.dirty = false;
				// fill constant buffers with default data
				memcpy(backingStore.data.ToPtr(), buffer.defaults.ToPtr(), buffer.size);
			}

			FxCBufferBinding& cbufferBinding = shader.CBs[ iCB ];
			cbufferBinding.slot = buffer.slot;
			cbufferBinding.id = iCB;
		}
	}

	{
		const UINT32 numSamplers = metadata.samplers.Num();
		shader.SRs.SetNum( numSamplers );
		shader.localSRs.SetNum( numSamplers );
		for( UINT32 iTS = 0; iTS < numSamplers; iTS++ )
		{
			const ShaderSampler& sampler = metadata.samplers[ iTS ];
			FxResource* pResource = &shader.localSRs[ iTS ];

			pResource->name = sampler.name;
			pResource->UpdateNameHash();

			pResource->texture.SetNil();
			pResource->sampler.SetNil();

			FxTextureBinding& samplerBinding = shader.SRs[ iTS ];
			samplerBinding.slot = sampler.slot;
			samplerBinding.id = iTS;
		}
	}

	{
		ñache.m_programs.SetNum(programs.Num());
		for( UINT32 iProgram = 0; iProgram < programs.Num(); iProgram++ )
		{
			const ProgramD3D& program = programs[iProgram];
			CachedProgram_d& cachedProgram = ñache.m_programs[iProgram];
			cachedProgram.pd.name = program.name;
			cachedProgram.pd.UpdateNameHash();
			memcpy(cachedProgram.pd.shaders, program.shaders, sizeof(cachedProgram.pd.shaders));
			ExtractBindings( &cachedProgram.gl, program.metadata );
			cachedProgram.pd.bindings = &cachedProgram.gl;
		}
	}

	return ALL_OK;
}
#endif

#endif // USE_D3D_SHADER_COMPILER

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
