/*
=============================================================================
	File:	Version.cpp
	Desc:	Version structs.
=============================================================================
*/

#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>
#include <Base/Util/Version.h>


/*
--------------------------------------------------------------
	Signed64_Union
--------------------------------------------------------------
*/
mxBEGIN_REFLECTION(Signed64_Union)
	mxMEMBER_FIELD( lo ),
	mxMEMBER_FIELD( hi ),
mxEND_REFLECTION

/*
--------------------------------------------------------------
	Unsigned64_Union
--------------------------------------------------------------
*/
mxBEGIN_REFLECTION(Unsigned64_Union)
	mxMEMBER_FIELD( lo ),
	mxMEMBER_FIELD( hi ),
mxEND_REFLECTION

/*
--------------------------------------------------------------
	mxVersion
--------------------------------------------------------------
*/
mxREFLECT_STRUCT_VIA_STATIC_METHOD(mxVersion)
mxBEGIN_REFLECTION(mxVersion)
	mxMEMBER_FIELD( minor ),
	mxMEMBER_FIELD( major ),
mxEND_REFLECTION


void mxGetCurrentEngineVersion( mxVersion &OutVersion )
{
	OutVersion.minor = mxENGINE_VERSION_MINOR;
	OutVersion.major = mxENGINE_VERSION_MAJOR;
}

/*
-----------------------------------------------------------------------------
	mxVersion64
-----------------------------------------------------------------------------
*/
mxREFLECT_STRUCT_VIA_STATIC_METHOD(mxVersion64)
mxBEGIN_REFLECTION(mxVersion64)
	mxMEMBER_FIELD( minor ),
	mxMEMBER_FIELD( major ),
	mxMEMBER_FIELD( patch ),
	mxMEMBER_FIELD( build ),
mxEND_REFLECTION

/*
--------------------------------------------------------------
	STimeStamp
--------------------------------------------------------------
*/
mxREFLECT_STRUCT_VIA_STATIC_METHOD(STimeStamp)
mxBEGIN_REFLECTION(STimeStamp)
	mxMEMBER_FIELD( value ),
mxEND_REFLECTION

STimeStamp::STimeStamp( UINT year_, UINT month_, UINT day_, UINT hour_, UINT minute_, UINT second_ )
{
	mxASSERT( year_ < 2030 );
	mxASSERT( month_ <= 12 );
	mxASSERT( day_ <= 31 );
	mxASSERT( hour_ < 24 );
	mxASSERT( minute_ < 60 );
	mxASSERT( second_ < 60 );

	this->year		= year_ - YEAR_SHIFT;	// [0..2012+64) -> [0..63]
	this->month		= month_ - 1;	// [1..12] -> [0..11]
	this->day		= day_ - 1;	// [1..31] -> [0..30]
	this->hour		= hour_;
	this->minute	= minute_;
	this->second	= second_;
}

void STimeStamp::ToPtr( ANSICHAR *buffer, UINT maxChars )
{
	const UINT realYear		= year + YEAR_SHIFT;
	const UINT realMonth	= month + 1;
	const UINT realDay		= day + 1;
	const UINT realHour		= hour;
	const UINT realMinute	= minute;
	const UINT realSecond	= second;

	sprintf_s( buffer, maxChars,
		"mxTIMESTAMP_YMD_HMS( %u,%u,%u,  %u,%u,%u )",
		realYear, realMonth, realDay,	realHour, realMinute, realSecond );
}

/*static*/ STimeStamp STimeStamp::Compose( UINT year, UINT month, UINT day, UINT hour, UINT minute, UINT second )
{
	return STimeStamp( year, month, day, hour, minute, second );
}

/*static*/ STimeStamp STimeStamp::GetCurrent()
{
	const CalendarTime currentTime = 
		//CalendarTime::GetCurrentSystemTime()
		CalendarTime::GetCurrentLocalTime()
		;
	return STimeStamp::Compose( currentTime.year, currentTime.month, currentTime.day,
		currentTime.hour, currentTime.minute, currentTime.second );
}


#if MX_DEVELOPER
/*
--------------------------------------------------------------
	BuildOptionsList
--------------------------------------------------------------
*/
void DevBuildOptionsList::ToChars( ANSICHAR *buffer, UINT maxChars )
{
	buffer[0] = nil;

	const UINT num = this->Num();

	for( UINT i=0; i < num; i++ )
	{
		ANSICHAR	temp[64];
		sprintf_s(
			temp, mxCOUNT_OF(temp), "%s%s",
			(*this)[i], (i != num-1)
				?
				"\n" : ""//", " : ""
			);

		strcat( buffer, temp );
	}
}
#endif // MX_DEVELOPER

/*
-----------------------------------------------------------------------------
	PtSessionInfo
-----------------------------------------------------------------------------
*/
mxREFLECT_STRUCT_VIA_STATIC_METHOD(PtSessionInfo)
mxBEGIN_REFLECTION(PtSessionInfo)
	//mxMEMBER_FIELD( fourCC ),
	mxMEMBER_FIELD( version ),
	mxMEMBER_FIELD( flags ),
	mxMEMBER_FIELD( platform ),	
	mxMEMBER_FIELD( sizeOfPtr ),
	mxMEMBER_FIELD( bitsInByte ),	
mxEND_REFLECTION

const PtSessionInfo PtSessionInfo::CURRENT =
{
	{ mxENGINE_VERSION_MINOR, mxENGINE_VERSION_MAJOR },
	0,
	mxPLATFORM,
	sizeof(void*),
	BITS_IN_BYTE,
};

bool PtSessionInfo::AreCompatible( const PtSessionInfo& a, const PtSessionInfo& b )
{
	return 1
		//&& a.fourCC == b.fourCC
		&& a.version.minor == b.version.minor && a.version.major == b.version.major
		&& a.platform == b.platform
		&& a.flags == b.flags
		&& a.sizeOfPtr == b.sizeOfPtr
		;
}

ERet PtSessionInfo::ValidateSession( const PtSessionInfo& other )
{
	if( !AreCompatible(CURRENT,other) )
	{
		return ERR_INCOMPATIBLE_VERSION;
	}
	return ALL_OK;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
