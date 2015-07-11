//@todo: reduce vertex size; push more work to the vertex shader;
#include "Renderer/Renderer_PCH.h"
#pragma hdrstop
#include <algorithm>
#include <Graphics/Effects.h>
#include <Renderer/Renderer.h>
#include <Renderer/DebugHUD.h>

namespace DebugHUD
{

namespace
{
	// each single character corresponds to exactly one vertex:
	// we do point sprite => quad expansion in geometry shader.
	struct TextVertex
	{
		Float4	xy_wh;
		Float4	tl_br;	// UV texture coords for top left and bottom right corners
	};

	struct TextBatch
	{
		utl::SpriteFont *	font;
		UINT16				x;
		UINT16				y;
		UINT16				text;	// offset of character data in the text buffer
		UINT16				length;
	};

	enum { MAX_TEXT_BATCHES = 512 };
	enum { TEXT_BUFFER_SIZE = 8096 };
	enum { MAX_NUM_VERTICES = 1024 };

	struct PrivateData
	{
		TArray< TextVertex >	batchedVertices;

		HBuffer	dynamicVB;
		HInputLayout	vertexLayout;

		TPtr< FxShader >	technique;

		float		invScreenWidth;
		float		invScreenHeight;

		UINT16		numBatches;
		UINT16		charsWriten;
		TextBatch	batches[ MAX_TEXT_BATCHES ];
		char		textBuffer[ TEXT_BUFFER_SIZE ];
	};

