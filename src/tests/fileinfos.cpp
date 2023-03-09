#include "TkUtil/File.h"
#include "TkUtil/Exception.h"

#include <iostream>
#include <stdexcept>


USING_STD
USING_UTIL


int main (int argc, char* argv [])
{
	if (1 == argc)
	{
		cout << "Syntax : " << argv [0] << " file1 file2 ... filen" << endl;
		return -1;
	}	// if (1 == argc)

	for (int i = 1; i < argc; i++)
	{
		try
		{
			File	file (argv [i]);

			cout << "=============================================================="
			     << endl;
			cout << file;
			cout << "=============================================================="
			     << endl;
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
	}	// for (int i = 1; i < argc; i++)

	return 0;
}	// main
