// effect compiler library interface, public header file
#pragma once

#if MX_AUTOLINK
	#pragma comment( lib, "EffectCompiler2.lib" )
#endif

#include <Graphics/Effects.h>
// needed for vertex types
#include <Graphics/Geometry.h>

enum EShaderTarget {
	PC_Direct3D_11,
	PC_OpenGL_4plus,
};
mxDECLARE_ENUM( EShaderTarget, UINT32, ShaderTargetT );

struct FxOptions : CStruct
{
///	ShaderTargetT	target;

	TArray< FxDefine > defines;

	StringListT	search_paths;	// paths for opening source files
	//String	pathToGenCppFiles;	// path to generated .h/.cpp files

	//// testing and debugging
	//String		outputPath;
	//String	whereToSavePreprocessedShaderCode;
	//String	whereToSaveDisassembledShaderCode;
	//String	whereToSaveParsedShaderCode;	// folder to save source tree dump into
	//String	whereToSaveGeneratedShaderCode;
	//bool		dumpPreprocessedShaders;
	//bool		dumpDisassembledShaders;

	// whereToSaveDisassembledShaderCode
	String	debugDumpPath;

	bool	dumpPipeline;		// dump text-serialized library?
	bool	dumpShaderCode;		// save generated HLSL code on disk?
	bool	optimizeShaders;
	//bool	dumpDisassembly;
	bool	stripReflection;	// remove shader metadata
	bool	stripSymbolNames;	// remove string names (but leave name hashes)
	bool	generateCppHeaders;	// generate C/C++ headers?

public:
	mxDECLARE_CLASS( FxOptions, CStruct );
	mxDECLARE_REFLECTION;
	FxOptions();
};

// Compiles the source asset file into two parts:
// 1) the memory-resident part (a clump containing effect library structures);
// 2) the temporary data that is used only for initializing run-time structures and exists only during loading (compiled shader code);
ERet FxCompileEffectFromFile(const char* filename,
							 ByteArrayT &effectBlob,
							 const FxOptions& options);

ERet FxCompileAndSaveEffect(const char* sourceFile,
							const char* destination,
							const FxOptions& options);

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
