#include <Base/Base.h>
#pragma hdrstop
#include <Core/Core.h>
#include <Core/ObjectModel.h>
#include <ShaderCompiler/ShaderCompiler.h>
#include "Target_Common.h"

mxDEFINE_CLASS(FxStateBlockDescription);
mxBEGIN_REFLECTION(FxStateBlockDescription)
	mxMEMBER_FIELD(blendFactorRGBA),
	mxMEMBER_FIELD(sampleMask),
	mxMEMBER_FIELD(blendState),
	mxMEMBER_FIELD(depthStencilState),
	mxMEMBER_FIELD(stencilRef),
	mxMEMBER_FIELD(rasterizerState),
mxEND_REFLECTION;
FxStateBlockDescription::FxStateBlockDescription()
{
	stencilRef = 0;
	blendFactorRGBA = RGBAf::BLACK;
	sampleMask = ~0;
}

mxDEFINE_CLASS(FxColorTargetDescription);
mxBEGIN_REFLECTION(FxColorTargetDescription)
	mxMEMBER_FIELD(format),
mxEND_REFLECTION;
FxColorTargetDescription::FxColorTargetDescription()
{
	format = PixelFormat::Unknown;
}

mxDEFINE_CLASS(FxDepthTargetDescription);
mxBEGIN_REFLECTION(FxDepthTargetDescription)
	mxMEMBER_FIELD(format),
	mxMEMBER_FIELD(sample),
mxEND_REFLECTION;
FxDepthTargetDescription::FxDepthTargetDescription()
{
	format = DepthStencilFormat::D24S8;
	sample = false;
}

mxDEFINE_CLASS(FxDefineDescription);
mxBEGIN_REFLECTION(FxDefineDescription)
	mxMEMBER_FIELD(name),
	mxMEMBER_FIELD(tooltip),
	mxMEMBER_FIELD(UIName),
	mxMEMBER_FIELD(UIWidget),
	mxMEMBER_FIELD(defaultValue),
mxEND_REFLECTION;
FxDefineDescription::FxDefineDescription()
{
	UIWidget.SetReference("CheckBox");
}

mxDEFINE_CLASS(FxShaderEntryD3D);
mxBEGIN_REFLECTION(FxShaderEntryD3D)
	mxMEMBER_FIELD(file),
	mxMEMBER_FIELD(vertex),
	mxMEMBER_FIELD(hull),
	mxMEMBER_FIELD(domain),
	mxMEMBER_FIELD(geometry),
	mxMEMBER_FIELD(pixel),
mxEND_REFLECTION;
const String& FxShaderEntryD3D::GetEntryFunction( EShaderType _type ) const
{
	switch( _type )
	{
	case ShaderVertex:		return vertex;
	case ShaderHull:		return hull;
	case ShaderDomain:		return domain;
	case ShaderGeometry:	return geometry;
	case ShaderFragment:	return pixel;
	mxDEFAULT_UNREACHABLE;
	}
	return String();
}
mxDEFINE_CLASS(FxShaderEntryOGL);
mxBEGIN_REFLECTION(FxShaderEntryOGL)
	mxMEMBER_FIELD(file),
mxEND_REFLECTION;

mxDEFINE_CLASS(FxShaderDescription);
mxBEGIN_REFLECTION(FxShaderDescription)
	mxMEMBER_FIELD(name),
	mxMEMBER_FIELD(info),
	mxMEMBER_FIELD(D3D),
	mxMEMBER_FIELD(OGL),
	mxMEMBER_FIELD(defines),
	mxMEMBER_FIELD(supportedVertexTypes),
	mxMEMBER_FIELD(mirrored_uniform_buffers),
mxEND_REFLECTION;
FxShaderDescription::FxShaderDescription()
{
	// By default, create a mirror copy in RAM for the global constant buffer.
	String globalCBufferID;
	globalCBufferID.SetReference( Shaders::DEFAULT_GLOBAL_UBO );
	mirrored_uniform_buffers.Add( globalCBufferID );
}

