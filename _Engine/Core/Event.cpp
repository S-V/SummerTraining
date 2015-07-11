#include <Core/Core_PCH.h>
#pragma hdrstop
#include <Core/Event.h>

mxSWIPED("this is based on Quake2/Quake3/Doom3 code");
namespace EventSystem
{
	// circular buffer of system events
	#define	EVENT_QUEUE_SIZE	512
	#define	EVENT_QUEUE_MASK	( EVENT_QUEUE_SIZE - 1 )

	SystemEvent	ringBuffer[EVENT_QUEUE_SIZE];
	int			eventHead = 0;
	int			eventTail = 0;

	void Initialize()
	{
		ptPRINT("EventSystem:Initialize(): sizeof(SystemEvent)=%d\n",sizeof(SystemEvent));
		DBGOUT("MAX_KEYS = %d\n", MAX_KEYS);
		//return ALL_OK;
	}
	void Shutdown()
	{
	}
	void PostEvent( const SystemEvent& _event )
	{
		SystemEvent * ev = &ringBuffer[ eventHead & EVENT_QUEUE_MASK ];

		if ( eventHead - eventTail >= EVENT_QUEUE_SIZE ) {
			ptWARN("PostEvent(): overflow\n");
			//// we are discarding an event, but don't leak memory
			//if ( ev->evPtr ) {
			//	mxFree( ev->evPtr );
			//}
			eventTail++;
		}

		eventHead++;

		*ev = _event;
	}
	void ClearEvents()
	{
		eventHead = eventTail = 0;
	}
	SystemEvent GetEvent()
	{
		SystemEvent	ev;

		// return if we have data
		if ( eventHead > eventTail ) {
			eventTail++;
			return ringBuffer[ ( eventTail - 1 ) & EVENT_QUEUE_MASK ];
		}

		// return the empty event 
		memset( &ev, 0, sizeof( ev ) );

		return ev;
	}

}//namespace EventSystem
