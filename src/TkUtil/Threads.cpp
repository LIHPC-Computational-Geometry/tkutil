#include "TkUtil/Mutex.h"
#include "TkUtil/Threads.h"
#include "TkUtil/ThreadManager.h"
#include "TkUtil/Exception.h"
#include "TkUtil/MemoryError.h"


#include <assert.h>
#include <unistd.h>


USING_UTIL
USING_STD



BEGIN_NAMESPACE_UTIL

static const Charset	charset ("àéèùô");


#define CHECK_POSIX_CALL(call,name)                                          \
    {                                                                        \
	int	retVal	= call;                                                      \
	if (0 != retVal)                                                         \
	{                                                                        \
		UTF8String	msg (charset);                                           \
		msg << "Erreur lors de l'appel à la fonction " << #name << ".";      \
		throw Exception (msg);                                               \
	}                                                                        \
	}

	
// ============================================================================
//                          L'INTERFACE ThreadIfc
// ============================================================================


vector<ThreadIfc*>		ThreadIfc::_threads;
Mutex					ThreadIfc::_mutex;


ThreadIfc::ThreadIfc (size_t concurrencyFlag)
	: _concurrencyFlag (concurrencyFlag), _thread (0), _running (true), _cancelled (false)	// v 6.2.0
{	// v 6.2.0 : true est affecté d'emblée à _running car il pouvait arriver que join soit invoqué avant que startTask
	// n'aie appelé run_thread ou que tout au moins que run_thread n'aie affecté true à running.
	registerThread (this);
}	// ThreadIfc::ThreadIfc


ThreadIfc::ThreadIfc (const ThreadIfc&)
{
	assert (0 && "ThreadIfc copy constructor is not allowed.");
}	// ThreadIfc::ThreadIfc


ThreadIfc& ThreadIfc::operator = (const ThreadIfc&)
{
	assert (0 && "ThreadIfc::operator = is not allowed.");
	return *this;
}	// ThreadIfc::operator =


ThreadIfc::~ThreadIfc ( )
{
	unregisterThread (this);
}	// ThreadIfc::~ThreadIfc ( )


pthread_t ThreadIfc::getId ( ) const
{
	return getPosixThread ( );
}	// ThreadIfc::getId


bool ThreadIfc::isRunning ( ) const
{
	return _running;
}	// ThreadIfc::isRunning


bool ThreadIfc::cancelRequested ( ) const
{
	return _cancelled;
}	// ThreadIfc::cancelRequested


void ThreadIfc::cancel ( )
{
	_cancelled	= true;
	_running	= false;
}	// ThreadIfc::cancel


ThreadIfc* ThreadIfc::current ( )
{
	return withId (pthread_self ( ));
}	// ThreadIfc::current


ThreadIfc* ThreadIfc::withId (pthread_t id)
{
	int	old	=0, current = 0;

	AutoMutex	mutex (&_mutex);

	for (vector<ThreadIfc*>::iterator it = _threads.begin ( ); _threads.end ( ) != it; it++)
	{
		CHECK_NULL_PTR_ERROR (*it)
		if ((*it)->getId ( ) == id)
			return *it;
	}	// for (vector<ThreadIfc*>::iterator it = _threads.begin ( ); ...

	return 0;
}	// ThreadIfc::withId


void ThreadIfc::registerThread (ThreadIfc* thread)
{
	int	old	=0, current = 0;
	AutoMutex	mutex (&_mutex);

	CHECK_NULL_PTR_ERROR (thread)
	_threads.push_back (thread);
}	// ThreadIfc::registerThread


void ThreadIfc::unregisterThread (ThreadIfc* thread)
{
	int	old	=0, current = 0;
	AutoMutex	mutex (&_mutex);

	CHECK_NULL_PTR_ERROR (thread)
	for (vector<ThreadIfc*>::iterator it = _threads.begin ( ); _threads.end ( ) != it; it++)
	{
		if (*it == thread)
		{
			_threads.erase (it);
			break;
		}	// if (*it == thread)
	}	// for (vector<ThreadIfc*>::iterator it = ...
}	// ThreadIfc::unregisterThread


