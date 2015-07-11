/*
=============================================================================
	File:	ScopedTimer.h
	Desc:	
=============================================================================
*/
#pragma once

class ScopedTimer
{
	UINT64	m_startTimeMicroseconds;
	const char *	m_name;
public:
	ScopedTimer( const char* _name = NULL )
	{
		m_startTimeMicroseconds = mxGetTimeInMicroseconds();
		m_name = _name;
	}
	~ScopedTimer()
	{
		if( m_name )
		{
			UINT64 elapsedTimeMcSec = mxGetTimeInMicroseconds() - m_startTimeMicroseconds;
			DBGOUT( "%s took %u milliseconds\n", m_name, elapsedTimeMcSec/1000 );
		}
	}
	UINT64 ElapsedMicroseconds() const
	{
		return mxGetTimeInMicroseconds() - m_startTimeMicroseconds;
	}
	UINT64 ElapsedMilliseconds() const
	{
		return this->ElapsedMicroseconds() / 1000;
	}
	UINT ElapsedSeconds() const
	{
		return this->ElapsedMilliseconds() / 1000;
	}
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
