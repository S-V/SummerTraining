/*
=============================================================================
	File:	Memory.cpp
	Desc:	Memory management.
	Note:	all functions have been made to allocate 16-byte aligned memory blocks.
	ToDo:	use several memory heaps
	(on Windows use heaps with low fragmentation feature turned on)
=============================================================================
*/

#include <Base/Base_PCH.h>
#pragma hdrstop
#include <Base/Base.h>

#if MX_DEBUG_MEMORY
	// Visual Leak Detector (VLD) Version 2.3.0
	//#include "vld.h"
#endif // MX_DEBUG_MEMORY

#include <Base/Util/LogUtil.h>
#include "Memory_Private.h"

#if MX_OVERRIDE_GLOBAL_NEWDELETE && MX_USE_SSE

	mxMSG(Global 'new' and 'delete' should return 16-byte aligned memory blocks for SIMD.)

#endif //!MX_OVERRIDE_GLOBAL_NEWDELETE

//-------------------------------------------------------------------------

size_t F_GetMaxAllowedAllocationSize()
{
	//return 128*1024*1024;	//128 Mb
	return 32*mxMEGABYTE;
}

char* F_SysAlloc( size_t numBytes )
{
	mxASSERT(numBytes <= F_GetMaxAllowedAllocationSize());

#if 0
	void* pNewMem = ::_aligned_malloc( numBytes, EFFICIENT_ALIGNMENT );
#else
	const size_t expandedSize = MemAlignUtil::GetAlignedMemSize( numBytes );
	void* pNewMem = new char [expandedSize];
	pNewMem = MemAlignUtil::GetAlignedPtr( pNewMem );
#endif

	mxASSERT_PTR(pNewMem);

	if( nil == pNewMem )
	{
		//(*g_onMallocFailed)( numBytes );
		ptERROR("SysAlloc() failed (%ul bytes)\n", (ULONG)numBytes);
	}

	return (char*) pNewMem;
}

void F_SysFree( const void* pointer )
{
	// deleting null pointer is valid in ANSI C++
	if( !pointer ) {
		return;
	}

#if 0
	mxASSERT(IS_ALIGNED_BY(pointer,EFFICIENT_ALIGNMENT));
	::_aligned_free( (void*)pointer );
#else
	void* rawPtr = MemAlignUtil::GetUnalignedPtr( (void*)pointer );
	delete [] rawPtr;
#endif
}

size_t F_SysSizeOfMemoryBlock( const void* pointer )
{
	if( pointer == nil )
	{
		return 0;
	}
#if 0
	return ::_aligned_msize( (void*)pointer, EFFICIENT_ALIGNMENT, 0 /*offset*/ );
#else
	return 0;
#endif
}

void F_MemDbg_ValidateHeap()
{
	//_CrtCheckMemory();
}

/*================================
		mxMemoryStatistics
================================*/

static void WriteMemHeapStats( const mxMemoryStatistics& stats, ALog& log )
{
	log.PrintF( LL_Info,
		"\nAllocated now: %u bytes, allocated in total: %u bytes, total allocations: %u"
		"\nDeallocated in total: %u bytes, total deallocations: %u"
		"\nPeak memory usage: %u bytes"
		,stats.bytesAllocated ,stats.totalAllocated ,stats.totalNbAllocations
		,stats.totalFreed ,stats.totalNbDeallocations
		,stats.peakMemoryUsage
		);
}

void mxMemoryStatistics::Dump( ALog& log )
{
	WriteMemHeapStats( *this, log );
}

/*
================================
	F_GetGlobalMemoryStats
================================
*/
void F_GetGlobalMemoryStats( mxMemoryStatistics &outStats )
{
	mxZERO_OUT(outStats);
}

/*
================================
	F_DumpGlobalMemoryStats
================================
*/
void F_DumpGlobalMemoryStats( ALog& log )
{
	//CalendarTime	localTime( CalendarTime::GetCurrentLocalTime() );
	//LocalString	timeOfDay;
	//GetTimeOfDayString( timeOfDay, localTime.hour, localTime.minute, localTime.second );

	//log.Logf( LL_Info, "\n\n[%s] Memory heap stats:\n", timeOfDay.ToPtr() );

	//mxMemoryStatistics	totalStats;

	//for( UINT iMemHeap = 0; iMemHeap < EMemHeap::HeapCount; iMemHeap++ )
	//{
	//	const mxMemoryStatistics& heapStats = g_memStats[ iMemHeap ];

	//	log.Logf( LL_Info, "\n\n--- [%u] Memory heap: '%s' ----------", iMemHeap, mxGetMemoryHeapName( (EMemHeap)iMemHeap ) );
	//	WriteMemHeapStats( heapStats, log );

	//	totalStats.bytesAllocated += heapStats.bytesAllocated;
	//	totalStats.totalAllocated += heapStats.totalAllocated;
	//	totalStats.totalNbAllocations += heapStats.totalNbAllocations;
	//	totalStats.peakMemoryUsage += heapStats.peakMemoryUsage;

	//	totalStats.totalNbReallocations += heapStats.totalNbReallocations;

	//	totalStats.totalFreed += heapStats.totalFreed;
	//	totalStats.totalNbDeallocations += heapStats.totalNbDeallocations;
	//}

	//log.Logf( LL_Info, "\n--- Total ----------\n" );
	//WriteMemHeapStats( totalStats, log );
}

//-------------------------------------------------------------------------

void F_SetupMemorySubsystem()
{
}

void F_ShutdownMemorySubsystem()
{
}

// To see if a pointer is bad for reading, just read it (and similarly writing).
// If the pointer is bad, the read (or write) will raise an exception,
// and then you can investigate the bad pointer at the point it is found.
// We read from the memory by comparing it to itself and write to the memory by copying it to itself.
// These have no effect but they do force the memory to be read or written. 
// http://blogs.msdn.com/b/oldnewthing/archive/2007/06/25/3507294.aspx
inline BOOL IsBadReadPtr2(CONST VOID *p, UINT_PTR cb)
{
  memcmp(p, p, cb);
  return FALSE;
}

inline BOOL IsBadWritePtr2(LPVOID p, UINT_PTR cb)
{
  memmove(p, p, cb);
  return FALSE;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
