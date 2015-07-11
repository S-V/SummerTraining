/*
=============================================================================
	File:	PC_D3D11.cpp
	Desc:	
=============================================================================
*/
#include <Base/Base.h>
#pragma hdrstop

#include <Core/Text/Token.h>
#include <Core/Text/TextWriter.h>
#include <Core/Serialization.h>
#include <Graphics/Device.h>
#include <Graphics/Effects.h>
#include <ShaderCompiler/ShaderCompiler.h>

#include "Target_OpenGL.h"

#if USE_OGL_SHADER_COMPILER

using namespace Shaders;

struct ProgramOGL
{
	UINT16			shaders[ShaderTypeCount];
	ShaderMetadata	metadata;
};

enum GLSLVersion
{
	GLSL_3_2,
	GLSL_3_3,
	GLSL_4_0,
	GLSL_4_2,
	GLSL_4_3,
};
const char* GetGLSLVersionDefine( GLSLVersion version )
{
	switch( version )
	{
	case GLSL_3_2:	return "#version 150\n";
	case GLSL_3_3:	return "#version 330\n";
	case GLSL_4_0:	return "#version 400\n";
	case GLSL_4_2:	return "#version 420\n";
	case GLSL_4_3:	return "#version 430\n";
		mxNO_SWITCH_DEFAULT;
	}
	return NULL;
}

static ERet LoadFile( AFileInclude* include, const String& file, ScopedStackAlloc& allocator, SourceCode &code )
{
	if( !file.IsEmpty() )
	{
#if 0
		Preprocessor	preprocessor;

		// Preprocess shader source code.
		// This resolves all #defines and #includes,
		// providing a self-contained shader for subsequent compilation.
		TokenStream		preprocessedSource;
		mxTRY(preprocessor.PreprocessFile( file.ToPtr(), include, preprocessedSource ));

		mxStreamWriter_CountBytes	countLength;
		preprocessedSource.Dump_HumanReadable(countLength);

		const UINT32 textLength = countLength.NumBytesWritten();
		void* buffer = allocator.AllocateAligned( textLength );

		MemoryWriter	stream(buffer, textLength);
		preprocessedSource.Dump_HumanReadable(stream);

		code.text = (char*) buffer;
		code.size = textLength;
#else
		char *	fileData;
		UINT	fileSize;
		if( !include->OpenFile(file.ToPtr(), &fileData, &fileSize) )
		{
			return ERR_FAILED_TO_OPEN_FILE;
		}

		code.text = fileData;
		code.size = fileSize;
#endif
	}
	return ALL_OK;
}

static UINT32 GetShaderCodeIndex( ShaderCache_d& shaderCache, EShaderType type, const SourceCode& code )
{
	if( code.text )
	{
		return shaderCache.AddShaderCode(type, code.text, code.size);
	}
	return -1;
}

//static void SaveShaderCodeToFile( const String& file, const SourceCode& code )
//{
//	if( !file.IsEmpty() )
//	{
//		String256	temp(file);
//		Str::StripPath(temp);
//
//		String256	fileName;
//		Str::CopyS(fileName, "R:/temp/");
//		Str::Append(fileName, temp);
//		Str::AppendS(fileName, ".cxx");
//		Util_SaveDataToFile(fileName.ToPtr(), code.text, code.size);
//	}
//}

static int SkipWhitespace() {
	//
}

