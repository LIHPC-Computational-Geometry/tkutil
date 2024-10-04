/**
 * Test simple de la classe Process avec récupération de la sortie standard du processus détaché.
 */
#include "TkUtil/Exception.h"
#include "TkUtil/Process.h"

#include <iostream>
#include <memory>


USING_STD
USING_UTIL



int main (int argc, char* argv [], char* envp[])
{
	if (2 != argc)
	{
		cout << "Syntax : " << argv [0] << " directory" << endl;
		return -1;
	}	// if (2 != argc))

	try
	{
		Process::initialize (envp);

		unique_ptr<Process>	process (new Process ("ls"));
		process->getOptions ( ).addOption ("-al");
		process->getOptions ( ).addOption (argv [1]);
		cout << "Cmd line is : " << process->getCommandLine ( ) << endl;
		process->enableChildToSonCommunications (true);
		process->execute (false);
		const int	status	= process->wait ( );
		cout << "Process standard output :" << endl;
		bool	completed	= false;
		while (false == completed)
		{
			try
			{
				const string	output	= process->getChildLine ( );
				cout << output << endl;
			}
			catch (...)
			{
				completed	= true;
			}
		}	// while (false == completed)
		if (0 != status)
			cout << "Process failed : " << process->getErrorMessage ( ) << endl;

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