void* ThreadIfc::run_thread (void* t)
{
	ThreadIfc*	thread	= (ThreadIfc*)(t);
	assert (0 != thread);

	thread->_running	= true;
	thread->execute ( );
	thread->_running	= false;

	try
	{
		// ThreadManager::instance ( ) lève une exception si l'API n'est pas initialisée.
		ThreadManager::instance ( ).taskCompleted (thread);
	}
	catch (...)
	{
	}

	return 0;
}	// ThreadIfc::run_thread


// ============================================================================
//                          LA CLASSE DetachedThread
// ============================================================================


DetachedThread::DetachedThread (size_t concurrencyFlag)
	: ThreadIfc (concurrencyFlag)
{
}	// DetachedThread::DetachedThread


DetachedThread::DetachedThread (const DetachedThread&)
	: ThreadIfc (0)
{
	assert (0 && "DetachedThread copy constructor is not allowed.");
}	// DetachedThread::DetachedThread


DetachedThread& DetachedThread::operator = (const DetachedThread&)
{
	assert (0 && "DetachedThread::operator = is not allowed.");
	return *this;
}	// DetachedThread::operator =


DetachedThread::~DetachedThread ( )
{
}	// DetachedThread::~DetachedThread ( )


void DetachedThread::startTask ( )
{
	pthread_attr_t	attr;
	pthread_attr_init (&attr);
	CHECK_POSIX_CALL (pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED), "pthread_attr_setdetachstate")
#ifdef FORCE_THREADS_STACK_SIZE
	CHECK_POSIX_CALL (pthread_attr_setstacksize(&attr,(2*PTHREAD_STACK_MIN)), "pthread_attr_setstacksize")
#endif	// FORCE_THREADS_STACK_SIZE
	CHECK_POSIX_CALL(pthread_create (&getPosixThread ( ), &attr, ThreadIfc::run_thread, this),"pthread_create")
	CHECK_POSIX_CALL(pthread_attr_destroy (&attr), "pthread_attr_destroy")
//	CHECK_POSIX_CALL(pthread_detach (getPosixThread ( )), "pthread_detach")
}	// DetachedThread::startTask


// ============================================================================
//                          LA CLASSE JoinableThread
// ============================================================================


JoinableThread::JoinableThread (size_t concurrencyFlag)
	: ThreadIfc (concurrencyFlag)
{
}	// JoinableThread::JoinableThread


JoinableThread::JoinableThread (const JoinableThread&)
	: ThreadIfc (0)
{
	assert (0 && "JoinableThread copy constructor is not allowed.");
}	// JoinableThread::JoinableThread


JoinableThread& JoinableThread::operator = (const JoinableThread&)
{
	assert (0 && "JoinableThread::operator = is not allowed.");
	return *this;
}	// JoinableThread::operator =


JoinableThread::~JoinableThread ( )
{
}	// JoinableThread::~JoinableThread ( )


void JoinableThread::startTask ( )
{
	// Rem : par défaut les threads créés sont de type joinable.
	pthread_attr_t	attr;
	CHECK_POSIX_CALL (pthread_attr_init (&attr), "pthread_attr_init")
	CHECK_POSIX_CALL (pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE), "pthread_attr_setdetachstate")
#ifdef FORCE_THREADS_STACK_SIZE
	CHECK_POSIX_CALL (pthread_attr_setstacksize(&attr,(2*PTHREAD_STACK_MIN)), "pthread_attr_setstacksize")
#endif	// FORCE_THREADS_STACK_SIZE
	CHECK_POSIX_CALL (pthread_create (&getPosixThread ( ), &attr, ThreadIfc::run_thread, this), "pthread_create")
	CHECK_POSIX_CALL (pthread_attr_destroy (&attr), "pthread_attr_destroy")
}	// JoinableThread::startTask


void JoinableThread::join ( )
{
	if (true == isRunning ( ))
	{
		void*	status	= 0;
		CHECK_POSIX_CALL(pthread_join (getPosixThread ( ), &status), "pthread_join")
	}	// if (true == isRunning ( ))
}	// JoinableThread::join


END_NAMESPACE_UTIL
