#include "TkUtil/ThreadPool.h"
#include "TkUtil/MachineData.h"
#include "TkUtil/NetworkData.h"
#include "TkUtil/UTF8String.h"
#include "TkUtil/Exception.h"

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


using namespace std;
using namespace TkUtil;

static const Charset	charset ("àéèùô");

class Writer
{
	public :

	// Ecriture dans un flux non protégé par mutex :
	Writer (FILE* stream)
		: _stream (stream)
	{ }

	virtual void write (const string& str);


	private :

	FILE*	_stream;
};	// class Writer


void Writer::write (const string& str)
{
	for (int i = 0; i < str.length ( ); i++)
		fputc (str [i], _stream);
}	// Writer::write


class ProtectedWriter : public Writer
{
	public :

	ProtectedWriter (FILE* stream)
		: Writer (stream), _mutex ( )
	{ }

	virtual void write (const string& str);


	private :

	mutex			_mutex;
};	// class ProtectedWriter


void ProtectedWriter::write (const string& str)
{
	unique_lock<mutex>	lock (_mutex);
	Writer::write (str);
}	// ProtectedWriter::write


class WriteTask : public ThreadPool::TaskIfc
{
	public :

	WriteTask (size_t steps, Writer& writer, size_t id, size_t flag = 0)
		: ThreadPool::TaskIfc ( ), _steps (steps), _writer (writer), _id (id)
	{
	}

	virtual ~WriteTask ( )
	{
	  UTF8String 	deathMsg (charset);
		deathMsg << "Death of task " << (unsigned long)getId ( ) << ".\n";
		_writer.write (deathMsg.iso ( ));
	}	// ~WriteTask

	virtual pthread_t getId ( )
	{ return _id; }


	protected :

	virtual void execute ( )
	{
		for (size_t i = 0; i < _steps; i++)
		{
			UTF8String	str (charset);
			str << "Thread " << getId ( ) << " is at step "
			    << (unsigned long)i << ".\n";
			_writer.write (str);

			// Certaines taches (~ 1 sur 10) finissent mal :
			int	r	= (int)(((double)rand( ) / (double)RAND_MAX) * 10 * _steps);
			if (1 == r)
				throw Exception (UTF8String ("Erreur d'écriture", charset));
			else if (0 == r)
			{
				setStatus (
						IN_ERROR, UTF8String ("Erreur d'algorithme.", charset));
				break;
			}	// else if (0 == r)
			else if (2 == r)
			{
				setStatus (
						CANCELED, UTF8String ("Ecriture annulée.", charset));
				break;
			}	// else if (2 == r)
		}
	}


	private :

	Writer&				_writer;
	size_t				_steps;
	size_t				_id;
};	// class WriteTask


int main (int argc, char* argv[])
{
	try
	{
		const size_t	nbProcs	= MachineData::instance ( ).getProcessorsNum ( );

		cout << "Computer " << NetworkData::getCurrentHostName ( ) << " has "
			<< nbProcs << " processors." << endl;

		ThreadPool::initialize (nbProcs / 3);
	//	ThreadPool::initialize (2);
		srand (time (NULL));

		size_t						i	= 0;
		const size_t				max	= 100;

		ProtectedWriter*			pwriter	= new ProtectedWriter (stdout);
		cout << "Creation of " << (unsigned long)max
			<< " writing tasks with mutexes ..." << endl;
		size_t	running	= 0, queued = 0;
		UTF8String	message;
		for (i = 0; i < max; i++)
		{
			WriteTask*	task	= new WriteTask(max, *pwriter, i);
			task->toDeleteAtCompletion (true);
			ThreadPool::instance ( ).addTask (*task, true);
			ThreadPool::instance ( ).stats (running, queued);
			message.clear ( );
			message << "Progression : " << (max - running - queued) << "/"
					<< max << "\n";
			pwriter->write (message.ascii ( ));
		}	// for (i = 0; i < max; i++)
		pwriter->write ("Joining tasks ...\n");
		ThreadPool::instance ( ).barrier ( );
		ThreadPool::instance ( ).stats (running, queued);
		message.clear ( );
		message << "Progression : " << (max - running - queued) << "/" << max;
		pwriter->write (message.ascii ( ));
		pwriter->write ("Joinable writing tasks with mutexes completed.\n\n");

		message.clear ( );
		message << "Creation of " << (unsigned long)max
			<< " writing tasks with mutexes and concurrency flag ...\n";
		pwriter->write (message.ascii ( ));
		for (i = 0; i < max; i++)
		{
			WriteTask*	task	= new WriteTask(max, *pwriter, i);
			task->toDeleteAtCompletion (true);
			task->setConcurrencyFlag (i % 2);
			ThreadPool::instance ( ).addTask (*task, true);
			ThreadPool::instance ( ).stats (running, queued);
			message.clear ( );
			message << "Progression : " << (max - running - queued) << "/"
				<< max << "\n";
			pwriter->write (message.ascii ( ));
		}	// for (i = 0; i < max; i++)
		pwriter->write ("Joining tasks ...\n");
		ThreadPool::instance ( ).barrier ( );
		ThreadPool::instance ( ).stats (running, queued);
		message.clear ( );
		message << "Progression : " << (max - running - queued) << "/" << max
				<< "\n"
				<< "Joinable writing tasks with mutexes and concurrency flag "
				<< "completed.\n"
				<< "Creation of " << (unsigned long)max << " writing tasks with "
				<< "mutexes and concurrency flag and give them together ...\n";
		pwriter->write (message.ascii ( ));
		vector<ThreadPool::TaskIfc*>	tasks;
		for (i = 0; i < max; i++)
		{
			WriteTask*	task	= new WriteTask(max, *pwriter, i);
			task->toDeleteAtCompletion (false);
			task->setConcurrencyFlag (i % 2);
			tasks.push_back (task);
			ThreadPool::instance ( ).stats (running, queued);
			message.clear ( );
			message << "Progression : " << (max - running - queued) << "/"
					<< max << "\n";
			pwriter->write (message.ascii ( ));
		}	// for (i = 0; i < max; i++)
		ThreadPool::instance ( ).addTasks (tasks, true);
		pwriter->write ("Joining tasks ...\n");
		ThreadPool::instance ( ).barrier ( );
		ThreadPool::instance ( ).stats (running, queued);
		message.clear ( );
		message << "Progression : " << (max - running - queued) << "/" << max
				<< "\n"
				<< "Joinable writing tasks with mutexes and concurrency flag "
				<< "given together completed.\n";
		pwriter->write (message.ascii ( ));

		// On évalue les taches :
		for (vector<ThreadPool::TaskIfc*>::iterator itt	=  tasks.begin ( );
			itt != tasks.end ( ); itt++)
		{
			switch ((*itt)->getStatus ( ))
			{
				case ThreadPool::TaskIfc::CANCELED	:
					cout << "Tache annulée : " << (*itt)->getMessage ( ) << endl;
					break;
				case ThreadPool::TaskIfc::IN_ERROR	:
					cout << "Tache en erreur : " << (*itt)->getMessage ( ) << endl;
					break;
			}	// switch ((*itt)->getStatus ( ))

			delete *itt;
		}	// for (vector<ThreadPool::TaskIfc*>::iterator itt =  tasks.begin ( );
		

		ThreadPool::finalize ( );
	}
	catch (const Exception& exc)
	{
		cout << "Exception caught : " << exc.getFullMessage ( ) << endl;
		return -1;
	}
	catch (...)
	{
		cout << "Unexpected error caught." << endl;
		return -1;
	}

	return 0;
}	// main
