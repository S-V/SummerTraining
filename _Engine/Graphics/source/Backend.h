// common header file to be included by each back-end
#pragma once

#include "Frontend.h"

namespace llgl
{

// these functions must be implemented by the backend:

ERet driverInitialize( const void* context );
void driverShutdown();

HContext driverGetMainContext();
HContext driverCreateContext();
void driverDeleteContext( HContext handle );

HInputLayout driverCreateInputLayout( const VertexDescription& desc, const char* name );
void driverDeleteInputLayout( HInputLayout handle );

HTexture driverCreateTexture( const void* data, UINT size );
HTexture driverCreateTexture2D( const Texture2DDescription& txInfo, const void* imageData = NULL );
HTexture driverCreateTexture3D( const Texture3DDescription& txInfo, const Memory* initialData = NULL );
void driverDeleteTexture( HTexture handle );

HColorTarget driverCreateColorTarget( const ColorTargetDescription& rtInfo );
void driverDeleteColorTarget( HColorTarget rt );

HDepthTarget driverCreateDepthTarget( const DepthTargetDescription& dtInfo );
void driverDeleteDepthTarget( HDepthTarget dt );

HDepthStencilState driverCreateDepthStencilState( const DepthStencilDescription& dsInfo );
HRasterizerState driverCreateRasterizerState( const RasterizerDescription& rsInfo );
HSamplerState driverCreateSamplerState( const SamplerDescription& ssInfo );
HBlendState driverCreateBlendState( const BlendDescription& bsInfo );
void driverDeleteDepthStencilState( HDepthStencilState ds );
void driverDeleteRasterizerState( HRasterizerState rs );
void driverDeleteSamplerState( HSamplerState ss );
void driverDeleteBlendState( HBlendState bs );

HBuffer driverCreateBuffer( EBufferType type, const void* data, UINT size );
void driverDeleteBuffer( HBuffer handle );

HShader driverCreateShader( EShaderType shaderType, const void* compiledBytecode, UINT bytecodeLength );
void driverDeleteShader( HShader handle );

HProgram driverCreateProgram( const ProgramDescription& pd );
void driverDeleteProgram( HProgram handle );

HResource driverGetShaderResource( HBuffer br );
HResource driverGetShaderResource( HTexture tx );
HResource driverGetShaderResource( HColorTarget rt );
HResource driverGetShaderResource( HDepthTarget dt );

void driverSubmitFrame( CommandBuffer & commands, UINT size );

}//namespace llgl

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
