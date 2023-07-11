#include "TkUtil/Process.h"
#include "TkUtil/File.h"
#include "TkUtil/UTF8String.h"
#include "TkUtil/Exception.h"

#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>	// memset
#include <strings.h>	// strdup
#include <stdlib.h>		// getenv
#include <unistd.h>		// getcwd
#include <fcntl.h>

#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>

// ATTENTION : sous SGI/g++ ces declarations doivent venir apres les 
// en-tetes c++
# if defined(__GNUC__) && defined(__sgi)
#define _POSIX_C_SOURCE 1
#endif	// if defined(__GNUC__) && defined(__sgi)
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

//#define TKUTIL_PIPE_BUFFER_SIZE (PIPE_BUF)
#define TKUTIL_PIPE_BUFFER_SIZE (1000)

USING_UTIL
USING_STD


BEGIN_NAMESPACE_UTIL

static const Charset	charset ("àéèùô");

// ===========================================================================
//                            Classe ProcessOptions
// ===========================================================================

Process::ProcessOptions::ProcessOptions ( )
	: _options ( )
{
}	// ProcessOptions::ProcessOptions


Process::ProcessOptions::ProcessOptions (const ProcessOptions& copied)
	: _options (copied.getOptions ( ))
{
}	// ProcessOptions::ProcessOptions


Process::ProcessOptions& Process::ProcessOptions::operator = (
						const ProcessOptions& copied)
{
	if (&copied != this)
		_options	= copied.getOptions ( );

	return *this;
}	// ProcessOptions::ProcessOptions


void Process::ProcessOptions::addOption (const string& arg)
{
	_options.push_back (arg);
}	// ProcessOptions::addOption


const vector<string>& Process::ProcessOptions::getOptions ( ) const
{
	return _options;
}	// ProcessOptions::getOptions


// ===========================================================================
//                              Classe Process
// ===========================================================================

vector<Process*>	Process::_tasks;
int					Process::_argc	= 0;
char**				Process::_argv	= 0;
char**				Process::_envp	= 0;
string				Process::_currentSoftware;
Version				Process::_currentSoftwareVersion ("0.0.0");


Process::Process (const string& processName)
	: _processName (processName), _options ( ), _processId ((pid_t)-1),
	  _completed (false),
	  _childToSonCommunications (false), _blockingCommunications (true),
	  _waited (false), _childBuffer (""), _completionCode (-1)
//	  _errorMessage ("")	// v4.4.0
{
	memset (_errorMessage, '\0', PROCESS_ERROR_MESSAGE_SIZE * sizeof (char));	// v 4.4.0
	_tasks.push_back (this);
	_fifo [0]	= _fifo [1]	= -1;
}	// Process::Process


Process::Process (const Process&)
{
	assert (0 && "Process::Process (const Process&) is not allowed.");
}	// Process::Process (const Process&)


Process& Process::operator = (const Process&)
{
	assert (0 && "Process::operator = (const Process&) is not allowed.");
	return *this;
}	// Process::Process (const Process&)


Process::~Process ( )
{
	for (vector<Process*>::iterator it = _tasks.begin ( );
	     _tasks.end ( ) != it; it++)
		if (*it == this)
		{
			_tasks.erase (it);
			break;
		}
}	// Process::~Process


Process* Process::create (const string& cmdLine)	// v 6.2.0
{
	istringstream	stream (cmdLine);
	string			exe;
	stream >> exe;
	if (true == stream.fail ( ))
	{
		UTF8String	error (charset);
		error << "Impossibilité d'extraire le nom du processus depuis la ligne de commande \"" << cmdLine << "\".";
		throw Exception (error);
	}	// if (true == stream.fail ( ))
	
	Process*	process	= new Process (exe);
	while (false == stream.eof ( ))
	{
		string	option;
		stream >> option;
		if (true == stream.fail ( ))
		{
			delete process;		process	= 0;
			UTF8String	error (charset);
			error << "Erreur lors de l'extraction d'une option de lancement du processus " << exe << " à partir de la ligne de commande \"" << cmdLine << "\".";
			throw Exception (error);
		}	// if (true == stream.fail ( ))

		process->getOptions ( ).addOption (option);
	}	// while (false == stream.eof ( ))
	
	return process;
}	// Process::create


Process::ProcessOptions& Process::getOptions ( )
{
	return _options;
}	// Process::getOptions


