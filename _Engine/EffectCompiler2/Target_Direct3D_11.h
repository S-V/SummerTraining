#pragma once

#include "Target_Common.h"

ERet CompileLibraryD3D11(
	const FxLibraryDescription& src,
	const FxOptions& options,
	AFileInclude* include,
	Clump &clump,
	ShaderCache_d &ñache
	);

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
