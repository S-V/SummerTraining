/*
=============================================================================
	File:	Memory.h
	Desc:	Memory management.
	Note:	It's far from finished.
	ToDo:	Memory limits and watchdogs, thread-safety,
			allow users to define their own OutOfMemory handlers, etc.
=============================================================================
*/
#pragma once

//---------------------------------------------------------------------------
//		Defines.
//---------------------------------------------------------------------------


// 1 - Track each memory allocation/deallocation, record memory usage statistics and detect memory leaks and so on.
#define MX_DEBUG_MEMORY			MX_DEBUG

#define MX_ENABLE_DEBUG_HEAP	(1)

// 1 - Redirect the global 'new' and 'delete' to our memory manager.
// this is dangerous, it may be better to define them on per-type basis or use special macros.
#define MX_OVERRIDE_GLOBAL_NEWDELETE	(0)

// 1 - prevent the programmer from using malloc/free.
#define MX_HIDE_MALLOC_AND_FREE		(0)



//---------------------------------------------------------------------------
//		Declarations.
//---------------------------------------------------------------------------

//
//	mxMemoryStatistics - holds memory allocation information.
//
struct mxMemoryStatistics
{
	size_t	bytesAllocated;	// total size of currently allocated memory, in bytes

	size_t	totalAllocated;	// size of memory that have been allocated so far, in bytes
	size_t	totalFreed;		// size of memory that have been deallocated so far, in bytes

	size_t	peakMemoryUsage;	// in bytes

	UINT	totalNbAllocations;	// incremented whenever allocation occurs
	UINT	totalNbReallocations;// incremented whenever reallocation occurs
	UINT	totalNbDeallocations;// incremented whenever deallocation occurs

public:
	inline mxMemoryStatistics()
	{
		Reset();
	}
	inline void Reset()
	{
		bytesAllocated = 0;

		totalAllocated = 0;
		totalFreed = 0;

		peakMemoryUsage = 0;

		totalNbAllocations = 0;
		totalNbReallocations = 0;
		totalNbDeallocations = 0;
	}
	inline void UpdateOnAllocation( UINT numAllocatedBytes )
	{
		bytesAllocated += numAllocatedBytes;
		totalAllocated += numAllocatedBytes;
		totalNbAllocations++;

		peakMemoryUsage = Max( peakMemoryUsage, bytesAllocated );
	}
	inline void UpdateOnDeallocation( UINT bytesFreed )
	{
		bytesAllocated -= bytesFreed;
		totalFreed += bytesFreed;
		totalNbDeallocations++;
	}

	void Dump( ALog& log );
};

// reports memory statistics
void F_GetGlobalMemoryStats( mxMemoryStatistics &outStats );

// writes memory statistics, usage & leak info to the specified file
void F_DumpGlobalMemoryStats( ALog& log );

//
//	System memory management functions.
//
size_t F_GetMaxAllowedAllocationSize();

char* F_SysAlloc( size_t numBytes );
void F_SysFree( const void* pointer );
size_t F_SysSizeOfMemoryBlock( const void* pointer );

void F_MemDbg_ValidateHeap();	// debug mode only

//---------------------------------------------------------------------------
//		Helper macros.
//---------------------------------------------------------------------------

#define mxAlloc( numBytes )					F_SysAlloc( (numBytes) )
#define mxFree( pointer )					F_SysFree( (pointer) )

#define mxMalloc16( numBytes )	mxAlloc( ALIGN16(numBytes) )
#define mxFree16( pMemory )		mxFree( (pMemory) )

#if defined(new_one) || defined(free_one)
#	error 'new_one' and 'free_one' have already been  defined - shouldn't happen!
#endif

#define new_one( x )	new x
#define free_one( x )	delete x

// Array operators

#if defined(new_array) || defined(free_array)
#	error 'new_array' and 'free_array' have already been defined - shouldn't happen!
#endif

#define new_array( x, num )		new x [num]
#define free_array( x )			delete[] x


#if MX_OVERRIDE_GLOBAL_NEWDELETE
	#error Incompatible options: overriden global 'new' and 'delete' and per-class memory heaps.
#endif

