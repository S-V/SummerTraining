#pragma once

#include "Target_Common.h"

ERet CompileLibraryOpenGL(
	const FxLibraryDescription& library,
	const FxOptions& options,
	AFileInclude* include,
	Clump &clump,
	ShaderCache_d &ñache
	);

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