string Process::getCommandLine ( )
{
	UTF8String	cmdLine (charset);
	cmdLine << getPath ( );
	for (size_t o = 0; o < _options.getOptionsNum ( ); o++)
		cmdLine << ' ' << _options.getOption (o);

	return cmdLine;
}	// Process::getCommandLine


bool Process::isCompleted ( ) const
{
	return _completed;
}	// Process::isCompleted


void Process::enableChildToSonCommunications (bool blocking)
{
	_childToSonCommunications	= true;
	if ((blocking == _blockingCommunications) || (true == blocking))
		return;

	_blockingCommunications		= false;
	int*	fifo	= getPipeDescriptors ( );
	int		flags	= fcntl (fifo[0], F_GETFL);
	flags		|= O_NONBLOCK;
	errno	= 0;
	if (0 != fcntl (fifo[0], F_SETFL, flags))
	{
		UTF8String	error (charset);
		error << "Impossibilité d'établir une communication non bloquante avec le processus " << getName ( ) << " : "
		      << strerror (errno);
		throw Exception (error);
	}	// if (0 != fcntl (fifo[0], F_SETFL, flags))
}	// Process::enableChildToSonCommunications


void Process::setCompleted ( )
{
	_completed	= true;
}	// Process::setCompleted


void Process::execute (bool autoDelete)
{
	if (true == isCompleted ( ))
	{
		UTF8String	errorMsg (charset);
		errorMsg << "Impossibilité d'exécuter le processus " << getName ( )  << " : processus terminé.";
		throw Exception (errorMsg);
	}	// if (true == isCompleted ( ))
	setWaited (!autoDelete);
//cout << "==============================================================" << endl;
//for (size_t opt = 0; opt < _options.getOptionsNum ( ); opt++)
//	cout << _options.getOption (opt) << " ";
//cout << endl;

	// Les fonctions exec requierent que le path complet soit transmis. Elles
	// n'utilisent pas la variable envp.
	string		process	= getPath ( );
	File		executable (process);
	if (false == executable.isExecutable ( ))
	{
		UTF8String	message (charset);
		message << "Il est impossible d'exécuter le process " << process << " : absence de droits en exécution.";
		setErrorMessage (message.utf8 ( ).c_str ( ));
		throw Exception (message);
	}	// if (false == executable.isExecutable ( ))

	if (true == isChildToSonCommunicationsEnabled ( ))
	{
		errno	= 0;
		if (-1 == pipe (_fifo))
		{
			UTF8String	message (charset);
			message << "Il est impossible d'ouvrir un pipe de communication "  << " avec le processus fils " << process << " : "
			        << strerror (errno);
			setErrorMessage (strerror (errno));
			errno	= 0;
			throw Exception (message);
		}	// if (-1 == pipe (fifo))
	}	// if (true == isChildToSonCommunicationsEnabled ( ))

	errno		= 0;
	_processId	= fork ( );

	if ((pid_t)-1 == _processId)
	{
		UTF8String	message (charset);
		message << "Il est impossible d'exécuter le process " << getName ( ) << " : " << strerror (errno) << ".";
		setErrorMessage (strerror (errno));
		errno	= 0;
		throw Exception (message);
	}	// if ((pid_t)-1 == _processId)

	if (0 != _processId)		// Process parent.
	{
		// Eviter une autodestruction avant la fin de execute cote pere
		// (processus tres court, qui ne s'execute pas, ...) :
		setWaited (true);
		sigset_t			sigmask;
		struct sigaction	action, oldaction;

		sigemptyset (&sigmask);
		errno	= 0;

		sigemptyset (&action.sa_mask);
		action.sa_sigaction	= &(Process::childDeath);
		action.sa_flags		= SA_RESTART | SA_SIGINFO;

		if (-1 == sigaction (SIGCHLD, &action, &oldaction))
		{
			UTF8String	message (charset);
			message << "Il est impossible d'exécuter le process "  << getName ( ) << " : " << strerror (errno) << ".";
			setErrorMessage (strerror (errno));
			errno	= 0;
			setWaited (!autoDelete);
			throw Exception (message);
		}	// if (-1 == sigaction (SIGCHLD, &action, &oldaction))

		if (true == isChildToSonCommunicationsEnabled ( ))
			close (_fifo [1]);

		setWaited (!autoDelete);
		return;
	}	// if (0 != _processId)			Fin process parent

	if (true == isChildToSonCommunicationsEnabled ( ))
	{
		close (_fifo [0]);
		if (-1 == dup2 (_fifo [1], STDOUT_FILENO))
			ConsoleOutput::cout ( ) << "Processus " << getName ( )  << " : échec de la redirection de la sortie standard vers "
			     << "son processus parent." << co_endl;
	}	// if (true == isChildToSonCommunicationsEnabled ( ))

	char**	argv	= new char* [_options.getOptionsNum ( ) + 2];
	argv [_options.getOptionsNum ( ) + 1]	= NULL;
	argv [0]	= 0 == process.c_str ( ) ? 
				       NULL : strdup (process.c_str ( ));
	for (size_t o = 0; o < _options.getOptionsNum ( ); o++)
		argv [o + 1]	= 0 == _options.getOption (o).c_str ( ) ?
						  NULL : strdup (_options.getOption (o).c_str ( ));
	errno	= 0;
	int	result	= execve (process.c_str ( ), argv, _envp);
	if (-1 == result)
	{
		ConsoleOutput::cerr ( ) << "Il est impossible d'exécuter le process " 
		     << getName ( ) << " : " << strerror (errno) << ".\a" << co_endl;
		errno	= 0;
	}	// if (-1 == result)
	for (size_t i = 0; i < _options.getOptionsNum ( ) + 1; i++)
		if (0 != argv [i])
			free (argv [i]);
	delete [] argv;

	exit (result);
}	// Process::execute


