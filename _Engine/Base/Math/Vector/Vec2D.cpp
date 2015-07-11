/*
=============================================================================
	File:	Vec2D.cpp
	Desc:.
=============================================================================
*/

#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>

#include <Base/Math/Math.h>



mxBEGIN_STRUCT(Vec2D)
	mxMEMBER_FIELD(x),
	mxMEMBER_FIELD(y),
mxEND_REFLECTION

const Vec2D Vec2D::vec2_origin	( 0.0f, 0.0f );
const Vec2D Vec2D::vec2_unit	( 1.0f, 1.0f );

/*
=============
Lerp

Linearly inperpolates one vector to another.
=============
*/
void Vec2D::Lerp( const Vec2D &v1, const Vec2D &v2, const FLOAT l ) {
	if ( l <= 0.0f ) {
		(*this) = v1;
	} else if ( l >= 1.0f ) {
		(*this) = v2;
	} else {
		(*this) = v1 + l * ( v2 - v1 );
	}
}

/*
=============
Vec2D::Rotate
=============
*/
Vec2D & Vec2D::Rotate( FLOAT angle )
{
	FLOAT s, c;
	Float_SinCos( angle, s, c );
   
    FLOAT nx = c * x - s * y;
    FLOAT ny = s * x + c * y;
   
    this->x = nx;
    this->y = ny;
    
    return *this;
}



//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
