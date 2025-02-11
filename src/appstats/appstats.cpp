#include <TkUtil/ApplicationStats.h>
#include <TkUtil/Date.h>
#include <TkUtil/Exception.h>
#include <TkUtil/NumericConversions.h>

#include <iostream>

using namespace TkUtil;
using namespace std;


static bool parseArgs (int argc, char* argv []);
static int syntax (int argc, char* argv []);

static string	appName, logDir, from, to;


int main (int argc, char* argv [])
{
	try
	{
		if (false == parseArgs (argc, argv))
			return syntax (argc, argv);

cout << "APP=" << appName << " DIR=" << logDir << " FROM=" << from << " TO=" << to << endl;
		ApplicationStats::logStats (cout, appName, from, to, logDir);
	}
	catch (const Exception& exc)
	{
		cerr << "Erreur : " << exc.getFullMessage ( ) << endl;
		return syntax (argc, argv);
	}
	catch (...)
	{
		cerr << "Erreur non documentée." << endl;
		return syntax (argc, argv);
	}
	
	return 0;
}	// main


static bool parseArgs (int argc, char* argv [])
{
	for (int i = 1; i < argc; i++)
	{
		bool	kept	= false;

		if (i < argc)
		{
			if (string ("-name") == argv [i])
			{
				appName	= argv [++i];
				kept	= true;
			}	// if (string ("-name") == argv [i])
			else if (string ("-dir") == argv [i])
			{
				logDir	= argv [++i];
				kept	= true;
			}	// if (string ("-dir") == argv [i])
			else if (string ("-from") == argv [i])
			{
				from	= argv [++i];
				kept	= true;
			}	// if (string ("-from") == argv [i])
			else if (string ("-to") == argv [i])
			{
				to		= argv [++i];
				kept	= true;
			}	// if (string ("-to") == argv [i])

			if (false == kept)
			{
				UTF8String	message (Charset::UTF_8);
				message << "Argument non traité : " << argv [i];
				throw Exception (message);
			}	// if (false == kept)
		}	// if (i < argc)
	}	// for (int i = 0; i < argc; i++)
	if ((true == appName.empty ( )) || (true == logDir.empty ( )))
		return false;

	Date	now;
	if (true == from.empty ( ))
		from	= NumericConversions::toStr ((size_t)now.getMonth ( ), 2) + NumericConversions::toStr ((size_t)(now.getYear ( )), 4);	// Hum hum
	if (true == to.empty ( ))
		to	= NumericConversions::toStr ((size_t)now.getMonth ( ), 2) + NumericConversions::toStr ((size_t)(now.getYear ( )), 4);	// Hum hum
	
	return true;
}	// parseArgs


static int syntax (int argc, char* argv [])
{
	cerr << argv [0] << "-name appName -dir logDir [-from date][-to date]" << endl
	                 << "où :" << endl
	                 << "appName est le nom de l'application," << endl
	                 << "logDir est le répertoire où sont stockés les fichiers de logs de l'application" << endl
	                 << "date est une date au format MMYYYY, sindiquant les premiers logs à prendre en compte après -from," << endl
	                 << "et les derniers logs à prendre en compte après -to." << endl;

	return -1;
}	// syntax
