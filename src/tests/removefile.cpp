#include "TkUtil/File.h"
#include "TkUtil/Exception.h"

#include <iostream>
#include <stdexcept>


USING_STD
USING_UTIL

static const Charset	charset ("àéèùô");


int main (int argc, char* argv [])
{
	if (1 == argc)
	{
		cout << "Syntax : " << argv [0] << " file1" << endl;
		cout << "Removes given file or directory." << endl;
		return -1;
	}	// if (1 == argc)

	try
	{
		File	file (argv [1]);

		cout 
			<< "=============================================================="
		    << endl;
		cout << file;
		cout 
			<< "=============================================================="
		    << endl;

		if (true == file.exists ( ))
		{
			cout << "Removing " << file.getFileName ( ) << " ..." << endl;
			file.remove ( );
			cout << file.getFileName ( ) << " removed." << endl;
		}
		else
		{
			UTF8String	message (charset);
			message << "File " << file.getFileName ( ) << " does not exist.";

			throw Exception (message);
		}
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
