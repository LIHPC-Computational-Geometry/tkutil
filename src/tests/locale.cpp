#include "TkUtil/Locale.h"
#include "TkUtil/Exception.h"

#include <iostream>


USING_UTIL
USING_STD


int main (int, char*[])
{
#if defined (__linux__)
	const string	frIso ("fr_FR.iso88591");		// Linux
#elif defined (__sparc)
	const string	frIso ("fr_FR.ISO8859-1");		// Sun
#elif defined (__osf__)
	const string	frIso ("fr_FR.ISO8859-1");
#elif defined (__sgi)
	const string	frIso ("fr");		// Sun
#else
#	error	"OS non specifie"
#endif
	const string	enGB ("en_GB");
	try
	{
		cout << "VALEURS PAR DEFAUT AU LANCEMENT DU PROGRAMME :" << endl;
		cout << "Locale du programme :" << endl
			<< Locale::toStr (Locale::EXE) << endl
			<< "Locale de l'environnement :" << endl
			<< Locale::toStr (Locale::USER_ENV) << endl
			<< "Locale programme + environnement :" << endl
			<< Locale::toStr (Locale::BOTH) << endl;

		cout << endl
			<< "CHANGEMENT DE LOCALE LC_CTYPE : AFFECTATION DE " << frIso << " :"
			<< endl;
		Locale::setLocale (Locale::C_TYPE, frIso);
		cout << "Locale du programme :" << endl
			<< Locale::toStr (Locale::EXE) << endl
			<< "Locale de l'environnement :" << endl
			<< Locale::toStr (Locale::USER_ENV) << endl
			<< "Locale programme + environnement :" << endl
			<< Locale::toStr (Locale::BOTH) << endl;

		cout << endl
			<< "CHANGEMENT DE LOCALE LC_ALL : AFFECTATION DE " << frIso << " :"
			<< endl;
		Locale::setLocale (Locale::ALL, frIso);
		cout << "Locale du programme :" << endl
			<< Locale::toStr (Locale::EXE) << endl
			<< "Locale de l'environnement :" << endl
			<< Locale::toStr (Locale::USER_ENV) << endl
			<< "Locale programme + environnement :" << endl
			<< Locale::toStr (Locale::BOTH) << endl;

		cout << endl
			<< "CHANGEMENT DE SEPARATEUR NUMERIQUE : AFFECTATION DE \"C\" A LC_NUMERIC :"
			<< endl;
		Locale::setLocale (Locale::NUMERIC, "C");
		cout << "Locale du programme :" << endl
			<< Locale::toStr (Locale::EXE) << endl
			<< "Locale de l'environnement :" << endl
			<< Locale::toStr (Locale::USER_ENV) << endl
			<< "Locale programme + environnement :" << endl
			<< Locale::toStr (Locale::BOTH) << endl;

		cout << endl << "Valeurs possibles : ";
		vector<string>	locales	= Locale::getAvailableLocales ( );
		for (vector<string>::const_iterator it = locales.begin ( );
			locales.end ( ) != it; it++)
			cout << *it << " ";
		cout << endl;

		cout << endl
			<< "AFFECTATION AU PROGRAMME DE L'ENVIRONNEMENT UTILISATEUR :"
			<< endl;
		UTF8String	msg	= Locale::setUserLocales ( );
		cout << "OPERATIONS EFFECTUEES : " << endl << msg.iso ( ) << endl;
		cout << "Locale du programme :" << endl
			<< Locale::toStr (Locale::EXE) << endl
			<< "Locale de l'environnement :" << endl
			<< Locale::toStr (Locale::USER_ENV) << endl
			<< "Locale programme + environnement :" << endl
			<< Locale::toStr (Locale::BOTH) << endl;
	}
	catch (const Exception& exc)
	{
		cout << "Exception levée : " << exc.getFullMessage ( ) << endl;
		return -1;
	}
	catch (...)
	{
		cout << "Exception inattendue." << endl;
		return -1;
	}

	return 0;
}	// main
