#include "TkUtil/Exception.h"
#include "TkUtil/RemoteProcess.h"

#include <iostream>
#include <stdexcept>
#include <wait.h>
#include <errno.h>


USING_STD
USING_UTIL


static string getRemoteLauncherPath (const string& hostName);
static string getRemoteOsType (const string& hostName);


int main (int argc, char* argv [], char* envp[])
{
	if (3 > argc)
	{
		cout << "Syntax : " << argv [0] << " host process option1 ... optionn" 
		     << endl;
		return -1;
	}	// if (3 > argc))

	try
	{
		Process::initialize (envp);

		// Le lanceur d'applications sur des machines distantes :
		string	launcher	= getRemoteLauncherPath	(argv [1]);
		RemoteProcess::launcher	= launcher;

		// Terminaison prematuree du processus ?
		RemoteProcess*	process	= new RemoteProcess (argv [1], argv [2]);
		for (int i = 3; i < argc; i++)
			process->getOptions ( ).addOption (argv [i]);
		process->setServerWaitingMessage ("OCEANE is waiting for requests ...");
		cout << "Launching process " << argv [2] << " on " << argv [1] << " ..."
		     << endl;
		cout << "Cmd line is : " << process->getCommandLine ( ) << endl;
		process->execute (false);
		pid_t	pid	= (pid_t)-1;
		pid	= process->getRemotePid ( );
		string	path;
		path	= process->getRemotePath ( );
		cout << "Process " << argv [2] << " successfully ran on " 
		     << process->getHostName ( ) << ". PID = " << pid 
		     << " Remote path = " << path << endl;

		// Attente de la terminaison :
		int		status	= process->wait ( );
/*
		sleep (5);
		process->kill ( );
		int	status	= process->getCompletionCode ( );
*/
		string	errMessage;
		errMessage	= process->getErrorMessage ( );
		delete process;	process = 0;
		if (0 != status)
		{
			string	message	= string (argv [0]) +
						+ ". Erreur inattendue lors de la terminaison de "
						+ argv [2] + " sur " + argv [1] + " : " 
						+ errMessage;
			throw Exception (UTF8String (message, UTF8String::defaultCharset));
		}	// if ((pid_t)-1 == pid)

		return status;
	}
	catch (const Exception& exc)
	{
		cout << "Exception caught : " << exc.getFullMessage ( ) << endl;
		return -1;
	}
	catch (const exception& e)
	{
		cout << "Standard exception caught : " << e.what ( ) << endl;
		return -1;
	}
	catch (...)
	{
		cout << "Undocumented exception caught." << endl;
		return -1;
	}

	return 0;
}	// main


static string getRemoteLauncherPath (const string& hostName)
{
/*
	string	osType	= getRemoteOsType (hostName);

	UTF8String	path (charset);
	path << "/bin/launcher";

	return path.iso ( );
*/
	return "TkUtilLauncher";
}	// getRemoteLauncherPath


static string getRemoteOsType (const string& hostName)
{
	Process*	process	= new Process ("rsh");
	process->getOptions ( ).addOption (hostName);
	process->enableChildToSonCommunications ( );
	process->execute (false);
	string	osType;
	while (0 == osType.length ( ))
		osType	= process->getChildLine ( );
	// Preferable : eviter que 2 process arrivent a terminaison en meme temps :
	process->wait ( );

	return osType;
}	// getRemoteOsType


