#include "TkUtil/Exception.h"
#include "TkUtil/UrlFifo.h"

#include <iostream>
#include <string>
#include <string.h>

using namespace std;
using namespace TkUtil;


static bool parseArgs (int argc, char* argv []);
static int syntax (const string& pgm);

static string	inFile, outFile;


int main (int argc, char* argv [])
{
	if (false == parseArgs (argc, argv))
		return syntax (argv [0]);

	try
	{
		UrlFifo	urls;
		if (false == inFile.empty ( ))
		{
			cout << "Loading URLs from " << inFile << " ..." << endl;
			urls.load (inFile);
			cout << "URLs loaded." << endl;
		}	// if (false == inFile.empty ( ))

		const string	newUrl ("/usr/urls/toto.html");
		cout << "Adding URL " << newUrl << "..." << endl;
		urls.add (newUrl, true);
		cout << "URL added." << endl;

		if (false == outFile.empty ( ))
		{
			cout << "Writing URLs to " << outFile << " ..." << endl;
			urls.save (outFile);
			cout << "URLs wrote." << endl;
		}	// if (false == outFile.empty ( ))
	}
	catch (const Exception& exc)
	{
		cout << "Exception caught : " << exc.getFullMessage ( ) << endl;
	}
	catch (...)
	{
		cout << "Unexpected exception caught." << endl;
	}

	return 0;
}	// main


static bool parseArgs (int argc, char* argv [])
{
	try
	{
		for (int i = 1; i < argc - 1; i++)
		{
			if (0 == strcmp (argv [i], "-in"))
				inFile	= argv [++i];
			else if (0 == strcmp (argv [i], "-out"))
				outFile	= argv [++i];
		}	// for (int i = 1; i < argc - 1; i++)
	}
	catch (const Exception& e)
	{
		cerr << e.getFullMessage ( ) << endl;
		return false;
	}
	catch (...)
	{
		cerr << "Erreur non documentée." << endl;
		return false;
	}

	return true;
}	// parseArgs


static int syntax (const string& pgm)
{
	cout << pgm << " [-in urlsFile][-out urlsFile]" << endl;

	return -1;
}	// syntax



