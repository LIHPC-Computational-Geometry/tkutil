#include "TkUtil/ThreadPool.h"
#include "TkUtil/MachineData.h"
#include "TkUtil/Exception.h"
#include "TkUtil/MemoryError.h"

#include <cassert>


using namespace std;
using namespace TkUtil;



namespace TkUtil
{

static const Charset	charset ("àéèùô");

	
// ============================================================================
//                       LA CLASSE ThreadPool::TaskIfc
// ============================================================================

ThreadPool::TaskIfc::TaskIfc ( )
	: _status (ThreadPool::TaskIfc::WAITING), _message ( ), _toDelete (false), _concurrencyFlag (0)
{
}	// TaskIfc::TaskIfc


ThreadPool::TaskIfc::TaskIfc (const ThreadPool::TaskIfc& task)
	: _status (task._status), _message (task._message), _toDelete (task._toDelete), _concurrencyFlag (0)
{
}	// TaskIfc::TaskIfc


ThreadPool::TaskIfc& ThreadPool::TaskIfc::operator = (const ThreadPool::TaskIfc& task)
{
	if (&task != this)
	{
		_status				= task._status;
		_message			= task._message;
		_toDelete			= task._toDelete;
		_concurrencyFlag	= task._concurrencyFlag;
	}	// if (&task != this)

	return *this;
}	// TaskIfc::operator =


ThreadPool::TaskIfc::~TaskIfc ( )
{
}	// TaskIfc::~TaskIfc


void ThreadPool::TaskIfc::execute ( )
{
}	// TaskIfc::execute


void ThreadPool::TaskIfc::setStatus (STATUS status)
{
	_status	= status;
}	// TaskIfc::setStatus


void ThreadPool::TaskIfc::setStatus (STATUS status, const UTF8String& msg)
{
	_status		= status;
	_message	= msg;
}	// TaskIfc::setStatus


// ============================================================================
//                       LA CLASSE ThreadPool::WorkerThread
// ============================================================================

ThreadPool::WorkerThread::WorkerThread ( )
	: _thread (0), _task (0), _haltMutex ( ), _halted (false), _completed (false)
{	// Rem : les bouléens _halted et _barrier ne sont pas initialisés ici pour éviter des accès concurrents détectés par intel inspector.
	// Ils seront initialisés dans execute.
}	// WorkerThread::WorkerThread


ThreadPool::WorkerThread::WorkerThread (const ThreadPool::WorkerThread&)
	: _thread (0), _task (0), _haltMutex ( ), _halted (true), _completed (true)
{
	assert (0 && "WorkerThread copy constructor is not allowed.");
}	// WorkerThread::WorkerThread


ThreadPool::WorkerThread& ThreadPool::WorkerThread::operator = (const ThreadPool::WorkerThread&)
{
	assert (0 && "WorkerThread assignment operator is not allowed.");

	return *this;
}	// WorkerThread::operator =


ThreadPool::WorkerThread::~WorkerThread ( )
{
	delete _thread;		_thread	= 0;
}	// WorkerThread::~WorkerThread


void ThreadPool::WorkerThread::execute ( )
{
	bool	halted	= false;
	{
		unique_lock<mutex>	haltLock (_haltMutex);
		halted	= _halted;
	}

	while (false == halted)
	{
		ThreadPool::TaskIfc::STATUS	status	= ThreadPool::TaskIfc::RUNNING;
		UTF8String					message;
		_task	= ThreadPool::instance ( ).getTask ( );
		{	// waked up during getTask.
			unique_lock<mutex>	haltLock (_haltMutex);
			if (true == _halted)
			{
				unique_lock<mutex>	completedLock (_completedMutex);
				_completed	= true;
				return;
			}	// if (true == _halted)
		}

		if (0 == _task)
		{
//cout << "Worker " << (unsigned long)this << " WAITS " << yieldDelay ( ) << " nanoseconds ..." << endl;
			const size_t	delay	= ThreadPool::yieldDelay ( );
			if (0 == delay)
				this_thread::yield ( );
			else
				this_thread::sleep_for (std::chrono::nanoseconds (delay));	// v 6.7.0
//cout << "Worker " << (unsigned long)this << " HAS WAITED." << endl;
		}	// if (0 == _task)
		else
		{
			try
			{
				_task->setStatus (status);
				_task->execute ( );
				const ThreadPool::TaskIfc::STATUS	s	= _task->getStatus ( );
				message	= _task->getMessage ( );
				switch (s)
				{
					case ThreadPool::TaskIfc::COMPLETED	:
					case ThreadPool::TaskIfc::CANCELED	:
					case ThreadPool::TaskIfc::IN_ERROR	:
						status	= s;
						break;
					default								:
						status	= ThreadPool::TaskIfc::COMPLETED;
				}	// switch (s)
			}
			catch (const Exception& exc)
			{
				status	= ThreadPool::TaskIfc::IN_ERROR;
				if (true == _task->getMessage ( ).empty ( ))
					message	= exc.getFullMessage ( );
			}
			catch (const exception& e)
			{
				status	= ThreadPool::TaskIfc::IN_ERROR;
				if (true == _task->getMessage ( ).empty ( ))
					message	= e.what ( );
			}
			catch (...)
			{
				status	= ThreadPool::TaskIfc::IN_ERROR;
			}

			_task->setStatus (status, message);

			// La prise en charge de cette tache est finie :
			ThreadPool::TaskIfc*	task	= _task;
			_task	= 0;

			// Informer le gestionnaire de l'accomplissement de la tache :
			ThreadPool::instance ( ).taskCompleted (*task);
		}	// while (false == _halted)

		{
			unique_lock<mutex>	haltLock (_haltMutex);
			halted	= _halted;
		}
	}	// else if (0 == _task)

	unique_lock<mutex>	completedLock (_completedMutex);
	_completed	= true;
}	// WorkerThread::execute


void ThreadPool::WorkerThread::start ( )
{
	if (0 != _thread)
		throw Exception (UTF8String ("WorkerThread::start : thread déjà en cours d'exécution.", charset));

	_thread	= new thread (&ThreadPool::WorkerThread::execute, this);
}	// WorkerThread::start


void ThreadPool::WorkerThread::stop ( )
{
	unique_lock<mutex>	haltLock (_haltMutex);
	_halted	= true;
}	// WorkerThread::stop


bool ThreadPool::WorkerThread::completed ( ) const
{
	unique_lock<mutex>	completedLock (_completedMutex);
	return _completed;
}	// WorkerThread::completed


void ThreadPool::WorkerThread::join ( )
{
	assert (0 != _thread);
	_thread->join ( );
}	// WorkerThread::join


// ============================================================================
//                          LA CLASSE ThreadPool
// ============================================================================


ThreadPool*			ThreadPool::_instance	= 0;
bool				ThreadPool::_completed	= true;		// !running
size_t				ThreadPool::_yieldDelay	= 100000;		// v 6.7.0 - 1 milliseconde

ThreadPool::ThreadPool (size_t tasksNum)
	: _thread (0), _tasksNum (tasksNum), 
	  _queuedTasks ( ), _runningTasks ( ), _deadTasks ( ), _workerThreads ( ),
	  _tasksMutex ( ), _tasksCond ( ), _wakeUpCondMutex ( ), _barrierCondMutex ( )
{	// Rem : les bouléens _halted et _barrier ne sont pas initialisés ici pour éviter des accès concurrents détectés par intel inspector.
	// Ils seront initialisés dans init.
}	// ThreadPool::ThreadPool


ThreadPool::ThreadPool (const ThreadPool&)
	: _thread (0), _tasksNum (0), _haltMutex ( ), _queuedTasks ( ), _runningTasks ( ), _deadTasks ( ), _workerThreads ( ),
	  _tasksMutex ( ), _tasksCond ( ), _wakeUpCondMutex ( ), _barrierCondMutex ( )
{
	assert (0 && "ThreadPool copy constructor is not allowed.");
}	// ThreadPool::ThreadPool


ThreadPool& ThreadPool::operator = (const ThreadPool&)
{
	assert (0 && "ThreadPool::operator = is not allowed.");
	return *this;
}	// ThreadPool::operator =


ThreadPool::~ThreadPool ( )
{
	deleteDeadTasks ( );
	delete _thread;		_thread	= 0;
}	// ThreadPool::~ThreadPool ( )


void ThreadPool::initialize (size_t tasksNum)
{
	if (0 != _instance)
		throw Exception (UTF8String ("ThreadPool::initialize : API déjà initialisée.", charset));

#ifdef __INTEL_COMPILER
	if (__INTEL_COMPILER < 1500)
	{
		ConsoleOutput::cerr ( )
		     << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
		     << co_endl
		     << "ATTENTION : LE MULTITHREADING AVEC L'UTILISATION DE L'API "
		     << "ThreadPool RISQUE DE PROVOQUER DES PLANTAGES." << co_endl
		     << "Cette API ThreadPool est incompatible avec les versions du "
		     << "compilateur Intel antérieures à la version 15.0 (version "
		     << "courante : " << (unsigned long)__INTEL_COMPILER << ")."
		     << co_endl
		     << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
		     << co_endl;
	}	// if (__INTEL_COMPILER < 1500)
#endif	// __INTEL_COMPILER

	_instance	= new ThreadPool (0 == tasksNum ? MachineData::instance ( ).getProcessorsNum ( ) : tasksNum);
	CHECK_NULL_PTR_ERROR (_instance)

	// Création/mise en service des travailleurs :
	_instance->init ( );
}	// ThreadPool::initialize


void ThreadPool::finalize ( )
{
	if (0 == _instance)
		throw Exception (UTF8String ("ThreadPool::finalize : API non initialisée.", charset));

	_instance->stop ( );
	_instance->stopWorkers ( );
	_instance->deleteWorkers ( );
	_instance->join ( );

	ThreadPool*	instance	= _instance;
	_instance	= 0;
	delete instance;
}	// ThreadPool::finalize


ThreadPool& ThreadPool::instance ( )
{
	if (0 == _instance)
		throw Exception (UTF8String ("ThreadPool::instance : API non initialisée.", charset));

	return *_instance;
}	// ThreadPool::instance


size_t ThreadPool::yieldDelay ( )	// v 6.7.0
{
	return _yieldDelay;
}	// ThreadPool::yieldDelay


void ThreadPool::setYieldDelay (size_t delay)	// v 6.7.0
{
	_yieldDelay	= delay;
}	// ThreadPool::setYieldDelay


void ThreadPool::stop ( )
{
	unique_lock<mutex>	haltLock (_haltMutex);
	_halted	= true;
}	// ThreadPool::stop


void ThreadPool::stopWorkers ( )
{
	// On demande l'arrêt des travailleurs. */
	for (vector<WorkerThread*>::iterator itw1 = _workerThreads.begin ( ); _workerThreads.end ( ) != itw1; itw1++)
		(*itw1)->stop ( );

	// On attend l'arrêt des travailleurs. */
	// On réveille les travailleurs s'ils étaient au chômage pour qu'ils
	// puissent être joignables :
	{
		unique_lock<mutex>	wakeUpCondLock (_wakeUpCondMutex);
		_wakeUpCond.notify_all ( );
	}

	bool	completed	= false;
	while (false == completed)
	{
		// Pour une raison inconnue le réveil ci-dessus ne fonctionne pas toujours bien, on remet donc ici une couche tant que tous les threads
		// ne sont pas achevés.
		// 12/2017 - gcc 4.4.6
		{
			unique_lock<mutex>	wakeUpCondLock (_wakeUpCondMutex);
			_wakeUpCond.notify_all ( );
		}
		this_thread::yield ( );
		completed	= true;
		for (vector<WorkerThread*>::iterator itw3 = _workerThreads.begin ( ); _workerThreads.end ( ) != itw3; itw3++)
			if (false == (*itw3)->completed ( ))
				completed	= false;
	}	// while (false == completed)

	for (vector<WorkerThread*>::iterator itw2 = _workerThreads.begin ( ); _workerThreads.end ( ) != itw2; itw2++)
		(*itw2)->join ( );
}	// ThreadPool::stopWorkers


void ThreadPool::addTask (ThreadPool::TaskIfc& task, bool barrier)
{
	unique_lock<mutex>	tasksLock (_tasksMutex);
	unique_lock<mutex>	barrierLock (_barrierMutex);

	_barrier	= false == barrier ? false : barrier;

	_queuedTasks.push_back (&task);

	// On réveille le gestionnaire s'il était au chômage :
	_tasksCond.notify_one ( );

	// On réveille les travailleurs s'ils étaient au chômage :
	unique_lock<mutex>	wakeUpCondLock (_wakeUpCondMutex);
	_wakeUpCond.notify_one ( );
}	// ThreadPool::addTask (ThreadIfc* thread)


void ThreadPool::addTasks (const vector<ThreadPool::TaskIfc*>& tasks, bool barrier)
{
	unique_lock<mutex>	tasksLock (_tasksMutex);
	unique_lock<mutex>	barrierLock (_barrierMutex);

	_barrier	= false == barrier ? false : barrier;

	_queuedTasks.insert (_queuedTasks.end ( ), tasks.begin ( ), tasks.end ( ));

	// On réveille le gestionnaire s'il était au chômage :
	_tasksCond.notify_one ( );

	// On réveille les travailleurs s'ils étaient au chômage :
	unique_lock<mutex>	wakeUpCondLock (_wakeUpCondMutex);
	_wakeUpCond.notify_all ( );
}	// ThreadPool::addTasks (ThreadIfc* thread)


void ThreadPool::stats (size_t& running, size_t& queued) const
{
	running	= queued	= 0;

	unique_lock<mutex>	tasksLock (_tasksMutex);
	queued	= _queuedTasks.size ( );
	running	= _runningTasks.size ( );
}	// ThreadPool::stats


void ThreadPool::barrier ( )
{
	{
		unique_lock<mutex>	barrierLock (_barrierMutex);
		_barrier	= true;
	}
	unique_lock<mutex>	barrierCondLock (_barrierCondMutex);
	// Patch du 09/04/18 : Faut il se mettre en situation d'attente ?
	// ATTENTION : on peut avoir _queuedTasks et _runningTasks vides.
	unique_lock<mutex>	tasksLock (_tasksMutex);	// 09/04/18
	if ((0 != _queuedTasks.size( )) || (0 != _runningTasks.size( )))// 09/04/18
	{
		tasksLock.unlock ( );	// 09/04/18
		_barrierCond.wait (barrierCondLock);
	}	// if ((0 != _queuedTasks.size ( )) || (0 != _runningTasks.size ( )))
	{
		unique_lock<mutex>	barrierLock (_barrierMutex);
		_barrier	= false;
	}
}	// ThreadPool::barrier


void ThreadPool::init ( )
{
	unique_lock<mutex>	haltLock (_haltMutex);
	unique_lock<mutex>	tasksLock (_tasksMutex);
	unique_lock<mutex>	wakeUpCondLock (_wakeUpCondMutex);

/*
ConsoleOutput::cout ( )
     << "======================================================================"
     << co_endl;
ConsoleOutput::cout ( )
     << "ThreadPool::init : utilisation de " << _tasksNum
     << " threads. La machine courante ("
     << MachineData::instance ( ).getHostName ( ) << ") est dotée de "
     << MachineData::instance ( ).getProcessorsNum ( )
     << " processeurs." << co_endl;
ConsoleOutput::cout ( )
     << "======================================================================"
     << co_endl;
*/
	_halted	= false;
	for (size_t i = 0; i < _tasksNum; i++)
	{
		ThreadPool::WorkerThread*	wt	= new ThreadPool::WorkerThread ( );
		_workerThreads.push_back (wt);
	}	// for (size_t i = 0; i < _tasksNum; i++)

	// L'instance est prête : on lance le thread.
	if (0 != _thread)
		throw Exception (UTF8String ("ThreadPool::init : thread déjà en cours d'exécution.", charset));
	_thread	= new thread (&ThreadPool::execute, this);
}	// ThreadPool::init


void ThreadPool::execute ( )
{
	bool	halted	= false;
	{
		unique_lock<mutex>	haltLock (_haltMutex);
		halted			= _halted;
	}
	{
		unique_lock<mutex>	completedLock (_tasksMutex);
		_completed		= false;
	}

	// On met les travailleurs en marche. */
	for (vector<WorkerThread*>::iterator itw1 = _workerThreads.begin ( ); _workerThreads.end ( ) != itw1; itw1++)
		(*itw1)->start ( );

	// La boucle d'exécution du gestionnaire de travailleurs :
	while (false == halted)
	{
		deleteDeadTasks ( );

//cout << "Worker " << (unsigned long)this << " WAITS " << yieldDelay ( ) << " nanoseconds ..." << endl;
		if (0 == _yieldDelay)
			this_thread::yield ( );
		else
			this_thread::sleep_for (std::chrono::nanoseconds (_yieldDelay));	// v 6.7.0
//cout << "Worker " << (unsigned long)this << " HAS WAITED." << endl;

		checkBarrier ( );

		{	// Mise en sommeil si absence de travail :
			unique_lock<mutex>	sleepLock (_tasksMutex);
			// Pas de mutex sur _barrierCondMutex : volontaire
			if ((false == _barrier) && (0 == _queuedTasks.size ( )) && (0 == _runningTasks.size ( )))
				_tasksCond.wait (sleepLock);
		}

		{
			unique_lock<mutex>	haltLock (_haltMutex);
			halted	= _halted;
		}
	}	// while (false == halted)

//	stopWorkers ( );
	{
		unique_lock<mutex>	completedLock (_tasksMutex);
		_completed		= true;
	}
}	// ThreadPool::execute


void ThreadPool::taskCompleted (ThreadPool::TaskIfc& task)
{
	unique_lock<mutex>	tasksLock (_tasksMutex);

	if (true == task.toDeleteAtCompletion ( ))
		_deadTasks.push_back (&task);

	bool	found	= false;
	for (vector<ThreadPool::TaskIfc*>::iterator it = _runningTasks.begin ( ); _runningTasks.end ( ) != it; it++)
	{
		if (&task == *it)
		{
			_runningTasks.erase (it);
			found	= true;
			break;
		}	// if (&task == *it)
	}	// for (vector<ThreadPool::TaskIfc*>::const_iterator it = ...

	// On réveille les travailleurs s'ils étaient au chômage. Peut être y a t'il maintenant une tache pouvant être lancée en concurrence avec celles
	// actives :
	unique_lock<mutex>	wakeUpCondLock (_wakeUpCondMutex);
	_wakeUpCond.notify_all ( );
}	// ThreadPool::taskCompleted


void ThreadPool::deleteDeadTasks ( )
{
	unique_lock<mutex>	tasksLock (_tasksMutex);

	try
	{
		for (vector<ThreadPool::TaskIfc*>::iterator it	= _deadTasks.begin ( ); _deadTasks.end ( ) != it; it++)
			delete *it;
		_deadTasks.clear ( );
	}
	catch (...)
	{
	}
}	// ThreadPool::deleteDeadTasks


void ThreadPool::deleteWorkers ( )
{
	unique_lock<mutex>	tasksLock (_tasksMutex);

	vector<WorkerThread*>	workers	= _workerThreads;
	int	step	= 0;
	while (false == _workerThreads.empty ( ))
	{
		for (vector<WorkerThread*>::iterator it = _workerThreads.begin ( ); _workerThreads.end ( ) != it; it++)
		{
			const bool	completed	= (*it)->completed ( );
			if (true == completed)
			{
				delete *it;
				_workerThreads.erase (it);
				break;
			}	// if (true == completed)
		}	// for (vector<WorkerThread*>::iterator it = _workerThreads.begin( )
	}	// while (false == _workerThreads.empty ( ))
}	// ThreadPool::deleteWorkers


void ThreadPool::checkBarrier ( )
{
	{
		unique_lock<mutex>	haltLock (_haltMutex);
		if (true == _halted)
			return;
	}

	{
		unique_lock<mutex>	tasksLock (_tasksMutex);

		if ((0 != _queuedTasks.size ( )) || (0 != _runningTasks.size ( )))
			return;

		{
			unique_lock<mutex>	barrierLock (_barrierMutex);
			if (false == _barrier)
				return;
		}
	}

	unique_lock<mutex>	barrierCondLock (_barrierCondMutex);
	_barrierCond.notify_one ( );
}	// ThreadPool::checkBarrier


bool ThreadPool::validateConcurrency (size_t flag) const
{	// Pas d'accès concurrents ici + déjà sous mutex par ThreadPool::getTask
	if (0 == flag)
		return true;

	for (vector<ThreadPool::TaskIfc*>::const_iterator it = _runningTasks.begin ( ); _runningTasks.end ( ) != it; it++)
		if (0 != ((*it)->getConcurrencyFlag ( ) & flag))
			return false;

	return true;
}	// ThreadPool::validateConcurrency


void ThreadPool::join ( )
{
	assert (0 != _thread);
	{
		unique_lock<mutex>	tasksLock (_tasksMutex);
		_barrier	= true;
	}

	// Idem ThreadPool::stopWorkers : pour une raison inconnue le réveil ne fonctionne pas forcément du premier coup, on remet donc ici plusieurs couches.
	// 12/2017 - gcc 4.4.6
//	for (int i = 0; i < 10; i++)
	{
		unique_lock<mutex>	sleepLock (_tasksMutex);
		_tasksCond.notify_all ( );
		this_thread::yield ( );
	}
	bool	completed	= false;
	while (false == completed)
	{
		unique_lock<mutex>	completionLock (_tasksMutex);
		completed	= _completed;
		this_thread::yield ( );
	}	// while (false == completed)
	_thread->join ( );
}	// ThreadPool::join


ThreadPool::TaskIfc* ThreadPool::getTask ( )
{
	{
		unique_lock<mutex>	haltLock (_haltMutex);
		if (true == _halted)
			return 0;
	}

	ThreadPool::TaskIfc*	task	= 0;
	{
		unique_lock<mutex>	tasksLock (_tasksMutex);

		if (0 != _queuedTasks.size ( ))
		{
			for (deque<ThreadPool::TaskIfc*>::iterator it = _queuedTasks.begin ( ); _queuedTasks.end ( ) != it; it++)
			{
				if (true == validateConcurrency ((*it)->getConcurrencyFlag ( )))
				{
					task	= *it;
					_queuedTasks.erase (it);
					_runningTasks.push_back (task);
					break;
				}	// if (true == validateConcurrency ( ...
			}	// for (deque<ThreadPool::TaskIfc*>::iterator it = ...
		}	// if (0 != _queuedTasks.size ( ))
	}

/*	if (0 == task)		v 6.7.0 : appel à ThreadPool::getTask ( ) non bloquant
	{
		unique_lock<mutex>	wakeUpCondLock (_wakeUpCondMutex);
		_wakeUpCond.wait (wakeUpCondLock);
	}	*/

	return task;
}	// ThreadPool::getTask


}	// namespace TkUtil
