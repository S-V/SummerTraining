// Auxiliary renderer - immediate-mode emulation.
// Used mostly for debugging and editor purposes.
#pragma once

#include <Base/Util/Color.h>
#include <Graphics/Device.h>
#include <Graphics/Effects.h>

// Vertex type used for auxiliary rendering (editor/debug visualization/etc)
#pragma pack (push,1)
struct AuxVertex
{
	Float3	xyz;	//12 POSITION
	Float2	uv;		//8 TEXCOORD
	UByte4	N;		//4 NORMAL
	UByte4	T;		//4 TANGENT
	UByte4	rgba;	//4 COLOR
public:
	static void BuildVertexDescription( VertexDescription & _description );
};
#pragma pack (pop)

struct AuxRenderer
{
	virtual void DrawLine3D(
		const AuxVertex& start, const AuxVertex& end
		) = 0;

	virtual void DrawWireQuad3D(
		const AuxVertex& a, const AuxVertex& b, const AuxVertex& c, const AuxVertex& d
		) = 0;

	virtual void DrawSolidQuad3D(
		const AuxVertex& a, const AuxVertex& b, const AuxVertex& c, const AuxVertex& d
		) = 0;

	// DrawPrimitiveUP
	virtual void Draw(
		const AuxVertex* vertices, UINT32 numVertices,
		const UINT16* indices, UINT32 numIndices,
		const Topology::Enum topology
		) = 0;

public:

	void DrawLine(
		const Float3& start,
		const Float3& end,
		const RGBAf& startColor = RGBAf::WHITE,
		const RGBAf& endColor = RGBAf::WHITE
	);

	void DrawWireTriangle(
		const Float3& a, const Float3& b, const Float3& c,
		const RGBAf& color = RGBAf::WHITE
	);

	// wireframe axis-aligned bounding box
	void DrawAABB(
		const Float3& aabbMin, const Float3& aabbMax,
		const RGBAf& color = RGBAf::WHITE
		);

	virtual ~AuxRenderer() {}
};

/*
--------------------------------------------------------------
	BatchRenderer

	primitive-batching immediate mode renderer;
	designed for convenience, not for speed;
	is meant to be used for debug visualization, etc.
--------------------------------------------------------------
*/
class BatchRenderer : public AuxRenderer, SingleInstance< BatchRenderer > {
public:
	BatchRenderer();
	~BatchRenderer();

	ERet Initialize( UINT32 vertexBufferSize = mxMiB(4), UINT32 indexBufferSize = mxMiB(2) );
	void Shutdown();

	// Fixed-function pipeline emulation

	//void SetShader( FxShader* technique );
	//void SetTransform( const Float4x4& mWVP );
	//void SetColor( const float* rgba );

	void SetShader( FxShader* technique );

	// don't forget to call this function
	// before significant state changes and at the end of each frame
	// to flush any batched primitives and prevent flickering

	void Flush();

	//=== Wireframe mode

	virtual void DrawLine3D(
		const AuxVertex& start, const AuxVertex& end
	) override;

	//void DrawTriangle3D(
	//	const AuxVertex& a, const AuxVertex& b, const AuxVertex& c,
	//	const RGBAf& color = RGBAf::WHITE
	//);
	void DrawDashedLine(
		const Float3& start,
		const Float3& end,
		const float dashSize,
		const RGBAf& startColor = RGBAf::WHITE,
		const RGBAf& endColor = RGBAf::WHITE
	);
	void DrawAxes( float length = 1.0f );

	//void DrawWireBox(
	//	const Float3& center,
	//	const float sideLength,
	//	const RGBAf& color = RGBAf::WHITE
	//);

	// 'wireframe' arrow
	void DrawArrow(
		const Float4x4& arrowTransform,
		const RGBAf& color,
		float arrowLength = 1.f,
		float headSize = 0.1f
	);

	void DrawGrid(
		const Float3& origin,	// center of the grid
		const Float3& axisX,	// length = half size along X axis
		const Float3& axisY,	// length = half size along Y axis
		int numXDivisions,
		int numYDivisions,
		const RGBAf& color
	);
	//void DrawCube( const Float4x4& worldMatrix, const RGBAf& color );
	//void DrawOBB( const rxOOBB& box, const RGBAf& color );
	//void DrawFrustum( const XNA::Frustum& frustum, const RGBAf& color );
	//void DrawViewFrustum( const ViewFrustum& frustum, const RGBAf& color );

