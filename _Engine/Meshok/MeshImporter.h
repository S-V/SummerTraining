#pragma once

#include <Meshok/Meshok.h>

namespace Meshok
{

enum EOptLevel
{
	OL_None,
	OL_Fast,	// fast optimizations for real-time quality
	OL_High,	// high level of quality
	OL_Max		// maximum level of quality (slow)
};

ERet ImportMesh( AStreamReader& _source, TcMeshData &_output, const char* _hint = "" );
//ERet ImportMesh( AStreamReader& _source, TcMeshData &_output, EOptLevel _level, const ImportSettings& _settings );
ERet ImportMeshFromFile( const char* _path, TcMeshData &_mesh );

}//namespace Meshok

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
