/*
=============================================================================
	File:	SlowTasks.h
	Desc:	Scheduler for slow tasks which usually last more than one frame
	(e.g. streaming, background resource loading, AI)
=============================================================================
*/
#pragma once

struct SlowTaskCtx
{
	UINT64			data;
};

typedef void SlowTaskExec( const SlowTaskCtx& context );

struct SlowTaskDesc
{
	SlowTaskExec *	callback;
	UINT64			data;
};

namespace SlowTasks
{

ERet Initialize();
void Shutdown();

void AddTask( const SlowTaskDesc& desc );
UINT32 NumPendingTasks();

}//namespace SlowTasks

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