	void DrawCircle(
		const Float3& origin,
		const Float3& right,
		const Float3& up,
		const RGBAf& color,
		float radius,
		int numSides = 16
	);

	void DrawWireFrustum(
		const Float4x4& _viewProjection,
		const RGBAf& _color = RGBAf::WHITE
	);

	virtual void DrawWireQuad3D(
		const AuxVertex& a, const AuxVertex& b, const AuxVertex& c, const AuxVertex& d
	) override;

	//=== Solid mode

	void DrawSolidTriangle3D(
		const AuxVertex& a, const AuxVertex& b, const AuxVertex& c
	);

	virtual void DrawSolidQuad3D(
		const AuxVertex& a, const AuxVertex& b, const AuxVertex& c, const AuxVertex& d
	) override;

	void DrawSolidBox(
		const Float3& origin, const Float3& halfSize,
		const RGBAf& color = RGBAf::WHITE
	);

	void DrawSolidSphere(
		const Float3& center, float radius,
		const RGBAf& color = RGBAf::WHITE,
		int numStacks = 16, int numSlices = 16
	);

	//void DrawSolidAABB( const rxAABB& box, const RGBAf& color = RGBAf::WHITE );

	mxUNDONE
#if 0
	void DrawRing( const XMFLOAT3& Origin, const XMFLOAT3& MajorAxis, const XMFLOAT3& MinorAxis, const RGBAf& Color );
	void DrawSphere( const XNA::Sphere& sphere, const RGBAf& Color );
	void DrawRay( const XMFLOAT3& Origin, const XMFLOAT3& Direction, BOOL bNormalize, const RGBAf& Color );
#endif

	// accepts sprite's coordinates in world space
	void DrawSprite(
		const Float4x4& cameraWorldMatrix,
		const Float3& spriteOrigin,
		const float spriteSizeX = 1.0f, const float spriteSizeY = 1.0f,
		const RGBAf& color = RGBAf::WHITE
	);

	void DrawPoint( const AuxVertex& _p );

	virtual void Draw(
		const AuxVertex* vertices, UINT32 numVertices,
		const UINT16* indices, UINT32 numIndices,
		const Topology::Enum topology
		) override;

private:
	template< typename VERTEX_TYPE, typename INDEX_TYPE >
	UINT32 BeginBatch( TopologyT topology, UINT32 numVertices, VERTEX_TYPE *& vertices, UINT32 numIndices, INDEX_TYPE *& indices )
	{
		const UINT32 vertexDataSize = numVertices * sizeof(vertices[0]);
		const UINT32 indexDataSize = numIndices * sizeof(indices[0]);
		mxASSERT(vertexDataSize <= m_VBSize);
		mxASSERT(indexDataSize <= m_IBSize);

		const UINT32 oldVBSize = m_batchedVertices.Num();
		const UINT32 oldIBSize = m_batchedIndices.Num();
		const UINT32 newVBSize = oldVBSize + vertexDataSize;
		const UINT32 newIBSize = oldIBSize + indexDataSize;

		bool bNeedToFlush = false;

		bNeedToFlush |= (m_vertexStride != sizeof(vertices[0]));
		bNeedToFlush |= (m_indexStride != sizeof(indices[0]));
		bNeedToFlush |= (newVBSize > m_VBSize);
		bNeedToFlush |= (newIBSize > m_IBSize);
		bNeedToFlush |= (m_topology != topology);

		if( bNeedToFlush ) {
			this->Flush();
		}

		m_vertexStride = sizeof(vertices[0]);
		m_indexStride = sizeof(indices[0]);
		m_topology = topology;

		const UINT32 baseVertexIndex = m_batchedVertices.Num() / sizeof(vertices[0]);
		vertices = (VERTEX_TYPE*) m_batchedVertices.AddManyUninitialized(vertexDataSize);
		indices = (INDEX_TYPE*) m_batchedIndices.AddManyUninitialized(indexDataSize);
		return baseVertexIndex;
	}

public_internal:
	TArray< BYTE >		m_batchedVertices;
	TArray< BYTE >		m_batchedIndices;

	UINT32				m_VBSize;
	UINT32				m_IBSize;
	HBuffer				m_dynamicVB;
	HBuffer				m_dynamicIB;
	HInputLayout		m_layout;
	UINT8				m_vertexStride;
	UINT8				m_indexStride;

	TopologyT			m_topology;