int Process::wait ( )
{
	if (true == isCompleted ( ))
	{
		UTF8String	errorMsg (charset);
		errorMsg << "Impossibilité d'attendre la fin du processus " << getName ( ) << " : processus terminé.";
		throw Exception (errorMsg);
	}	// if (true == isCompleted ( ))
	setWaited (true);
	pid_t	childPid	= getPid ( );
	int		status		= -1;

	// WUNTRACED : appel bloquant
	errno	= 0;
	pid_t	pid	= waitpid (childPid, &status, WUNTRACED);
	if ((pid_t)-1 != pid)
	{	
		if (true == isChildToSonCommunicationsEnabled ( ))
			fillChildBuffer ( );	// v 2.19.1
		if (0 != WIFEXITED (status))	// Terminaison normale
		{
			setCompleted ( );
			setCompletionCode (WEXITSTATUS (status));
		}
		else if (0 != WIFSIGNALED (status))	// => signal	
		{
			setCompleted ( );
			setCompletionCode (WTERMSIG (status));
			setErrorMessage ("SIGNAL");
		}
		else if (0 !=WIFSTOPPED (status))	// => stoppé
			return wait ( );	
	}	// if ((pid_t)-1 != pid)
	errno	= 0;

	return getCompletionCode ( );
}	// Process::wait


void Process::kill ( )
{
	if (true == isCompleted ( ))
	{
		UTF8String	errorMsg (charset);
		errorMsg << "Impossibilité de tuer le processus  " << getName ( ) << " : processus terminé.";
		throw Exception (errorMsg);
	}	// if (true == isCompleted ( ))
	setCompletionCode (SIGKILL);
	setErrorMessage ("SIGNAL");
	::kill (getPid ( ), SIGKILL);
}	// Process::kill


string Process::getPath ( )
{
	string	name		= getName ( );
	string	processName	= _processName;
	File	file (name);
	if (true == file.exists ( ))
		return name;

	// CP v 3.0.1 : dans le cas des versions antérieures le parser s'arrêtait
	// sur la séquence :: et ne testait donc pas les répertoires suivants.
	// coverity : getenv peut retourner NULL.
	// const string	envPath (getenv ("PATH"));
	const char*		path	= getenv ("PATH");
	const string	envPath (NULL == path ? "" : path);
//	size_t			pos	= envPath.find (':');
//	string			tmp	= envPath.substr (0, string::npos == pos ? 0 : pos);
	char*	ptr		= (char*)envPath.c_str ( );			// v 3.0.1
	char*	saveptr	= (char*)envPath.c_str ( );			// v 3.0.1
	char*	dir		= strtok_r (ptr, ":", &saveptr);	// v 3.0.1
	while (NULL != dir)
//	while (0 != tmp.length ( ))
	{
//		file	= tmp + string ("/") + name;
		file	= string (dir) + string ("/") + name;	// v 3.0.1
		try
		{
			// Version 2.6.2 : try/catch pour des cas tordus. Le PATH contient
			// des répertoires que l'on ne peut pas explorer ... file.exists
			// lève alors une exception, et ce serait dommage de s'arrêter là.
			if (true == file.exists ( ))
				return file.getFullFileName ( );
		}
		catch (...)
		{
		}
		dir	= strtok_r (NULL, ":", &saveptr);	// v 3.0.1
/*		size_t	newPos	= envPath.find (':', pos + 1);
		tmp				= envPath.substr (pos + 1, newPos - pos - 1);
		pos				= newPos;
		if (pos >= envPath.length ( ))
			break;
*/
	}	// while (0 != tmp.length ( ))

	UTF8String	message (charset);
	message << "Il est impossible d'exécuter le process " << processName
	        << " : il n'existe pas ou son répertoire ne figure pas dans la "
	        << "variable d'environnement PATH.";
	setErrorMessage (message.utf8 ( ).c_str ( ));
	throw Exception (message);
}	// Process::getPath


