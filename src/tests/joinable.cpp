#include "TkUtil/Threads.h"
#include "TkUtil/Mutex.h"
#include "TkUtil/MachineData.h"
#include "TkUtil/NetworkData.h"
#include "TkUtil/UTF8String.h"
#include "TkUtil/Exception.h"

#include <iostream>
#include <vector>
#include <stdio.h>


USING_STD
USING_UTIL

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
		: Writer (stream), _mutex (true)
	{ }

	virtual void write (const string& str);


	private :

	Mutex			_mutex;
};	// class ProtectedWriter


void ProtectedWriter::write (const string& str)
{
	_mutex.lock ( );
	Writer::write (str);
	_mutex.unlock ( );
}	// ProtectedWriter::write


class WriterThread : public JoinableThread
{
	public :

	WriterThread (size_t steps, Writer& writer)
		: JoinableThread ( ), _steps (steps), _writer (writer)	
	{
	}

	virtual ~WriterThread ( )
	{
		UTF8String 	deathMsg (charset);
		deathMsg << "Death of thread " << (unsigned long)getId ( ) << ".\n";
		_writer.write (deathMsg.iso ( ));
	}	// ~WriterThread


	protected :

	virtual void execute ( ) throw ( )
	{
/*		struct sched_param	param;
		int					policy	= 0;
		pthread_t			current	= pthread_self ( );
		pthread_getschedparam (current, &policy, &param);
cout << "THR " << (unsigned long)current << " PRIORITY : " << param.__sched_priority << " POLICY : " << policy << endl; */
		for (size_t i = 0; i < _steps; i++)
		{
			UTF8String	str (charset);
			str << "Thread " << getId ( ) << " is at step "
			    << (unsigned long)i << ".\n";
			_writer.write (str);
		}
	}


	private :

	Writer&				_writer;
	size_t				_steps;
};	// class WriterThread


int main (int argc, char* argv[])
{
/*		struct sched_param	param;
		int					policy	= 0;
		pthread_t			current	= pthread_self ( );
		pthread_getschedparam (current, &policy, &param);
cout << "THR " << (unsigned long)current << " PRIORITY : " << param.__sched_priority << " POLICY : " << policy << endl; */
	try
	{

	const size_t	nbProcs	= MachineData::instance ( ).getProcessorsNum ( );

	cout << "Computer " << NetworkData::getCurrentHostName ( ) << " has " << nbProcs << " processors." << endl;

	vector <JoinableThread*>	tasks;
	size_t						i	= 0;
	const size_t				max	= 10;
	Writer						writer (stdout);
	cout << "Creation of " << (unsigned long)max << " joinable writing tasks without mutex ..." << endl;
	for (i = 0; i < max; i++)
		tasks.push_back (new WriterThread (max, writer));
	cout << "Running tasks ..." << endl;
	for (i = 0; i < max; i++)
		tasks [i]->startTask ( );
	// Rem : sched_yield important, on peut sinon observer des plantages, probablement à cause de tâches qui ne se sont pas encore lancées.
//	sched_yield ( );	// v 6.2.0 : not still usefull
	cout << "Joining tasks ..." << endl;
	for (i = 0; i < max; i++)
	{
		tasks [i]->join ( );
	}
	cout << "Joinable writing tasks without mutex completed." << endl;
	cout << "Deleting tasks ..." << endl;
	for (i = 0; i < max; i++)
		delete tasks [i];
	cout << endl << endl;

	tasks.clear ( );
	ProtectedWriter			pwriter (stdout);
	cout << "Creation of " << (unsigned long)max << " joinable writing tasks with mutexes ..." << endl;
	for (i = 0; i < max; i++)
		tasks.push_back (new WriterThread (max, pwriter));
	cout << "Running tasks ..." << endl;
	for (i = 0; i < max; i++)
		tasks [i]->startTask ( );
//	sched_yield ( );	// v 6.2.0 : not still usefull
	cout << "Joining tasks ..." << endl;
	for (i = 0; i < max; i++)
	{
		tasks [i]->join ( );
//		delete tasks [i];
cout << "Deleting tasks." << endl;
	}
	for (i = 0; i < max; i++)
		delete tasks [i];
	cout << "Joinable writing tasks with mutexes completed." << endl;

	}
	catch (const Exception& exc)
	{
		cout << "Exception caught : " << exc.getFullMessage ( ) << endl;
	}
	catch (...)
	{
		cout << "Unexpected error caught." << endl;
	}

	return 0;
}	// main