static ERet CompileProgramOGL(
	ProgramOGL &program,
	AFileInclude* include,
	const FxOptions& options,
	ShaderCache_d& shaderCache,
	const FxShaderEntryOGL& entry
	)
{
	ScopedStackAlloc	allocator(gCore.frameAlloc);

	//SourceCode	source;
	//mxTRY(LoadFile(include, entry.file, allocator, source));

	char *	fileData;
	UINT32	fileSize;
	if( !include->OpenFile(entry.file.ToPtr(), &fileData, &fileSize) )
	{
		ptERROR("Failed to open '%s'.\n", entry.file.ToPtr());
		return ERR_FAILED_TO_OPEN_FILE;
	}

	// NOTE: we write "#line X" in place of markers
	// so the markers must have a big enough length.
	static const Chars s_markers[ShaderTypeCount] =
	{
		"$VertexShader",
		"$HullShader",
		"$DomainShader",
		"$GeometryShader",
		"$FragmentShader",
	};

	Shaders::SourceCode	sourceStrings[ShaderTypeCount];
	int currentLine = 1;
	char* text = fileData;
	for( int type = 0; type < ShaderTypeCount; type++ )
	{
		const Chars& marker = s_markers[type];
		char* start = ::strstr(text, marker.buffer);
		if( start )
		{
			char* p = start;

			for( int i = 0; i < marker.length; i++ ) {
				*p++ = '\0';
			}

			// skip whitespace
			while( *p && isspace(*p) ) {
				if( *p == '\n' ) {
					currentLine++;
				}
				p++;
			}

			// this must be greater than strlen("#line 99999")
			const int whitespaceLength = p - start;

			char	lineMacro[16];
			const int macroLength = snprintf(lineMacro, mxCOUNT_OF(lineMacro), "#line %d\n", currentLine);
			mxASSERT(whitespaceLength > macroLength);

			start = start + (whitespaceLength - macroLength);
			strncpy(start, lineMacro, macroLength);

			sourceStrings[type].text = start;
			//sourceStrings[type].line = currentLine;

			// skip whitespace until the next section
			while( *p && *p != '$' ) {
				if( *p == '\n' ) {
					currentLine++;
				}
				p++;
			}
			sourceStrings[type].size = p - sourceStrings[type].text;

			text = p;

			//Util_SaveDataToFile(sourceStrings[type].text,sourceStrings[type].size,Str::SPrintF<String32>("R:/%s.glsl", g_shaderTypeName[type].buffer).ToPtr());
		}
	}

	TArray< BYTE >	programBinary;
	mxTRY(Shaders::CompileProgramGL(
		sourceStrings,
		program.metadata,
		programBinary/*, binaryFormat*/
		));

	for( int type = 0; type < ShaderTypeCount; type++ )
	{
		const Shaders::SourceCode& code = sourceStrings[type];
		if( code.text ) {
			program.shaders[type] = shaderCache.AddShaderCode((EShaderType)type, code.text, code.size);
		} else {
			program.shaders[type] = -1;
		}
	}

	return ALL_OK;
}

