/*
=============================================================================
	File:	Version.h
	Desc:	Version structs.
=============================================================================
*/
#pragma once

#include <Base/Object/Reflection.h>
#include <Base/Object/StructDescriptor.h>
#include <Base/Template/Containers/Array/TStaticList.h>

union Signed64_Union
{
	struct
	{
		INT32	lo;
		INT32	hi;
	};
	INT64	v;

	mxDECLARE_REFLECTION;
};
union Unsigned64_Union
{
	struct
	{
		UINT32	lo;
		UINT32	hi;
	};
	UINT64	v;

	mxDECLARE_REFLECTION;
};

/*
--------------------------------------------------------------
	STimeStamp
	used mainly for versioning
--------------------------------------------------------------
*/
union STimeStamp
{
	struct
	{
		UINT	year : 6;	// year starting from 2012
		UINT	month : 4;	// [0-11]
		UINT	day : 5;	// [0-30] (day of month)
		UINT	hour : 5;	// [0-23] (hours since midnight)
		UINT	minute : 6;	// minutes after the hour - [0,59]
		UINT	second : 6;	// seconds after the minute - [0,59]
	};
	UINT32		value;

public:
	mxDECLARE_REFLECTION;

	STimeStamp() {}
	STimeStamp( UINT year, UINT month, UINT day, UINT hour, UINT minute, UINT second );

	enum { YEAR_SHIFT = 2012 };

	void ToPtr( ANSICHAR *buffer, UINT maxChars );

	static STimeStamp Compose( UINT year, UINT month, UINT day, UINT hour, UINT minute, UINT second );

	static STimeStamp GetCurrent();
};

mxDECLARE_POD_TYPE(STimeStamp);
mxDECLARE_STRUCT(STimeStamp);

#if MX_DEVELOPER
// list of compile options (#defines)
struct DevBuildOptionsList : public TStaticList< const ANSICHAR*, 16 >
{
	void ToChars( ANSICHAR *buffer, UINT maxChars );
};
#endif // MX_DEVELOPER


// can be used for version checks, etc.
//
#pragma pack (push,1)

struct mxVersion
{
	UINT16	minor;
	UINT16	major;
	mxDECLARE_REFLECTION;
};

void mxGetCurrentEngineVersion( mxVersion &OutVersion );

struct mxVersion64
{
	UINT16	minor;
	UINT16	major;
	UINT16	patch;	// (bug correction number)
	UINT16	build;
	mxDECLARE_REFLECTION;
};

// Special flags for engine sessions.
enum ESessionFlags
{
//	SF_EditorMode = BIT(0),	// default = 0, so that package can be loaded in both engine versions
	SF_BigEndian = BIT(1),	// default = 0
};
// this structure should be as lightweight as possible
struct PtSessionInfo
{
	//UINT32 fourCC;	// 4-character identification value (FourCC).
	mxVersion version;	// engine version for release/development mode compatibility
	UINT8 flags;		// ESessionFlags
	UINT8 platform;		// EPlatformId	
	UINT8 sizeOfPtr;	// size of void pointer
	UINT8 bitsInByte;	// number of bits in a byte

public:
	mxDECLARE_REFLECTION;

	static const PtSessionInfo CURRENT;	// current platform and version

	static bool AreCompatible( const PtSessionInfo& a, const PtSessionInfo& b );

	static ERet ValidateSession( const PtSessionInfo& other );

private:
	mxSTATIC_ASSERT( sizeof(STimeStamp) == 4 );
};
#pragma pack (pop)

mxDECLARE_POD_TYPE(mxVersion);
mxDECLARE_STRUCT(mxVersion);

mxDECLARE_POD_TYPE(mxVersion64);
mxDECLARE_STRUCT(mxVersion64);

mxDECLARE_POD_TYPE(PtSessionInfo);
mxDECLARE_STRUCT(PtSessionInfo);

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
