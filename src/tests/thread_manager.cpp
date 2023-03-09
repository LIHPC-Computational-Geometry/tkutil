#include "TkUtil/ThreadManager.h"
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
		: Writer (stream), _mutex ( )
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

	WriterThread (size_t steps, Writer& writer, size_t id, size_t flag = 0)
		: JoinableThread (flag), _steps (steps), _writer (writer), _id (id)
	{
	}

	virtual ~WriterThread ( )
	{
		UTF8String 	deathMsg (charset);
		deathMsg << "Death of thread " << (unsigned long)getId ( ) << ".\n";
		_writer.write (deathMsg.iso ( ));
	}	// ~WriterThread

	virtual pthread_t getId ( )
	{ return _id; }


	protected :

//	virtual void execute ( ) throw ( )
	virtual void execute ( )
	{
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
	size_t				_id;
};	// class WriterThread


int main (int argc, char* argv[])
{
	try
	{
		const size_t	nbProcs	= MachineData::instance ( ).getProcessorsNum ( );

		cout << "Computer " << NetworkData::getCurrentHostName ( ) << " has "
			<< nbProcs << " processors." << endl;

		ThreadManager::initialize ( );

	//	vector <JoinableThread*>	tasks;
		size_t						i	= 0;
		const size_t				max	= 10;
		Writer						writer (stdout);
		cout << "Creation of " << (unsigned long)max
			<< " joinable writing tasks without mutex ..." << endl;
		for (i = 0; i < max; i++)
			ThreadManager::instance ( ).addTask (new WriterThread (max, writer, i));
		cout << "Joining tasks ..." << endl;
		ThreadManager::instance ( ).join ( );
		cout << "Joinable writing tasks without mutex completed." << endl;
		cout << endl << endl;

		ProtectedWriter			pwriter (stdout);
		cout << "Creation of " << (unsigned long)max
			<< " joinable writing tasks with mutexes ..." << endl;
		for (i = 0; i < max; i++)
			ThreadManager::instance ( ).addTask (new WriterThread(max, pwriter, i));
		cout << "Joining tasks ..." << endl;
		ThreadManager::instance ( ).join ( );
		cout << "Joinable writing tasks with mutexes completed." << endl;
		cout << endl << endl;

		cout << "Creation of " << (unsigned long)max
			<< " joinable writing tasks with mutexes and concurrency flag ..."
			<< endl;
		for (i = 0; i < max; i++)
			ThreadManager::instance ( ).addTask (
				new WriterThread (max, pwriter, i, i % 2));
		cout << "Joining tasks ..." << endl;
		ThreadManager::instance ( ).join ( );
		cout << "Joinable writing tasks with mutexes and concurrency flag completed." << endl;

		ThreadManager::finalize ( );
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
