// Debug stuff (Windows only)
#pragma once

//------------------------------------------------------------------
//		Defines.
//------------------------------------------------------------------

// Causes a breakpoint exception to occur.
#define ptBREAK			DebugBreak()

#define ptBREAK_IF(_CONDITION_)		{if (_CONDITION_) ptBREAK;}

#if MX_DEBUG
	#define ptDBG_BREAK		(::IsDebuggerPresent() ? DebugBreak() : false)
#else
	#define ptDBG_BREAK		(false)
#endif

//
// Macro used to eliminate compiler warning 4715 within a switch statement
// when all possible cases have already been accounted for.
//
// switch (a & 3) {
//     case 0: return 1;
//     case 1: return Foo();
//     case 2: return Bar();
//     case 3: return 1;
//     DEFAULT_UNREACHABLE;
//
#if MX_DEBUG
	#define mxDEFAULT_UNREACHABLE		default: ptBREAK
#else
	#if (_MSC_VER > 1200)
		#define mxDEFAULT_UNREACHABLE	default: __assume(0)
	#endif
#endif

//------------------------------------------------------------------
//		Memory debugging.
//------------------------------------------------------------------
/*
If you're using the debug heap in MVC++, memory is initialized and cleared with special values:

0xCD - "Clean" - Allocated in heap, but not initialized (new objects are filled with 0xCD when they are allocated).
0xDD - "Dead" - Released heap memory (freed blocks).
0xED - no-man's land for aligned routines
0xFD - "Fence" - "NoMansLand" fences automatically placed at boundary of heap memory (on either side of the memory used by an application). Should never be overwritten. If you do overwrite one, you're probably walking off the end of an array.
0xCC - "Clean" - Allocated on stack, but not initialized.
0xAB - "?" - Used by Microsoft's HeapAlloc() to mark "no man's land" guard bytes after allocated heap memory.
0xBA - "?" - Used by Microsoft's LocalAlloc(LMEM_FIXED) to mark uninitialized allocated heap memory.
0xFD - "Fence" Used by Microsoft's C++ debugging heap to mark "no man's land" guard bytes before and after allocated heap memory
0xFE - "Free" - Used by Microsoft's HeapFree() to mark freed heap memory

See the file "dbgheap.c".
*/

// uninitialized heap memory (MVC++-specific)
#define mxDBG_UNINITIALIZED_MEMORY_TAG		0xCDCDCDCD

// freed memory is overwritten with this value (MVC++-specific)
#define mxDBG_FREED_MEMORY_TAG				0xDDDDDDDD

//
//	Returns true if given a valid heap pointer.
//	Can be used for debugging purposes.
//
mxFORCEINLINE bool mxIsValidHeapPointer( const void* ptr )
{
	const size_t value = (size_t) ptr;
	return ( value != 0 )
		&& ( value != 0xCCCCCCCC )	// <- MVC++-specific (uninitialized stack memory)
		&& ( value != 0xFEEEFEEE )	// <- MVC++-specific (freed memory)
		&& ( value != 0xDDDDDDDD )	// <- MVC++-specific (freed memory)
		&& ( value != 0xFDFDFDFD )	// <- MVC++-specific
		&& ( value != 0xABABABAB )	// <- MVC++-specific ("no man's land" guard bytes after allocated heap memory)
		&& ( value != 0xCDCDCDCD )	// <- MVC++-specific (uninitialized heap memory)
		;
}

/*
================================================================================
	Minidump.

	From MSDN:

		Applications can produce user-mode minidump files, which contain a useful
	subset of the information contained in a crash dump file.
	Applications can create minidump files very quickly and efficiently.
		Because minidump files are small, they can be easily sent over the internet
	to technical support for the application.
	A minidump file does not contain as much information as a full crash dump file,
	but it contains enough information to perform basic debugging operations.
		To read a minidump file, you must have the binaries and symbol files
	available for the debugger.
		Current versions of Microsoft Office and Microsoft Windows
	create minidump files for the purpose of analyzing failures on customers' computers.

	The following DbgHelp functions are used with minidump files.
	
		MiniDumpCallback 
		MiniDumpReadDumpStream 
		MiniDumpWriteDump
================================================================================
*/
namespace Win32MiniDump
{
	/// setup the the Win32 exception callback hook
	void Setup();
	/// write a mini dump
	bool WriteMiniDump();
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