	TPtr< FxShader >	m_technique;

	//Float4x4				m_transform;
};

/*
=======================================================================
	DEBUG FONT
=======================================================================
*/
mxSWIPED("DirectXTK - the DirectX Tool Kit");
namespace Fonts
{
#pragma pack(push,1)
	struct FileHeader
	{
		UINT32		magic;
		FLOAT32		lineSpacing;	// vertical spacing
		UINT32		defaultChar;
		UINT32		numGlyphs;
		// followed by:
		// FontGlyph glyphs[ numGlyphs ];
		// BitmapHeader;
		// texture data.
	};
	struct SubRect
	{
		UINT32		left;
		UINT32		top;
		UINT32		right;
		UINT32		bottom;
	};
	struct Glyph
	{
		UINT32		Character;  // unicode character code
		SubRect		Subrect;    // texture rect, in texels
		FLOAT32		XOffset;    // dunno, something in texels
		FLOAT32		YOffset;    // small letters are shifted down by this value (in texels)
		FLOAT32		XAdvance;   // dunno, something in texels
	};
	struct BitmapHeader
	{
		UINT32		width;	// in texels
		UINT32		height;	// in texels
		UINT32		format;	// DXGI_FORMAT
		UINT32		pitch;
		UINT32		rows;
		// followed by:
		// BYTE imageData[ pitch * rows ];
	};
#pragma pack(pop)

	struct SpriteFont
	{
		TBuffer< Glyph >	m_glyphs;
		const Glyph *		m_default;
		float				m_lineSpacing;    // vertical spacing
		HTexture			m_textureAtlas;
		HResource			m_resourceHandle;
		float				m_invTextureWidth;
		float				m_invTextureHeight;
	public:
		SpriteFont();
		~SpriteFont();

		ERet Load( AStreamReader& stream );
		void Shutdown();

		const Glyph* FindGlyph( UNICODECHAR character ) const;
		bool ContainsCharacter( UNICODECHAR character ) const;
		void SetDefaultCharacter( UNICODECHAR character );

		float GetLineSpacing() const;
		void MeasureString( _In_z_ wchar_t const* text, float *width, float *height );

		template< typename WHAT >    // where WHAT has operator () ( const Glyph& glyph, float x, float y, UINT32 index, void* userData )
		void TForEachGlyph( _In_z_ wchar_t const* text, WHAT action, void* userData ) const;

		typedef void GlyphCallback( const Glyph& glyph, float x, float y, UINT32 index, void* userData );
		void ForEachGlyph( _In_z_ wchar_t const* text, GlyphCallback* callback, void* userData ) const;
	};

	class FontRenderer
	{
		HBuffer			m_vertexBuffer;
		HInputLayout	m_vertexLayout;
	public:
		FontRenderer();

		ERet Initialize();
		void Shutdown();

		// x and y are in viewport coordinates of the text sprite's top left corner
		ERet RenderText(
			HContext _context, llgl::DrawCall& batch,
			UINT16 screenWidth, UINT16 screenHeight,
			const SpriteFont* font,
			UINT16 x, UINT16 y,
			const char* text, UINT16 length = 0
			);
	};

}// namespace Fonts


// this can be used for drawing translation gizmo arrows / coordinate frames
struct AxisArrowGeometry
{
	enum { AXIS_ARROW_SEGMENTS = 6 };

	Float3	m_axisRoot;
	Float3	m_arrowBase;
	Float3	m_arrowHead;
	Float3	m_segments[ AXIS_ARROW_SEGMENTS + 1 ];

public:
	AxisArrowGeometry();

	void BuildGeometry();

	void Draw( BatchRenderer& renderer, const RGBAf& color ) const;
	void Draw( BatchRenderer& renderer, const Float4x4& transform, const RGBAf& color ) const;
};

// Scaling so gizmo stays same size no matter what perspective/fov/distance
//
static inline float GetGizmoScale( const Float3& eyePos, const Float3& objPos )
{
	// Scale the gizmo relative to the distance.
	float distance = Float3_Length(eyePos - objPos);
	float scaleFactor = distance;
	return scaleFactor;
}
void DrawGizmo( const AxisArrowGeometry& gizmo, const Float4x4& localToWorld, const Float3& cameraPosition, BatchRenderer & renderer );

	struct TEMP_TriangleMesh
	{
		TArray< AuxVertex >	vertices;
		TArray< UINT16 >	indices;
	};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