void Process::initialize (int argc, char* argv [], char* envp [])
{
	_argc	= argc;
	_argv	= argv;
	_envp	= envp;
}	// Process::initialize


void Process::initialize (char* envp [])
{
	_envp	= envp;
}	// Process::initialize


string Process::getChildLine ( )
{
	if ((true == isCompleted ( )) && (0 == _childBuffer.length ( )))
	{	// v 2.21.0 : le remplissage du buffer n'est plus effectué dans
		// childDeath (accès concurrents depuis la même pile aux fonctions de
		// la lib malloc).
		try
		{
			fillChildBuffer ( );
		}
		catch (...)
		{
		}
	}	// if ((true == isCompleted ( )) && (0 == _childBuffer.length ( )))

	string	strBuffer	= _childBuffer;
	string	processName	= getName ( );
	int*	fifo		= getPipeDescriptors ( );
	if (false == isChildToSonCommunicationsEnabled ( ))
	{
		UTF8String	errorMsg (charset);
		errorMsg << "Impossibilité de lire une ligne de la sortie standard du processus " << processName
		         << " : communication non établie.";
		throw Exception (errorMsg);
	}	// if (false == isChildToSonCommunicationsEnabled ( ))

	// Le tampon de lecture contient il une ligne ?
	string::size_type	pos	= strBuffer.find ('\n');
	if (string::npos != pos)
	{
		string	line	= strBuffer.substr (0, pos);
		if (pos + 1 <= strBuffer.size ( ))
			_childBuffer	= strBuffer.substr (pos + 1);
		return line;
	}	// if (string::npos != pos)

	// Ne contient pas de ligne : effectuer une lecture :
	char	buffer [TKUTIL_PIPE_BUFFER_SIZE + 1];
	memset (buffer, '\0', TKUTIL_PIPE_BUFFER_SIZE + 1);
	errno	= 0;
	ssize_t	count	= read (fifo[0], buffer, TKUTIL_PIPE_BUFFER_SIZE);
	// Version 2.19.1 : levée d'exception si nécessaire.
	// La grande difficulté consiste à savoir pourquoi count est nul : fin de
	// fichier, ou pipe fermé ??? Nous sommes ici en lecture non bloquante
	// (O_NONBLOCK), donc on peut être en fin de fichier et avoir count nul.
	// Dans ce cas, d'après les pages de manuel (man -s 2 read), -1 doit être
	// retourné et errno doit recevoir EAGAIN. Ce cas de figure n'est jamais
	// observé sur station RHEL_5__x86_64 (errno toujours nul ...).
	// On teste alors fcntl, mais, lors de ces tests, le pipe est toujours vidé
	// dans son intégralité ...
	if (count <= 0)
	{
		if (EAGAIN == errno)
			return "";

		errno	= 0;
		int*	fifo	= getPipeDescriptors ( );
		int		flags	= fcntl (fifo[0], F_GETFL);
		if (- 1 == flags)
		{
			UTF8String	errorMsg (charset);
			errorMsg << "Impossibilité de lire une ligne de la sortie standard du processus " << getName ( )
			         << " : processus terminé.";
			throw Exception (errorMsg);
		}	// if (- 1 == flags)

		if (true == isCompleted ( ))
		{
			UTF8String	errorMsg (charset);
			errorMsg << "Impossibilité de lire une ligne de la sortie standard du processus " << getName ( )
			         << " : processus terminé.";
			throw Exception (errorMsg);
		}	// if (- 1 == flags)

		return "";
	}	// if (count <= 0)

	_childBuffer	+= buffer;
	return getChildLine ( );
}	// Process::getChildLine


