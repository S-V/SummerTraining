// platform-independent 'low-level' shader compiler interface
#pragma once

#if MX_AUTOLINK
#pragma comment( lib, "ShaderCompiler.lib" )
#endif //MX_AUTOLINK

#include <Core/Text/Preprocessor.h>
#include <Graphics/Device.h>
#include <Graphics/Effects.h>

#define USE_D3D_SHADER_COMPILER	(LLGL_Driver_Is_Direct3D)
#define USE_OGL_SHADER_COMPILER	(LLGL_Driver_Is_OpenGL)

//#define REFLECT_GLOBAL_CONSTANT_BUFFERS		(0)

namespace Shaders
{
	//=====================================================
	//		SHADER REFLECTION
	//=====================================================
	struct Field
	{
		String	name;
		//String	type;
		UINT32	size;
		UINT32	offset;
		//TBuffer< BYTE >	defaults;
	};
	struct ShaderCBuffer
	{
		String	name;
		UINT32	size;
		UINT8	slot;	// not fixed in OpenGL
		TArray< Field >	fields;
		TArray< BYTE >	defaults;
	public:
		bool IsGlobal() const;
	};
	struct ShaderSampler
	{
		String	name;
		UINT8	slot;	// not fixed in OpenGL
	};
	struct ShaderMetadata
	{
		TArray< ShaderCBuffer >	cbuffers;	// uniform block bindings
		TArray< ShaderSampler >	samplers;	// shader sampler bindings
		UINT32	vertexAttribMask;	// active attributes mask (OpenGL-only)
		UINT32	instructionCount;	// Direct3D-only
	public:
		ShaderMetadata();
		void Clear();
	};

	extern const Chars DEFAULT_GLOBAL_UBO;	// name of the default global constant buffer
	extern const Chars GLOBAL_UBO_PREFIX;	// engine-managed UBOs start with this prefix

	//=====================================================
	//		DIRECT 3D
	//=====================================================

#if USE_D3D_SHADER_COMPILER

	enum CompilationFlags
	{
		// Will produce best possible code but may take significantly longer to do so.
		// This will be useful for final builds of an application where performance is the most important factor.
		Compile_Optimize = BIT(0),

		Compile_AvoidFlowControl = BIT(2),

		//Compile_RowMajorMatrices = BIT(3),

		Compile_DefaultFlags = Compile_Optimize
	};

	struct Macro
	{
		const char* name;
		const char* value;
	};

	typedef AFileInclude Include;

	struct Options
	{
		Macro *		defines;
		int			numDefines;

		Include *	include;

		UINT32		flags;	// CompilationFlags

	public:
		Options();
	};

	typedef void* ByteCode;

	void* GetBufferData( ByteCode buffer );
	size_t GetBufferSize( ByteCode buffer );
	void ReleaseBuffer( ByteCode &buffer );

	ERet CompileShaderD3D(
		ByteCode &compiledCode,
		const char* sourceCode,
		size_t sourceCodeLength,
		const char* entryPoint,
		EShaderType shaderType,
		const Options* options = nil,
		const char* sourceFile = nil	// name of source file for debugging
	);

	ERet CompileShaderFromFile(
		ByteCode &compiledCode,
		const char* sourceFile,
		const char* entryPoint,
		EShaderType shaderType,
		const Options* options = nil
	);

	enum DisassembleFlags
	{
		DISASM_NoDebugInfo	= BIT(0),
		DISASM_WriteAsHTML	= BIT(1),
	};

	ByteCode DisassembleShaderD3D(
		const void* compiledCode,
		size_t compiledCodeLength,
		UINT32 disassembleFlags,
		const char* comments
	);

	ERet ReflectShaderD3D(
		const void* compiledCode,
		size_t compiledCodeLength,
		ShaderMetadata &metadata
	);

	// removes all metadata from compiled byte code
	ByteCode StripShaderD3D(
		const void* compiledCode,
		size_t compiledCodeLength
	);

#endif // USE_D3D_SHADER_COMPILER

	//=====================================================
	//		OPENGL
	//	NOTE: a valid OpenGL context must be created
	//	before calling these functions!
	//=====================================================

#if USE_OGL_SHADER_COMPILER

	ERet GetProgramBinaryFormatsGL( TArray< INT32 > &formats );

	struct SourceCode
	{
		const char *text;	//<= can be null
		UINT32		size;	//<= if 0, then will be auto-calculated
		//UINT32		line;	//<= start line in the source file
	public:
		SourceCode()
		{
			text = nil;
			size = 0;
			//line = 1;
		}
	};
	ERet CompileProgramGL(
		SourceCode source[ShaderTypeCount],
		ShaderMetadata &metadata,
		TArray< BYTE > &programBinary,	// valid if binaryProgramFormat != 0
		INT32 usedBinaryProgramFormat = 0
	);
	//ERet CompileProgramGL(
	//	const char* source,
	//	const UINT32 length,
	//	const char* fileName,
	//	const UINT32 startLine,
	//	ShaderMetadata &metadata,
	//	TArray< BYTE > &programBinary,	// valid if binaryProgramFormat != 0
	//	INT32 usedBinaryProgramFormat = 0
	//);
#endif // USE_OGL_SHADER_COMPILER

	//=====================================================
	//		UTILITIES
	//=====================================================

	ERet MergeMetadataOGL(
		ShaderMetadata &destination,
		const ShaderMetadata &source
	);
	ERet MergeMetadataD3D(
		ShaderMetadata &destination,
		const ShaderMetadata &source
	);
	void ExtractBindings(
		ProgramBindingsOGL *bindings,
		const ShaderMetadata& metadata
	);
}//namespace Shaders
