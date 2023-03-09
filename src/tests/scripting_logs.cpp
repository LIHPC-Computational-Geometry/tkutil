#include "TkUtil/ScriptingLog.h"
#include "TkUtil/ErrorLog.h"
#include "TkUtil/OstreamLogOutputStream.h"
#include "TkUtil/Exception.h"

#include <iostream>
#include <stdexcept>
#include <limits>


USING_STD
USING_UTIL

static const Charset	charset ("àéèùô");

int main (int argc, char* argv [])
{
	OstreamLogOutputStream*	stdOstream	=
					new OstreamLogOutputStream (cout, Log::PRODUCTION);

	try
	{	
		string			str1 ("une chaine de caracteres");
		bool			bool1	= true;
		long			long1	= -12;
//		double			double1	= 23.456e-7;
		double			double1	= 23.456e3;
		ScriptingLog	log1 ("abc.def", "simpleMethod", "", UTF8String ("Cette méthode prend en arguments une chaine, un booléen, un entier et un double.", charset));
//		log1 << string ("une chaine de caracteres") << true << -12 << 23.456e-7;
		log1 << str1 << bool1 << long1 << double1;
		*stdOstream << log1;
		delete stdOstream;
//cout << "MANTISSE : " << numeric_limits<double>::digits << endl;
//cout << "MANTISSE : " << numeric_limits<float>::digits << endl;
	}
	catch (const Exception& exc)
	{
		ErrorLog	error (exc);
		*stdOstream << error;
		delete stdOstream;
		return -1;
	}
	catch (const std::exception& e)
	{
		ErrorLog	error (e);
		*stdOstream << error;
		delete stdOstream;
		return -1;
	}
	catch (...)
	{
		UTF8String	errorMsg (charset);
		errorMsg << "Erreur non renseignée.";
		ErrorLog	error (errorMsg);
		*stdOstream << error;
		delete stdOstream;
		return -1;
	}

	return 0;
}	// main

