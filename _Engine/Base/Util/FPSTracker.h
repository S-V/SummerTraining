/*
=============================================================================
	File:	FPSTracker.h
	Desc:	
=============================================================================
*/

#ifndef __MX_FPS_TRACKER_H__
#define __MX_FPS_TRACKER_H__



//--------------------------------------------------------------//

template< UINT NUM_SAMPLES = 64 >
class FPSTracker
{
	FLOAT	m_samples[ NUM_SAMPLES ];	// time deltas for smoothing
	UINT	m_currentSampleIndex;

public:
	FPSTracker()
	{
		this->Reset();
	}
	void Reset()
	{
		mxZERO_OUT(m_samples);
		m_currentSampleIndex = 0;
	}
	// Call this function once per frame.
	void Update( FLOAT deltaSeconds )
	{
		//mxASSERT(deltaSeconds > 0.0f);
		if( deltaSeconds == 0.0f ) {
			deltaSeconds = 9999.0f;
		}

		m_samples[ m_currentSampleIndex ] = deltaSeconds;
		m_currentSampleIndex = (m_currentSampleIndex + 1) % NUM_SAMPLES;
	}
	FLOAT CalculateFPS() const
	{
		FLOAT averageDelta = 0;
		for(UINT i = 0; i < NUM_SAMPLES; ++i) {
			averageDelta += m_samples[i];
		}
		averageDelta /= NUM_SAMPLES;
		mxASSERT(averageDelta > 0.0f);
		return floor( (1.0f / averageDelta) + 0.5f );
	}
};

#if 0
template< UINT NUM_SAMPLES = 128 >
struct MeasureFps
{
	FLOAT	m_samples[ NUM_SAMPLES ];	// samples for calculating simple moving average (SMA)
	UINT	m_currentSampleIndex;

public:
	MeasureFps()
	{
		this->Reset();
	}
	void Reset()
	{
		mxZERO_OUT(m_samples);
		m_currentSampleIndex = 0;
	}
	FLOAT GetAverage()
	{
		//http://stackoverflow.com/questions/87304/calculating-frames-per-second-in-a-game

		/* need to zero out the ticklist array before starting */
/* average will ramp up until the buffer is full */
/* returns average ticks per frame over the MAXSAMPPLES last frames */

    ticksum-=ticklist[tickindex];  /* subtract value falling off */
    ticksum+=newtick;              /* add new value */
    ticklist[tickindex]=newtick;   /* save new value so it can be subtracted later */
    if(++tickindex==MAXSAMPLES)    /* inc buffer index */
        tickindex=0;

    /* return average */
    return((double)ticksum/MAXSAMPLES);
	}
};
#endif

#endif // !__MX_FPS_TRACKER_H__

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
