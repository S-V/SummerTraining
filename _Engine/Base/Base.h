/*
=============================================================================
	File:	Base.h
	Desc:	Foundation Library public header file.
=============================================================================
*/
#pragma once

#if MX_AUTOLINK
#pragma comment( lib, "Base.lib" )
#endif //MX_AUTOLINK

//---------------------------------------------------------------
//	Public header files.
//---------------------------------------------------------------

#include "Build/BuildConfig.h"		// Build configuration options, compile settings.
#include "System/ptPlatform.h"		// Platform-specific stuff.
#include "System/ptMacros.h"
#include "Debug/ptDebug.h"
#include "System/PtBaseTypes.h"
#include "System/ptProfiler.h"

// Forward declarations of common types and functions.

class String;

class ALog;

class URI;
class mxDataStream;
class AStreamWriter;
class AStreamReader;

template<
	typename TYPE,
	class DERIVED
> class TArrayBase;

template<
	class T
> class TArray;

template<
	typename TYPE,
	const UINT SIZE
> class TStaticArray;

template<
	typename TYPE,
	const UINT SIZE
> class TStaticList;

template<
	typename KEY,
	typename VALUE,
	class HASH_FUNC,
	class EQUALS_FUNC,
	typename SIZETYPE
> class THashMap;

template<
	typename KEY,
	typename VALUE,
	class HASH_FUNC,
	class EQUALS_FUNC
> class TDynaMap;

template<
	typename VALUE,
	class HASH_FUNC
> class TPointerMap;

// High-level concepts.
class AObject;

// Platform-independent application entry point.
extern int mxAppMain();

//------ Common ----------------------------------------------------

// dependencies
#include "IO/StreamIO.h"

//------ Memory management ------------------------------------------------

#include "Memory/Memory.h"
//#include "Memory/BlockAlloc/BlockAllocator.h"
//#include "Memory/BlockAlloc/DynamicBlockAlloc.h"

//------ Templates ------------------------------------------------

// Common stuff.
//#include "Template/Templates.h"

// Smart pointers.
#include "Template/SmartPtr/TPtr.h"
#include "Template/SmartPtr/TAutoPtr.h"
//#include "Template/SmartPtr/TSharedPtr.h"
#include "Template/SmartPtr/TSmartPtr.h"
#include "Template/SmartPtr/TReference.h"
#include "Template/SmartPtr/ReferenceCounted.h"

// Arrays.
#include "Template/Containers/Array/TStaticList.h"
#include "Template/Containers/Array/TStaticArray.h"
#include "Template/Containers/Array/TStatic2DArray.h"
#include "Template/Containers/Array/TFixedArray.h"
#include "Template/Containers/Array/Array.h"
#include "Template/Containers/Array/TBuffer.h"

// Lists.
#include "Template/Containers/LinkedList/TCircularList.h"
#include <Base/Template/Containers/LinkedList/TSinglyLinkedList.h>
#include <Base/Template/Containers/LinkedList/TDoublyLinkedList.h>
#include "Template/Containers/HashMap/TKeyValue.h"

//------ String ----------------------------------------------------

#include "Text/StringTools.h"
#include "Text/String.h"		// String type.
//#include "Text/TextUtils.h"
//#include "Text/Token.h"
//#include "Text/Lexer.h"
//#include "Text/Parser.h"

// low-level math
#include "Math/ptInteger.h"
#include "Math/ptFloat16.h"
#include "Math/ptFloat32.h"
#include "Math/NewMath.h"
#include "Math/Hashing/HashFunctions.h"

// Input/Output system.
#include "IO/StreamIO.h"
#include "IO/FileIO.h"
//#include "IO/IOServer.h"
//#include "IO/URI.h"
//#include "IO/DataStream.h"
//#include "IO/Archive.h"
//#include "IO/FileStream.h"
//#include "IO/MemoryStream.h"
#include "IO/Log.h"



//#include "Template/Containers/HashMap/TStringMap.h"
//#include "Template/Containers/HashMap/BTree.h"
//#include "Template/Containers/HashMap/RBTreeMap.h"
//#include "Template/Containers/HashMap/THashMap.h"
//#include "Template/Containers/HashMap/TDynaMap.h"
//#include "Template/Containers/HashMap/TKeyValue.h"
//#include "Template/Containers/HashMap/Dictionary.h"
//#include "Template/Containers/HashMap/TPointerMap.h"
//#include "Template/Containers/HashMap/StringMap.h"

//#include "Template/Containers/BitSet/BitField.h"
//#include "Template/Containers/BitSet/BitArray.h"

//------ Math ------------------------------------------------------

// Hashing
//#include "Math/Hashing/CRC8.h"
//#include "Math/Hashing/CRC16.h"
//#include "Math/Hashing/CRC32.h"
//#include "Math/Hashing/Honeyman.h"
//#include "Math/Hashing/MD4.h"
//#include "Math/Hashing/MD5.h"

// Common
//#include "Math/Math.h"


//------ Object system ------------------------------------------------------

//#include "Object/ObjectFactory.h"	// Class factory. NOTE: must be included before 'TypeDescriptor.h' !
//#include "Object/TypeDescriptor.h"			// Run-Time Type Information.
//#include "Object/Object.h"			// Base class. NOTE: must be included after 'TypeDescriptor.h' !
//#include "Object/Message.h"			// Messaging.

#include "Object/Reflection.h"
#include "Object/StructDescriptor.h"
#include "Object/ClassDescriptor.h"
// for reflecting array types
#include "Object/ArrayDescriptor.h"
#include "Object/PointerType.h"
#include "Object/EnumType.h"
#include "Object/FlagsType.h"
#include "Object/UserPointerType.h"
#include "Object/BinaryBlobType.h"
#include "Object/ManualSerialization.h"

//------ Miscellaneous Utilities --------------------------------------------

//#include "Util/Sorting.h"
//#include "Util/Rectangle.h"
//#include "Util/FourCC.h"
//#include "Util/Color.h"
//#include "Util/Misc.h"
#include "Util/Version.h"

//---------------------------------------------------------------
//	Don't forget to call these functions before/after
//	using anything from the base system!
//---------------------------------------------------------------

// must be called on startup by each system dependent on the base system (increments internal ref counter)
bool	mxInitializeBase();

// must be called on shutdown by each system dependent on the base system (decrements internal ref counter);
// returns 'true' if the system has really been shut down
bool	mxShutdownBase();

bool	mxBaseSystemIsInitialized();

// unsafe, terminates the app immediately
void	mxForceExit( int exitCode );

// sets the function to be called before closing the base subsystem.
// can be used to clean up resources after a fatal error has occurred
void	mxSetExitHandler( FCallback* pFunc, void* pArg );

void	mxGetExitHandler( FCallback **pFunc, void **pArg );


struct SetupBaseUtil
{
	SetupBaseUtil()
	{
		mxInitializeBase();
	}
	~SetupBaseUtil()
	{
		mxShutdownBase();
	}
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