void Process::setErrorMessage (const char* message)	// v 4.4.0
{
	if (0 != message)
		strncpy (_errorMessage, message, PROCESS_ERROR_MESSAGE_SIZE - 1);
	else
		memset (_errorMessage, '\0', PROCESS_ERROR_MESSAGE_SIZE * sizeof (char));
}	// Process::setErrorMessage


void Process::fillChildBuffer ( )
{
	if (false == isChildToSonCommunicationsEnabled ( ))
		return;

	int		count		= -1;

	int*	fifo		= getPipeDescriptors ( );
	bool	done	= false;
	while (false == done)
	{
		static	char	buffer [TKUTIL_PIPE_BUFFER_SIZE + 1];
		memset (buffer, '\0', TKUTIL_PIPE_BUFFER_SIZE + 1);
		errno	= 0;
		count	= read (fifo[0], buffer, TKUTIL_PIPE_BUFFER_SIZE);
		if (count > 0)
		{
			_childBuffer	+= buffer;
			continue;
		}

		if ((count <= 0) && (EAGAIN != errno))
			done	= true;
	}	// while (0 != count)
}	// Process::fillChildBuffer


void Process::appendChildBuffer (const string& str, bool newLine)
{
	if (false == isChildToSonCommunicationsEnabled ( ))
		return;

	_childBuffer	+= str;
	if (true == newLine)
		_childBuffer	+= '\n';
}	// Process::appendChildBuffer


string Process::getCurrentDirectory ( )
{
	char	dirName [FILENAME_MAX + 1];
	if (NULL == getcwd (dirName, FILENAME_MAX))
		return "";

	return string (dirName);
}	// Process::getCurrentDirectory


File Process::getCurrentExecutable ( )
{	
	if (0 >= _argc)
		throw Exception (UTF8String ("Process::getCurrentExecutable : API non initialisée.", charset));

	unique_ptr <Process>	process	(new Process (_argv [0]));
	return File (process->getPath ( ));
}	// Process::getCurrentExecutable


void Process::setCurrentSoftware (const string& name, const Version& version)
{
	_currentSoftware		= name;
	_currentSoftwareVersion	= version;
}	// Process::setCurrentSoftware


const string& Process::getCurrentSoftware ( )
{
	return _currentSoftware;
}	// Process::getCurrentSoftware


const Version& Process::getCurrentSoftwareVersion ( )
{
	return _currentSoftwareVersion;
}	// Process::getCurrentSoftwareVersion


void Process::childDeath (int sigNum, siginfo_t*, void*)
{
	if (SIGCHLD != sigNum)
		return;

	int	status = 0;

	Process*	toDelete	= 0;	// v 4.4.0
	for (vector<Process*>::iterator it = _tasks.begin ( );
	     _tasks.end ( ) != it; it++)
	{
		if ((pid_t)-1 == (*it)->getPid ( ))
			continue;

		// WNOHANG : appel non bloquant
		errno	= 0;
		pid_t	pid	= waitpid ((*it)->getPid ( ), &status, WNOHANG);
		if ((*it)->getPid ( ) == pid)
		{
// v 2.21.0 : on ne lit plus les sorties du processus fils depuis childDeath
// car on a alors des appels concurrents depuis la même pile à la libmalloc
// entre ces lectures et la fonction en cours d'exécution. Or les appels à la
// libmalloc sont protégés par des verrous => situation de dead lock.
//			if (true == (*it)->isChildToSonCommunicationsEnabled ( ))
//				(*it)->fillChildBuffer ( );
			if (0 != WIFEXITED (status))	// Terminaison normale
			{
				(*it)->setCompleted ( );
				(*it)->setCompletionCode (WEXITSTATUS (status));
			}
			else if (0 != WIFSIGNALED (status))	// => signal
			{
				(*it)->setCompleted ( );
				(*it)->setCompletionCode (WTERMSIG (status));
				(*it)->setErrorMessage ("SIGNAL");
			}
			else if (0 !=WIFSTOPPED (status))	// => stoppé
				continue;

			if (false == (*it)->isWaited ( ))
				toDelete	= *it;	// v 4.4.0
//				delete (*it);		// v 4.4.0
			break;
		}	// if ((*it)->getPid ( ) == pid)
		errno	= 0;
	}	// for (vector<Process*>::iterator it = _tasks.begin ( ); ...

//	delete toDelete;	// v 4.4.0. childDeath caught by signal, malloc/free
//	should not be called here ...
}	// Process::childDeath


END_NAMESPACE_UTIL
