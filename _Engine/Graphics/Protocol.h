// Communication protocol for executing remote commands (for example, via network)
#pragma once

#include <Core/Core.h>
#include <Graphics/Device.h>
#include <Graphics/Effects.h>

//#if MX_EDITOR
//
//struct PassData : CStruct
//{
//	TArray< Shader >	shaders;
//public:
//	mxDECLARE_CLASS(PassData, CStruct);
//	mxDECLARE_REFLECTION;
//};
//struct TechniqueData : CStruct
//{
//	String				name;
//	TArray< PassData >	passes;
//public:
//	mxDECLARE_CLASS(TechniqueData, CStruct);
//	mxDECLARE_REFLECTION;
//};
//
//struct NetCmd_ReloadShaders : CStruct
//{
//	TArray< TechniqueData >	techniques;
//	FxShaderCache				shaderCode;
//public:
//	mxDECLARE_CLASS(NetCmd_ReloadShaders, CStruct);
//	mxDECLARE_REFLECTION;
//};
//
//#endif // MX_EDITOR

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
