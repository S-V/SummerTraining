/*
=============================================================================
	File:	SlowTasks.cpp
	Desc:	Scheduler for slow tasks which usually last more than one frame
	(e.g. streaming, background resource loading, AI)
=============================================================================
*/
#include <Core/Core_PCH.h>
#pragma hdrstop
#include <Core/Core.h>
#include <Core/SlowTasks.h>
#include <Core/bitsquid/queue.h>

namespace SlowTasks
{

namespace bitsquid = foundation;

struct QueuedTask
{
	SlowTaskExec *	callback;
	UINT64			data;
};

struct SlowTasksManager
{
	bitsquid::Queue< QueuedTask >	pendingTasks;	// pending tasks
	SpinWait	pendingTasksCS;	// for serializing access to the pending tasks queue

	bitsquid::Queue< QueuedTask >	completedTasks;	// executed tasks
	SpinWait	completedTasksCS;

	Thread	thread9;

	bool	exiting;

public:
	SlowTasksManager()
		: pendingTasks( bitsquid::memory_globals::default_allocator() )
		, completedTasks( bitsquid::memory_globals::default_allocator() )
	{}
};
mxDECLARE_PRIVATE_DATA( SlowTasksManager, gSlowTasksManager );

#define me	mxGET_PRIVATE_DATA( SlowTasksManager, gSlowTasksManager )

static UINT32 ThreadFunction( void* _taskMgr );

ERet Initialize()
{
	mxINITIALIZE_PRIVATE_DATA( gSlowTasksManager );

	me.pendingTasksCS.Initialize();
	me.completedTasksCS.Initialize();

	return ALL_OK;
}

void Shutdown()
{
	//me.pendingTasks.Empty();
	//me.completedTasks.Empty();

	me.pendingTasksCS.Shutdown();
	me.completedTasksCS.Shutdown();

	mxSHUTDOWN_PRIVATE_DATA( gSlowTasksManager );
}

void AddTask( const SlowTaskDesc& desc )
{
	SpinWait::Lock	scopedLock( me.pendingTasksCS );

	QueuedTask	newTask;
	newTask.callback = desc.callback;
	newTask.data = desc.data;

	bitsquid::queue::push_back( me.pendingTasks, newTask );
}

UINT32 NumPendingTasks()
{
	SpinWait::Lock	scopedLock( me.pendingTasksCS );
	return bitsquid::queue::size( me.pendingTasks );
}

void AddCompletedTask( const QueuedTask& task )
{
	SpinWait::Lock	scopedLock( me.completedTasksCS );
	bitsquid::queue::push_back( me.completedTasks, task );
}

static UINT32 ThreadFunction( void* _taskMgr )
{
	SlowTasksManager* taskMgr = static_cast< SlowTasksManager* >(_taskMgr);

	while( !taskMgr->exiting )
	{
		me.pendingTasksCS.Enter();
		if( !bitsquid::queue::size( me.pendingTasks ) )
		{
			me.pendingTasksCS.Leave();
			continue;
		}
		QueuedTask* task = bitsquid::queue::begin_front(me.pendingTasks);
		me.pendingTasksCS.Leave();

		SlowTaskCtx	context;
		context.data = task->data;
		(*task->callback)( context );

		AddCompletedTask( *task );

		me.pendingTasksCS.Enter();
		bitsquid::queue::pop_front(me.pendingTasks);
		me.pendingTasksCS.Leave();
	}
	return 0;
}

}//namespace SlowTasks

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
