#include "TkUtil/UTF8String.h"
#include "TkUtil/InformationLog.h"
#include "TkUtil/WarningLog.h"
#include "TkUtil/ErrorLog.h"
#include "TkUtil/ProcessLog.h"
#include "TkUtil/TraceLog.h"
#include "TkUtil/Locale.h"
#include "TkUtil/LogDispatcher.h"
#include "TkUtil/OstreamLogOutputStream.h"
#include "TkUtil/FileLogOutputStream.h"

#include <iostream>
#include <stdexcept>


USING_STD
USING_UTIL

static const Charset	charset ("àéèùô");
static Charset			outCharset (charset);	// v 4.0.0

int main (int argc, char* argv [])
{
	cout << argv [0] << "[-charset][[-charset1] file1 [-charset2] file2 [-charset3] file3 ...]" << endl
	     << "Charsets possibles : ascii iso8859 utf8" << endl
	     << "-àéèùô pour détermination automatique de la sortie standard" << endl
	     << "Exemple : ./logs -utf8 -ascii toto.ascii -iso8859 toto.iso" << endl
	     << "=> sortie standard UTF-8, toto.ascii au format ASCII, "
	     << "toto.iso au format ISO 8859" << endl;

	if (1 < argc)
	{
		if (string ("-ascii") == argv [1])
			outCharset	= Charset (Charset::ASCII);
		else if (string ("-iso8859") == argv [1])
			outCharset	= Charset (Charset::ISO_8859);
		else if (string ("-utf8") == argv [1])
			outCharset	= Charset (Charset::UTF_8);
		else if ('-' == *argv [1])
		{
			outCharset	= Charset (argv [1] + 1);
			cout << "Detection du jeu de caracteres de la sortie standard a "
			     << "partir de la chaine \"" << argv [1] << "\" : "
			     << outCharset.name ( ) << endl;
		}
	}	// if (1 < argc)

	if (Charset::UNKNOWN == outCharset.charset ( ))
		outCharset	= Locale::detectCharset (string (""));
	ConsoleOutput::cout ( ).setCharset (outCharset);

	ConsoleOutput::cout ( ) << "Jeu de caractères utilisé pour la sortie standard : "
	     << outCharset.name ( ) << co_endl;
	LogDispatcher			dispatcher;
	OstreamLogOutputStream*	stdOstream	=
					new OstreamLogOutputStream (cout, Log::PRODUCTION);
	stdOstream->setCharset (outCharset);
	const	bool			enableDate	= false;
	const	bool			enableTime	= false;

	dispatcher.addStream (stdOstream);
	dispatcher.enableDate (enableDate, enableTime);

	for (int i = 1; i < argc; i++)
	{
		if ('-' == *argv [i])
		{
			if (string ("-ascii") == argv [i])
				outCharset	= Charset (Charset::ASCII);
			else if (string ("-iso8859") == argv [i])
				outCharset	= Charset (Charset::ISO_8859);
			else if (string ("-utf8") == argv [i])
				outCharset	= Charset (Charset::UTF_8);

			continue;
		}	// if ('-' == *argv [i])

		UTF8String	info1 (charset);
		info1 << "Ajout du fichier " << argv [i] << " comme fichier de traces "
		      << " avec pour jeu de caractères " << outCharset.name ( );
		TRACE_LOG_1 (trace1, info1)
		dispatcher << trace1;

		FileLogOutputStream*	file	= 0;
		try
		{
			file	= new FileLogOutputStream (
						UTF8String (argv [i], charset), outCharset, true, Log::ALL_KINDS);
			if (true == file->isEnabled ( ))
			{
				dispatcher.addStream (file);
				file->enableDate (enableDate, enableTime);
			}
			else
			{
				UTF8String	err (charset);
				err << "Impossibilité d'utiliser " << argv [i] 
				    << " comme fichier de traces.";
				throw Exception (err);
			}
			UTF8String	info2 (charset);
			info2 << "Fichier " << argv [i]
			      << " ajouté comme fichier de traces.";
			TRACE_LOG_1 (trace2, info2)
			dispatcher << trace2;
		}
		catch (const Exception& exc)
		{
			ErrorLog	error (exc);
			dispatcher << error;
			delete file;
			return -1;
		}
		catch (const std::exception& e)
		{
			ErrorLog	error (e);
			dispatcher << error;
			return -1;
		}
		catch (...)
		{
			UTF8String	errorMsg (charset);
			errorMsg << "Erreur non renseignée lors de la création du fichier "
			         << "de logs " << argv [i] << ".";
			ErrorLog	error (errorMsg);
			dispatcher << error;
			delete file;
			return -1;
		}
	}	// for (int i = 1; i < argc; i++)

	try
	{
		if (1 < argc)
		{
			WarningLog warning (UTF8String ("Les anciens logs des fichiers sont détruits.", charset));
			dispatcher << warning;
		}

		InformationLog	info1 (UTF8String ("L'application est prête à fonctionner.", charset));
		dispatcher << info1;
		WarningLog	warn1 (
			UTF8String ("Exemple de log d'avertissement.", charset));
		dispatcher << warn1;
		ErrorLog	err1 (UTF8String ("Exemple de log d'erreur.", charset));
		dispatcher << err1;
		ProcessLog	proc1 (
			UTF8String ("Exemple de log de type processus.", charset),
			UTF8String ("Sorties du processus", charset));
		dispatcher << proc1;
		TraceLog	trace1 (
			UTF8String ("Exemple de log de type trace.", charset),
		 	Log::TRACE_1, __FILE__, __LINE__);
		dispatcher << trace1;
		// ...
		InformationLog	info2 (UTF8String ("Fin de l'application.", charset));
		dispatcher << info2;
	}
	catch (const Exception& exc)
	{
		ErrorLog	error (exc);
		dispatcher << error;
		return -1;
	}
	catch (const std::exception& e)
	{
		ErrorLog	error (e);
		dispatcher << error;
		return -1;
	}
	catch (...)
	{
		UTF8String	errorMsg (charset);
		errorMsg << "Erreur non renseignée lors l'écriture d'un log.";
		ErrorLog	error (errorMsg);
		dispatcher << error;
		return -1;
	}
	
	return 0;
}	// main

