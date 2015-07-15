#include <Base/Base.h>
#include <Core/Core.h>

#include <bgfx.h>

struct CSG_VERTEX
{
	Float3 xyz;
	UINT32 N;
	UINT32 T;
	INT16 U;
	INT16 V;
	float c1;
	float c2;
	//!32

	static void init()
	{
		ms_decl
			.begin()
			.add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Normal,    4, bgfx::AttribType::Uint8, true, true)
			.add(bgfx::Attrib::Tangent,   4, bgfx::AttribType::Uint8, true, true)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Int16, true, true)
			.add(bgfx::Attrib::Color0,    1, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color1,    1, bgfx::AttribType::Float)
			.end();
	}

	static bgfx::VertexDecl ms_decl;
};