	mxDECLARE_PRIVATE_DATA( PrivateData, gPrivateData );
};

//!=- MACRO -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#define me	mxGET_PRIVATE_DATA( PrivateData, gPrivateData )

//---------------------------------------------------------------------------

ERet Initialize( FxLibrary* library )
{
	mxINITIALIZE_PRIVATE_DATA( gPrivateData );

	me.batchedVertices.Reserve( MAX_NUM_VERTICES );

	VertexDescription	vertexDesc;
	{
		vertexDesc.Begin();
		vertexDesc.Add(AttributeType::Float, 4, VertexAttribute::Position, false, 0);
		vertexDesc.Add(AttributeType::Float, 4, VertexAttribute::TexCoord0, false, 0);
		vertexDesc.End();
	}
	me.vertexLayout = llgl::CreateInputLayout( vertexDesc, "TextVertex" );

	me.dynamicVB = llgl::CreateBuffer( nil, MAX_NUM_VERTICES * sizeof(TextVertex), true );

	me.technique = library->FindTechniqueByHash(mxHASH_ID(DebugText));
	chkRET_X_IF_NIL(me.technique, ERR_OBJECT_NOT_FOUND);

	me.invScreenWidth = 0.0f;
	me.invScreenHeight = 0.0f;

	me.numBatches = 0;
	me.charsWriten = 0;

	return IM_OK;
}

void Shutdown()
{
	me.technique = nil;

	llgl::DeleteBuffer( me.dynamicVB );
	me.dynamicVB.SetNil();

	llgl::DeleteInputLayout(me.vertexLayout);
	me.vertexLayout.SetNil();

	me.batchedVertices.Empty();

	mxSHUTDOWN_PRIVATE_DATA( gPrivateData );
}

ERet SubmitText( utl::SpriteFont* font, UINT16 x, UINT16 y, const char* text, UINT16 length )
{
	if( me.numBatches > MAX_TEXT_BATCHES ) {
		return ERR_BUFFER_TOO_SMALL;
	}
	if( me.charsWriten + length > sizeof(me.textBuffer) ) {
		return ERR_BUFFER_TOO_SMALL;
	}

	const UINT textOffset = me.charsWriten;
	memcpy( &me.textBuffer[textOffset], text, length );
	me.charsWriten += length;

	TextBatch& newBatch = me.batches[ me.numBatches++ ];
	{
		newBatch.font	= font;
		newBatch.x		= x;
		newBatch.y		= y;
		newBatch.text	= textOffset;
		newBatch.length	= length;
	}

	return IM_OK;
}

// viewport coordinates => normalized device coordinates (NDC)
template< typename POINT_SIZE >
static inline void ViewportToNDC(float invScreenWidth, float invScreenHeight,
								 POINT_SIZE screenX, POINT_SIZE screenY,
								 float &ndcX, float &ndcY)
{
	ndcX = (screenX * invScreenWidth) * 2.0f - 1.0f;	// [0..ScreenWidth] => [-1..+1]
	ndcY = (screenY * invScreenHeight) * 2.0f - 1.0f;	// [0..ScreenHeight] => [+1..-1] (flip Y axis)
	ndcY *= -1.0f;
}

static void AddCharacterVertex( const utl::Glyph& glyph, float x, float y, UINT index, void* userData )
{
	const TextBatch& batch = *(const TextBatch*) userData;
	const utl::SpriteFont* font = batch.font;
	const float invTextureWidth = font->m_invTextureWidth;
	const float invTextureHeight = font->m_invTextureHeight;

	// Calculate the X and Y pixel position on the screen to start drawing to.
	float glyphLeftX, glyphTopY;
	ViewportToNDC( me.invScreenWidth, me.invScreenHeight, batch.x, batch.y, glyphLeftX, glyphTopY );

	const UINT32 glyphWidth = glyph.Subrect.right - glyph.Subrect.left;
	const UINT32 glyphHeight = glyph.Subrect.bottom - glyph.Subrect.top;

	// this controls the size of the character sprite
	float glyphScalingX, glyphScalingY;
	glyphScalingX = me.invScreenWidth;
	glyphScalingY = me.invScreenHeight;
	glyphScalingX *= 2.0f;
	glyphScalingY *= 2.0f;


	glyphLeftX += x * glyphScalingX;
	glyphTopY -= (y + glyph.YOffset) * glyphScalingY;


	float glyphSizeX = (float)glyphWidth * glyphScalingX;
	float glyphSizeY = (float)glyphHeight * glyphScalingY;

	TextVertex& vertex = me.batchedVertices.Add();

	vertex.xy_wh = XMVectorSet(
		glyphLeftX,
		glyphTopY,
		glyphSizeX,
		glyphSizeY
		);

	vertex.tl_br = XMVectorSet(
		glyph.Subrect.left * invTextureWidth,
		glyph.Subrect.top * invTextureHeight,
		glyph.Subrect.right * invTextureWidth,
		glyph.Subrect.bottom * invTextureHeight
		);
}

static void AddSpriteVertices( const TextBatch& batch )
{
	const char* text = &me.textBuffer[ batch.text ];
	const UINT length = batch.length;

	ScopedStackAlloc	tempAlloc( gCore.frameAlloc );
	wchar_t * wideCharData = tempAlloc.AllocMany< wchar_t >( length + 1 );
	mbstowcs( wideCharData, text, length );
	wideCharData[ length ] = 0;

	me.batchedVertices.Reserve( me.batchedVertices.Num() + length );

	batch.font->ForEachGlyph( wideCharData, &AddCharacterVertex, (void*)&batch );
}

static void UpdateShaderConstants( const TextBatch& batch, UINT16 viewportWidth, UINT16 viewportHeight )
{
	const float invScreenWidth = 1.0f / viewportWidth;
	const float invScreenHeight = 1.0f / viewportHeight;
	const float invTextureWidth = batch.font->m_invTextureWidth;
	const float invTextureHeight = batch.font->m_invTextureHeight;

	// xy = inverse viewport size, zy = inverse font texture size
	Float4	shaderData;
	shaderData = XMVectorSet(
		invScreenWidth,
		invScreenHeight,
		invTextureWidth,
		invTextureHeight
		);

	XConstantBuffer& cbuffer = me.technique->cbuffers[0];
	llgl::UpdateUniformBuffer(cbuffer.handle, &shaderData, sizeof(shaderData));
}

static void FlushBatchedVertices()
{
	const UINT numVertices = me.batchedVertices.Num();
	if( numVertices )
	{
		const TextVertex* vertexData = me.batchedVertices.ToPtr();
		const UINT vertexDataSize = me.batchedVertices.GetDataSize();
		mxASSERT(numVertices <= MAX_NUM_VERTICES);

		llgl::UpdateDynamicVertexBuffer( me.dynamicVB, vertexData, vertexDataSize );

		me.technique->ApplyPass(0,0);

		llgl::SetVertexBuffers( 1, &me.dynamicVB, 0, numVertices );
		llgl::SetInputLayout( me.vertexLayout );
		llgl::SetTopology( Topology::PointList );

		llgl::SubmitBatch();

		me.batchedVertices.Empty();
	}
}

void RenderBatchedItems( UINT16 viewportWidth, UINT16 viewportHeight )
{
	const UINT numBatches = me.numBatches;
	const TextBatch* batches = me.batches;

	if( numBatches )
	{
		ScopedStackAlloc	tempAlloc( gCore.frameAlloc );
		const TextBatch** sortedBatches = tempAlloc.AllocMany< const TextBatch* >( numBatches );
		for( UINT i = 0; i < numBatches; i++ ) {
			sortedBatches[i] = &batches[i];
		}

		struct CompareBatches {
			inline bool operator () ( const TextBatch* a, const TextBatch* b ) const {
				return a->font < b->font;
			}
		};
		std::stable_sort( sortedBatches, sortedBatches + numBatches, CompareBatches() );

		me.invScreenWidth = 1.0f / viewportWidth;
		me.invScreenHeight = 1.0f / viewportHeight;

		utl::SpriteFont* currentFont = nil;

		for( UINT i = 0; i < numBatches; i++ )
		{
			const TextBatch& batch = *sortedBatches[i];
			if( currentFont != batch.font )
			{
				currentFont = batch.font;
				FlushBatchedVertices();
				UpdateShaderConstants( batch, viewportWidth, viewportHeight );
				me.technique->textures[0].handle = batch.font->m_resourceHandle;
			}
			if( me.batchedVertices.Num() + batch.length >= MAX_NUM_VERTICES ) {
				FlushBatchedVertices();
			}
			AddSpriteVertices( batch );
		}
		FlushBatchedVertices();

		me.numBatches = 0;
		me.charsWriten = 0;
	}
}

}//namespace DebugHUD

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
