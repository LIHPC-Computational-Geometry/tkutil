#include "TkUtil/TemporaryFile.h"
#include "TkUtil/Exception.h"
#include "TkUtil/UTF8String.h"

#include <iostream>
#include <fstream>
#include <stdexcept>


USING_STD
USING_UTIL

static const Charset	charset ("àéèùô");


int main (int argc, char* argv [])
{
	cout << argc << endl;
	if (3 != argc && 4 != argc)
	{
		cout << "Syntax : " << argv [0] << " tmpDirPrefix filePrefix [-r remove file without keystroke]" << endl;
		return -1;
	}	// if (3 != argc || 4 != argc)

	try
	{
		cout 
		<< "=============================================================="
		     << endl;
		cout << "Creation of temporary directory with prefix "
		     << argv [1] << endl;
		TemporaryFile	tmpDir (false, argv [1], true, true);
		cout << "Temporary directory created :" << endl << tmpDir;
		cout 
		<< "=============================================================="
		     << endl;

		cout << "Creation of temporary file with prefix " << argv [2] << endl;
		UTF8String	prefix (charset);
		prefix << tmpDir.getFullFileName ( ) << "/" << argv [2];
		TemporaryFile	tmpFile (true, prefix.iso ( ), false, true);
		cout << "Temporary file created :" << endl << tmpFile;
		cout 
		<< "=============================================================="
		     << endl;

		ofstream	ofs (tmpFile.getFullFileName ( ).c_str ( ));
		ofs << "Ceci est un test." << endl;
		ofs.close ( );

		if (argc == 3) { 
			cout << "Enter a key in order to remove temporary file and directory."
				<< endl;
			char	c;
			cin >> c;
		}
		cout << "Directory " << tmpDir.getFullFileName ( ) << " and file "
		     << tmpFile.getFullFileName ( ) << " will be removed." << endl;
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