mxDEFINE_CLASS(FxResourceDescriptions);
mxBEGIN_REFLECTION(FxResourceDescriptions)
	mxMEMBER_FIELD(color_targets),
	mxMEMBER_FIELD(depth_targets),
	mxMEMBER_FIELD(depth_stencil_states),
	mxMEMBER_FIELD(rasterizer_states),
	mxMEMBER_FIELD(sampler_states),
	mxMEMBER_FIELD(blend_states),
	mxMEMBER_FIELD(state_blocks),
mxEND_REFLECTION;
FxResourceDescriptions::FxResourceDescriptions()
{
}

mxDEFINE_CLASS(FxLibraryDescription);
mxBEGIN_REFLECTION(FxLibraryDescription)
	mxMEMBER_FIELD(shader_programs),
mxEND_REFLECTION;
FxLibraryDescription::FxLibraryDescription()
{
}

ERet CreateResourceObjects(
						   const FxResourceDescriptions& descriptions,
						   Clump &clump
						   )
{
	FxRenderResources* resources;
	mxDO(GoC_SingleInstance(clump, resources));

	// render targets

	const UINT32 numColorTargets = descriptions.color_targets.Num();
	if( numColorTargets > 0 )
	{
		mxDO(resources->color_targets.ReserveMore( numColorTargets ));
		for( UINT32 iColorTarget = 0; iColorTarget < numColorTargets; iColorTarget++ )
		{
			const FxColorTargetDescription& colorTargetDesc = descriptions.color_targets[ iColorTarget ];
			FxColorTarget* colorTarget = clump.New< FxColorTarget >();
			colorTarget->name = colorTargetDesc.name;
			colorTarget->UpdateNameHash();
			colorTarget->sizeX = colorTargetDesc.sizeX;
			colorTarget->sizeY = colorTargetDesc.sizeY;
			colorTarget->format = colorTargetDesc.format;

			resources->color_targets.Add( colorTarget );
		}
	}

	const UINT32 numDepthTargets = descriptions.depth_targets.Num();
	if( numDepthTargets > 0 )
	{
		mxDO(resources->depth_targets.ReserveMore( numDepthTargets ));
		for( UINT32 iDepthTarget = 0; iDepthTarget < numDepthTargets; iDepthTarget++ )
		{
			const FxDepthTargetDescription& depthTargetDesc = descriptions.depth_targets[ iDepthTarget ];
			FxDepthTarget* depthTarget = clump.New< FxDepthTarget >();
			depthTarget->name = depthTargetDesc.name;
			depthTarget->UpdateNameHash();
			depthTarget->sizeX = depthTargetDesc.sizeX;
			depthTarget->sizeY = depthTargetDesc.sizeY;
			depthTarget->format = depthTargetDesc.format;
			depthTarget->sample = depthTargetDesc.sample;

			resources->depth_targets.Add( depthTarget );
		}
	}

	// render states

	const UINT numDepthStencilStates = descriptions.depth_stencil_states.Num();
	if( numDepthStencilStates > 0 )
	{
		mxDO(resources->depth_stencil_states.ReserveMore( numDepthStencilStates ));
		for( UINT iDepthStencilState = 0; iDepthStencilState < numDepthStencilStates; iDepthStencilState++ )
		{
			const DepthStencilDescription& description = descriptions.depth_stencil_states[ iDepthStencilState ];
			FxDepthStencilState* depthStencilState = clump.New< FxDepthStencilState >();
			depthStencilState->name = description.name;
			depthStencilState->UpdateNameHash();

			DepthStencilDescription* destination = static_cast< DepthStencilDescription* >( depthStencilState );
			*destination = description;

			resources->depth_stencil_states.Add(depthStencilState);
		}
	}

	const UINT numRasterizerStates = descriptions.rasterizer_states.Num();
	if( numRasterizerStates > 0 )
	{
		mxDO(resources->rasterizer_states.ReserveMore( numRasterizerStates ));
		for( UINT iRasterizerState = 0; iRasterizerState < numRasterizerStates; iRasterizerState++ )
		{
			const RasterizerDescription& description = descriptions.rasterizer_states[ iRasterizerState ];
			FxRasterizerState* rasterizerState = clump.New< FxRasterizerState >();
			rasterizerState->name = description.name;
			rasterizerState->UpdateNameHash();

			RasterizerDescription* destination = static_cast< RasterizerDescription* >( rasterizerState );
			*destination = description;

			resources->rasterizer_states.Add(rasterizerState);
		}
	}

	const UINT numSamplerStates = descriptions.sampler_states.Num();
	if( numSamplerStates > 0 )
	{
		mxDO(resources->sampler_states.ReserveMore( numSamplerStates ));
		for( UINT iSamplerState = 0; iSamplerState < numSamplerStates; iSamplerState++ )
		{
			const SamplerDescription& description = descriptions.sampler_states[ iSamplerState ];
			FxSamplerState* samplerState = clump.New< FxSamplerState >();
			samplerState->name = description.name;
			samplerState->UpdateNameHash();
			
			SamplerDescription* destination = static_cast< SamplerDescription* >( samplerState );
			*destination = description;
			
			resources->sampler_states.Add(samplerState);
		}
	}

	const UINT numBlendStates = descriptions.blend_states.Num();
	if( numBlendStates > 0 )
	{
		mxDO(resources->blend_states.ReserveMore( numBlendStates ));
		for( UINT iBlendState = 0; iBlendState < descriptions.blend_states.Num(); iBlendState++ )
		{
			const BlendDescription& description = descriptions.blend_states[ iBlendState ];
			FxBlendState* blendState = clump.New< FxBlendState >();
			blendState->name = description.name;
			blendState->UpdateNameHash();

			BlendDescription* destination = static_cast< BlendDescription* >( blendState );
			*destination = description;

			resources->blend_states.Add(blendState);
		}
	}

	const UINT numStateBlocks = descriptions.state_blocks.Num();
	if( numStateBlocks > 0 )
	{
		mxDO(resources->state_blocks.ReserveMore( numStateBlocks ));
		for( UINT iStateBlock = 0; iStateBlock < numStateBlocks; iStateBlock++ )
		{
			const FxStateBlockDescription& description = descriptions.state_blocks[ iStateBlock ];

			FxStateBlock* stateBlock = clump.New< FxStateBlock >();

			stateBlock->name = description.name;
			stateBlock->UpdateNameHash();

			const int blendStateIndex = FindIndexByName(descriptions.blend_states, description.blendState);
			if( blendStateIndex == -1 ) {
				ptERROR("State block '%s' couldn't find blend state '%s'\n", description.name.c_str(), description.blendState.ToPtr());
				return ERR_OBJECT_NOT_FOUND;
			}
			stateBlock->blendState.id = blendStateIndex;
			stateBlock->blendFactor = description.blendFactorRGBA;
			stateBlock->sampleMask = description.sampleMask;

			const int rasterizerStateIndex = FindIndexByName(descriptions.rasterizer_states, description.rasterizerState);
			if( rasterizerStateIndex == -1 ) {
				ptERROR("State block '%s' couldn't find rasterizer state '%s'\n", description.name.c_str(), description.rasterizerState.ToPtr());
				return ERR_OBJECT_NOT_FOUND;
			}
			stateBlock->rasterizerState.id = rasterizerStateIndex;

			const int depthStencilStateIndex = FindIndexByName(descriptions.depth_stencil_states, description.depthStencilState);
			if( depthStencilStateIndex == -1 ) {
				ptERROR("State block '%s' couldn't find depth-stencil state '%s'\n", description.name.c_str(), description.depthStencilState.ToPtr());
				return ERR_OBJECT_NOT_FOUND;
			}
			stateBlock->depthStencilState.id = depthStencilStateIndex;
			stateBlock->stencilRef = description.stencilRef;

			resources->state_blocks.Add(stateBlock);
		}
	}

	return ALL_OK;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
