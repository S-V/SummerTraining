#pragma once

#include <TxTSupport/TxTCommon.h>

namespace SON
{

#if MX_DEBUG
	typedef unsigned char CharType;	// view character codes in debugger
#else
	typedef UINT32 CharType;		// register-sized integers are faster
#endif

// Use 32-bit integers as array indices/child counters.
//typedef UINT32 SizeType;

// Maximum level of nested structures (recursion depth).
//enum { PARSER_STACK_SIZE = 32 };

	// parser state
	struct Parser
	{
		char *	buffer;
		UINT32	length;

		UINT32	position;	// current position in the source file
		UINT32	line;		// current line (default value == 1)
		UINT32	column;		// current column (default == 0)

		const char*	file;	// file name for debugging

		INT32	errorCode;	// 0 == no error

	public:
		Parser();
		~Parser();
	};

	Node* ParseBuffer(
		Parser& _parser,
		Allocator & _allocator
	);

}//namespace SON

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
