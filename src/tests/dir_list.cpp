/**
 * Test simple de la classe File pour avoir la composition d'un répertoire.
 */
#include "TkUtil/Exception.h"
#include "TkUtil/File.h"

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
		File			dir (argv [1]);
		vector<File>	directories, files;
		dir.getChildren (directories, files);
		
		cout << dir.getFullFileName ( ) << "directories : " << endl;
		for (vector<File>::const_iterator	itd = directories.begin ( ); directories.end ( ) != itd; itd++)
			cout << (*itd).getFileName ( ) << ' ';
		cout << endl;
		cout << dir.getFullFileName ( ) << "files : " << endl;
		for (vector<File>::const_iterator	itf = files.begin ( ); files.end ( ) != itf; itf++)
			cout << (*itf).getFileName ( ) << ' ';
		cout << endl;
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