//
//	mxDECLARE_CLASS_ALLOCATOR
//
//	'className' can be used to track instances of the class.
//
#define mxDECLARE_CLASS_ALLOCATOR( memClass, className )\
public:\
	typedef className THIS_TYPE;	\
	static EMemHeap GetHeap() { return memClass; }	\
	mxFORCEINLINE void* operator new      ( size_t sizeInBytes )	{ return mxAllocX( memClass, sizeInBytes ); }	\
	mxFORCEINLINE void  operator delete   ( void* ptr )			{ mxFreeX( memClass, ptr ); }					\
	mxFORCEINLINE void* operator new      ( size_t, void* ptr )	{ return ptr; }									\
	mxFORCEINLINE void  operator delete   ( void*, void* )		{ }												\
	mxFORCEINLINE void* operator new[]    ( size_t sizeInBytes )	{ return mxAllocX( memClass, sizeInBytes ); }	\
	mxFORCEINLINE void  operator delete[] ( void* ptr )			{ mxFreeX( memClass, ptr ); }					\
	mxFORCEINLINE void* operator new[]    ( size_t, void* ptr )	{ return ptr; }									\
	mxFORCEINLINE void  operator delete[] ( void*, void* )		{ }												\

//
//	mxDECLARE_VIRTUAL_CLASS_ALLOCATOR
//
#define mxDECLARE_VIRTUAL_CLASS_ALLOCATOR( memClass, className )\
public:\
	typedef className THIS_TYPE;	\
	mxFORCEINLINE void* operator new      ( size_t sizeInBytes )	{ return mxAllocX( memClass, sizeInBytes ); }	\
	mxFORCEINLINE void  operator delete   ( void* ptr )			{ mxFreeX( memClass, ptr ); }					\
	mxFORCEINLINE void* operator new      ( size_t, void* ptr )	{ return ptr; }									\
	mxFORCEINLINE void  operator delete   ( void*, void* )		{ }												\
	mxFORCEINLINE void* operator new[]    ( size_t sizeInBytes )	{ return mxAllocX( memClass, sizeInBytes ); }	\
	mxFORCEINLINE void  operator delete[] ( void* ptr )			{ mxFreeX( memClass, ptr ); }					\
	mxFORCEINLINE void* operator new[]    ( size_t, void* ptr )	{ return ptr; }									\
	mxFORCEINLINE void  operator delete[] ( void*, void* )		{ }

//
//	mxDECLARE_NONVIRTUAL_CLASS_ALLOCATOR
//
#define mxDECLARE_NONVIRTUAL_CLASS_ALLOCATOR( memClass, className )\
public:\
	typedef className THIS_TYPE;	\
	mxFORCEINLINE void* operator new      ( size_t sizeInBytes )	{ return mxAllocX( memClass, sizeInBytes ); }	\
	mxFORCEINLINE void  operator delete   ( void* ptr )			{ mxFreeX( memClass, ptr ); }					\
	mxFORCEINLINE void* operator new      ( size_t, void* ptr )	{ return ptr; }									\
	mxFORCEINLINE void  operator delete   ( void*, void* )		{ }												\
	mxFORCEINLINE void* operator new[]    ( size_t sizeInBytes )	{ return mxAllocX( memClass, sizeInBytes ); }	\
	mxFORCEINLINE void  operator delete[] ( void* ptr )			{ mxFreeX( memClass, ptr ); }					\
	mxFORCEINLINE void* operator new[]    ( size_t, void* ptr )	{ return ptr; }									\
	mxFORCEINLINE void  operator delete[] ( void*, void* )		{ }


//---------------------------------------------------------------------------
//		Prevent usage of plain old 'malloc' & 'free' if needed.
//---------------------------------------------------------------------------

#if MX_HIDE_MALLOC_AND_FREE

	#define malloc( size )			ptBREAK
	#define free( mem )				ptBREAK
	#define calloc( num, size )		ptBREAK
	#define realloc( mem, newsize )	ptBREAK

#endif // MX_HIDE_MALLOC_AND_FREE

//------------------------------------------------------------------------
//	Useful macros
//------------------------------------------------------------------------

#ifndef SAFE_DELETE
#define SAFE_DELETE( p )		{ if( p != nil ) { delete (p);     (p) = nil; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY( p )	{ if( p != nil ) { delete[] (p);   (p) = nil; } }
#endif

#define mxNULLIFY_POINTER(p)		{(p) = nil;}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
