/*
=============================================================================
	File:	TextUtils.h
	Desc:
=============================================================================
*/

#ifndef __MX_TEXT_UTILS_H__
#define __MX_TEXT_UTILS_H__




// searches the specified string in the given array of strings
// and returns the index of the string
// returns INDEX_NONE if not found
UINT BinaryStringSearch(const char* stringArray[], const UINT arraySize,
					   const char* theString);


void SkipSpaces(const char *& buffer);
bool IsPathSeparator( char c );
UINT StripTrailingSlashes( char* s, const UINT len );
UINT IndexOfLastChar( const char* s, const UINT len, const char c );

int ParseInteger(const char *p);
float ParseFloat(const char* p);
double ParseDouble(const char* p);

UINT32 StringToU32( const char* s, UINT32 minValue, UINT32 maxValue );
FLOAT StringToF32( const char* s, FLOAT minValue, FLOAT maxValue );

char* ConvertBaseNToBinary32( UINT32 x, UINT base, char buf[32] );
void Print4BitsInBinary( UINT number, char _str[32] );
void Print8BitsInBinary( UINT number, char _str[32] );
void Print32BitsInBinary( UINT32 number, char _str[32], char space );


template< UINT LENGTH >
static inline
bool StringStartsWith( const char* _str, const char (&substr)[LENGTH] )
{
	return strncmp( _str, substr, LENGTH-1 ) == 0;
}



#endif // !__MX_TEXT_UTILS_H__

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
