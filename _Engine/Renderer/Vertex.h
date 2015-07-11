// vertex formats descriptions
#pragma once

#include <Graphics/Device.h>
#include <Graphics/Geometry.h>

//enum
//{
//	MAX_VERTEX_WEIGHTS = MAX_INFLUENCES
//};

enum EVertexFormat
{
	VTX_Pos4F,
	VTX_Draw,	// DrawVertex
	VTX_MAX
};

// generic vertex type, can be used for rendering both static and skinned meshes
struct DrawVertex
{
	Float3	xyz;	//12 POSITION
	Half2	st;		//4  TEXCOORD		DXGI_FORMAT_R16G16_FLOAT

	UByte4	N;		//4  NORMAL			DXGI_FORMAT_R8G8B8A8_UINT
	UByte4	T;		//4  TANGENT		DXGI_FORMAT_R8G8B8A8_UINT

	UByte4	indices;//4  BLENDINDICES	DXGI_FORMAT_R8G8B8A8_UINT
	UByte4	weights;//4  BLENDWEIGHT	DXGI_FORMAT_R8G8B8A8_UNORM
	//32 bytes
public:
	static void BuildVertexDescription( VertexDescription & _description );
};

struct P3f_TEX2f
{
	Float3	xyz;	// POSITION
};
struct P3f
{
	Float3	xyz;	// POSITION
	Float2	uv;		// TEXCOORD
};
struct P3f_TEX2f_N4Ub_T4Ub
{
	Float3	xyz;	//12 POSITION
	Float2	uv;		//8 TEXCOORD
	UByte4	N;		//4 NORMAL
	UByte4	T;		//4 TANGENT
};
struct P3f_TEX2s_N4Ub_T4Ub
{
	Float3	xyz;	//12 POSITION
	INT16	uv[2];	//4 TEXCOORD
	UByte4	N;		//4 NORMAL
	UByte4	T;		//4 TANGENT
};
struct P3f_TEX2f_COL4Ub
{
	Float3	xyz;	//12 POSITION
	Float2	uv;		//8 TEXCOORD
	UByte4	rgba;	//4 COLOR
};

#if 0
// NOTE: for storing position could use Half4 (DXGI_FORMAT_R16G16B16A16_SNORM, 8 bytes)

struct VTX_STATIC
{
	// stream 0 - 16 bytes
	Float3	pos;	//12 DXGI_FORMAT_R16G16B16A16_SNORM
	Half2	uv;		//4 DXGI_FORMAT_R16G16_SNORM
	// stream 1 - 8 bytes
	UByte4	N;		//4 DXGI_FORMAT_R8G8B8A8_UNORM
	UByte4	T;		//4 DXGI_FORMAT_R8G8B8A8_UNORM
};

struct VTX_SKINNED
{
	Float3	pos;	//12 POSITION
	Half2	uv;		//4  TEXCOORD		DXGI_FORMAT_R16G16_FLOAT

	UByte4	N;		//4  NORMAL			DXGI_FORMAT_R8G8B8A8_UINT
	UByte4	T;		//4  TANGENT		DXGI_FORMAT_R8G8B8A8_UINT

	UByte4	indices;//4  BLENDINDICES	DXGI_FORMAT_R8G8B8A8_UINT
	UByte4	weights;//4  BLENDWEIGHT	DXGI_FORMAT_R8G8B8A8_UNORM
	//32 bytes
};
#endif

#if 0
extern HInputLayout gs_inputLayouts[VertexType::Count];

void CreateVertexFormats();
void DeleteVertexFormats();
#endif
//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