ERet CompileLibraryOpenGL(
						  const FxLibraryDescription& library,
						  const FxOptions& options,
						  AFileInclude* include,
						  Clump &clump,
						  ShaderCache_d &ñache
					   )
{
	mxDO(CreateResourceObjects(library, clump));

	const UINT numShaders = library.shader_programs.Num();
	if( numShaders > 0 ) {
		clump.CreateObjectList< FxShader >( numShaders );
	}

	TArray< ProgramOGL >	programs;

	for( UINT iShader = 0; iShader < numShaders; iShader++ )
	{
		const FxShaderDescription& shaderDesc = library.shader_programs[ iShader ];
		const FxShaderEntryOGL& entry = shaderDesc.OGL;
		FxShader* shader = clump.New< FxShader >();

		shader->name = shaderDesc.name;
		shader->UpdateNameHash();

		ShaderMetadata	metadata;

		const UINT numPermutations = 1;
		shader->programs.SetNum(numPermutations);
		shader->permutations.SetNum(numPermutations);
		for( UINT iProgram = 0; iProgram < numPermutations; iProgram++ )
		{
			ProgramOGL& program = programs.Add();
			mxTRY(CompileProgramOGL(program, include, options, ñache, entry));
			mxTRY(MergeMetadataOGL(metadata, program.metadata));
		}

		DBGOUT("Program '%s':\n", shaderDesc.name.ToPtr());

		const UINT numCBuffers = metadata.cbuffers.Num();
		shader->CBs.SetNum( numCBuffers );

		const UINT numSamplers = metadata.samplers.Num();
		shader->SRs.SetNum( numSamplers );

		for( UINT iCB = 0; iCB < numCBuffers; iCB++ )
		{
			const ShaderCBuffer& buffer = metadata.cbuffers[ iCB ];
			FxCBuffer* pResource = clump.New< FxCBuffer >( 64 );

			pResource->name = buffer.name;
			pResource->UpdateNameHash();

			pResource->size = buffer.size;
			pResource->handle.SetNil();

			const UINT numFields = buffer.fields.Num();
			pResource->uniforms.SetNum(numFields);
			for( UINT iUniform = 0; iUniform < numFields; iUniform++ )
			{
				const Field& field = buffer.fields[ iUniform ];
				FxUniform& uniform = pResource->uniforms[ iUniform ];
				uniform.name = field.name;
				uniform.UpdateNameHash();
				uniform.offset = field.offset;
				uniform.size = field.size;
			}

			FxCBufferBinding& cbufferBinding = shader->CBs[ iCB ];
			cbufferBinding.resource = pResource;
			cbufferBinding.slot = buffer.slot;

			DBGOUT("\tCB '%s' at %u (%u bytes)\n", buffer.name.ToPtr(), buffer.slot, buffer.size);
		}

		for( UINT iTS = 0; iTS < numSamplers; iTS++ )
		{
			const ShaderSampler& sampler = metadata.samplers[ iTS ];
			FxResource* pResource = clump.New< FxResource >( 64 );

			pResource->name = sampler.name;
			pResource->UpdateNameHash();

			pResource->texture.SetNil();
			pResource->sampler.SetNil();

			FxTextureBinding& samplerBinding = shader->SRs[ iTS ];
			samplerBinding.resource = pResource;
			samplerBinding.slot = sampler.slot;

			DBGOUT("\tTS '%s' at %u\n", sampler.name.ToPtr(), sampler.slot);
		}
	}

	ñache.m_programs.SetNum(programs.Num());
	for( UINT iProgram = 0; iProgram < programs.Num(); iProgram++ )
	{
		const ProgramOGL& program = programs[iProgram];
		CachedProgram_d& cachedProgram = ñache.m_programs[iProgram];
		memcpy(cachedProgram.pd.shaders, program.shaders, sizeof(HShader)*ShaderTypeCount);
		ExtractBindings( &cachedProgram.gl, program.metadata );
		cachedProgram.pd.bindings = &cachedProgram.gl;
	}

#if 0


	// temporary storage for pass data
	TArray< PassDataOGL >	passDataArray;
	passDataArray.Reserve(4);

	// temporary storage for shader data merged from all passes
	ShaderMetadata	metadata;

	clump.m_techniques.SetNum(src.effects.Num());
	for( UINT iTechnique = 0; iTechnique < src.effects.Num(); iTechnique++ )
	{
		const FxEffectDescription& techniqueDesc = src.effects[ iTechnique ];
		FxShader& shader = clump.m_techniques[ iTechnique ];

		shader->name = techniqueDesc.name;
		shader->UpdateNameHash();

		const UINT32 techniqueBindingsIndex = clump.m_bindings.Num();
		ShaderInputBindings& techniqueBindings = clump.m_bindings.Add();

		const UINT numPasses = techniqueDesc.passes.Num();
		for( UINT i = 0; i < passDataArray.Num(); i++ ) {
			passDataArray[i].Clear();
		}
		passDataArray.SetNum(numPasses);

		metadata.Clear();

		shader->passes.SetNum(numPasses);
		for( UINT iPass = 0; iPass < numPasses; iPass++ )
		{
			const FxPassDescription& passDesc = techniqueDesc.passes[ iPass ];
			PassDataOGL& passData = passDataArray[ iPass ];
			FxPass& pass = shader->passes[ iPass ];

			pass.name = passDesc.name;
			pass.UpdateNameHash();

			const FxShaderEntryOGL& entryPoints = passDesc.OGL;

			const UINT numPrograms = 1;
			passData.programs.SetNum(numPrograms);
			for( UINT iProgram = 0; iProgram < numPrograms; iProgram++ )
			{
				ProgramOGL& program = passData.programs[ iProgram ];
				mxTRY(CompileProgramOGL(program, include, options, ñache, entryPoints));
				DumpMetadata(techniqueDesc, program.metadata);
				mxTRY(MergeMetadataOGL(passData.metadata, program.metadata));
			}

			mxTRY(MergeMetadataOGL(metadata, passData.metadata));
		}//For each pass

		ExtractBindings(techniqueBindings, metadata);

		shader->defaultId = 0;

		{
			const UINT numLocalCBs = metadata.cbuffers.Num();
			shader->locals.SetNum(numLocalCBs);
			for( UINT iCB = 0; iCB < numLocalCBs; iCB++ )
			{
				const ShaderCBuffer& buffer = metadata.cbuffers[ iCB ];
				FxCBuffer& localCB = shader->locals[ iCB ];

				pCB->name = buffer.name;
				pCB->UpdateNameHash();

				pCB->size = buffer.size;
				pCB->handle.SetNil();

				const UINT numFields = buffer.fields.Num();
				pCB->uniforms.SetNum(numFields);
				for( UINT iUniform = 0; iUniform < numFields; iUniform++ )
				{
					const Field& field = buffer.fields[ iUniform ];
					FxUniform& uniform = pCB->uniforms[ iUniform ];
					uniform.name = field.name;
					uniform.UpdateNameHash();
					uniform.offset = field.offset;
					uniform.size = field.size;
				}
			}

			const UINT numSamplers = techniqueBindings.samplers.Num();
			shader->inputs.cbuffers.SetNum(numLocalCBs);
			shader->inputs.samplers.SetNum(numSamplers);

			for( UINT iCB = 0; iCB < numLocalCBs; iCB++ )
			{
				const CBufferBinding& binding = techniqueBindings.uniforms[ iCB ];
				FxCBufferRef& rCB = shader->inputs.cbuffers[ iCB ];

				rCB.name = binding.name;
				rCB.UpdateNameHash();

				const int nCBIndex = FindIndexByName(shader->locals, rCB.name);
				chkRET_X_IF_NOT(nCBIndex != -1, ERR_OBJECT_NOT_FOUND);
				rCB.handle.id = nCBIndex;
				SetUpperBit(rCB.handle.id);
			}
			for( UINT iTS = 0; iTS < numSamplers; iTS++ )
			{
				const SamplerBinding& binding = techniqueBindings.samplers[ iTS ];
				FxSamplerRef& rSS = shader->inputs.samplers[ iTS ];

				pResource->name = binding.name;
				pResource->UpdateNameHash();

				pResource->texture.SetNil();
				pResource->sampler.SetNil();
			}
		}

		for( UINT iPass = 0; iPass < techniqueDesc.passes.Num(); iPass++ )
		{
			const PassDataOGL& passData = passDataArray[ iPass ];
			FxPass& pass = shader->passes[ iPass ];

			const UINT numPrograms = passData.programs.Num();
			pass.programs.SetNum(numPrograms);
			for( UINT iProgram = 0; iProgram < numPrograms; iProgram++ )
			{
				const ProgramOGL& program = passData.programs[ iProgram ];
				mxUNUSED(program);

				const UINT programIndex = clump.m_programs.Num();
				pass.programs[ iProgram ].id = programIndex;

				FxProgram& newProgram = clump.m_programs.Add();
				memset(newProgram.shaders, ~0, sizeof(newProgram.shaders));
				newProgram.shaders[ShaderVertex]	= program.shaders[ShaderVertex];
				newProgram.shaders[ShaderGeometry]	= program.shaders[ShaderGeometry];
				newProgram.shaders[ShaderFragment]	= program.shaders[ShaderFragment];
				newProgram.bindings = techniqueBindingsIndex;
			}

			const ShaderMetadata& passBindings = passData.metadata;

			pass.CBs.SetNum(passBindings.cbuffers.Num());
			for( UINT iCB = 0; iCB < passBindings.cbuffers.Num(); iCB++ )
			{
				const ShaderCBuffer& buffer = passBindings.cbuffers[ iCB ];
				FxBinding& binding = pass.CBs[ iCB ];
				binding.index = FindIndexByName(shader->inputs.cbuffers, buffer.name);
				chkRET_X_IF_NOT(binding.index != -1, ERR_OBJECT_NOT_FOUND);
				binding.slot = techniqueBindings.uniforms[ binding.index ].slot;
			}

			pass.TSs.SetNum(passBindings.samplers.Num());
			for( UINT iTS = 0; iTS < passBindings.samplers.Num(); iTS++ )
			{
				const ShaderSampler& sampler = passBindings.samplers[ iTS ];
				FxBinding& binding = pass.TSs[ iTS ];
				binding.index = FindIndexByName(shader->inputs.samplers, sampler.name);
				chkRET_X_IF_NOT(binding.index != -1, ERR_OBJECT_NOT_FOUND);
				binding.slot = techniqueBindings.samplers[ binding.index ].slot;
			}

		}//For each pass

	}//For each shader

	
	for( UINT shaderType = 0; shaderType < ShaderTypeCount; shaderType++ )
	{
		clump.m_shaders[shaderType].SetNum(ñache.m_shaders[shaderType].Num());
	}
#endif
#if 0
	if( options.generateCppHeaders )
	{
		FileWriter	file;
		mxTRY(file.Open("R:/bindings->h"));
		TextWriter	tw(file);

		Emit_ShaderCompiler_Preamble(tw);

		for( int iTechnique = 0; iTechnique < library.m_techniques.Num(); iTechnique++ )
		{
			const FxShader& shader = library.m_techniques[ iTechnique ];

			tw.PrintF("namespace %s\n", shader->name.ToPtr());
			const TextScope	techniqueScope(tw);

			for( int iCB = 0; iCB < shader->locals.Num(); iCB++ )
			{
				const FxCBuffer& rCB = shader->locals[ iCB ];

				tw.PrintF("struct %s // %d bytes\n", rCB.name.ToPtr(), rCB.size);
				const TextScope	cbufferScope(tw, TextScope::EndWithSemicolon);

				int currentOffset = 0;
				int paddingNumber = 0;
				for( int iUniform = 0; iUniform < rCB.uniforms.Num(); iUniform++ )
				{
					const FxUniform& uniform = rCB.uniforms[ iUniform ];

					const int padding = currentOffset % CONSTANT_BUFFER_ALIGNMENT;
					if( padding != 0 )
					{
						tw.PrintF("char	_pad%d[%d];\n", paddingNumber, padding);
						currentOffset += padding;
					}

					tw.PrintF("char	%s[%d]; // offset: %d, size: %d bytes\n",
						uniform.name.ToPtr(), uniform.size, uniform.offset, uniform.size);

					currentOffset += uniform.size;
				}
			}
		}

		Emit_End_Of_File(tw);
	}
#endif

	return ALL_OK;
}

#endif // USE_OGL_SHADER_COMPILER

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
