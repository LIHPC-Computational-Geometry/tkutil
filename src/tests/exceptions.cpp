#include "TkUtil/AnsiEscapeCodes.h"
#include "TkUtil/InternalError.h"

#include <iostream>


using namespace std;
using namespace TkUtil;


static void A (bool stdexc)
{
	if (true == stdexc)
	{
		throw runtime_error ("Runtime error in A ( ).");
	}

	throw Exception ("Error during A ( ) call.");
}	// A


static void B (bool stdexc)
{
	try
	{
		A (stdexc);
	}
	catch (const Exception& exc)
	{
		INTERNAL_EXCEPTION (e, "Erreur de la fonction B", exc);
		throw e;
	}
	catch (const exception& exc)
	{
		INTERNAL_EXCEPTION (e, "Erreur de la fonction B", exc);
		throw e;
	}
}	// B


static void C (bool stdexc)
{
	try
	{
		B (stdexc);
	}
	catch (const Exception& exc)
	{
		throw Exception ("Erreur de la fonction C", exc);
	}
}	// C ( )


static void D (bool stdexc)
{
	try
	{
		C (stdexc);
	}
	catch (const Exception& exc)
	{
		throw Exception ("Erreur de la fonction D", exc);
	}
}	// D ( )


int checkAndReport (bool stdexc)
{
	cout << AnsiEscapeCodes::blueFg;
	try
	{
		cout << "Test de levée d'une exception "
		    << (true == stdexc ? "standard" : "tkutil") << endl;
		D (stdexc);
		cout << "Absence d'erreur lors de la levée d'exception "
		     << "=> il y a probablement eu un problème !!!" << endl;
		return -1;
	}
	catch (const Exception& exc)
	{
		cerr << AnsiEscapeCodes::redFg
		     << "Erreur : " << exc.getFullMessage ( ) << endl;
	}
	catch (...)
	{
		cerr << AnsiEscapeCodes::redFg << "Erreur non documentée." << endl;
	}
	cout << AnsiEscapeCodes::reset;
	return 0;
}	// checkAndReport


int main (int argc, char* argv[])
{
	cout << "Exécution du programme " << argv [0]
	     << " : deux erreurs doivent survenir ..." << endl;
	int rc = checkAndReport (true);
	if (rc == -1)
	{
		return -1;
	}

	rc = checkAndReport (false);
	if (rc == -1)
	{
		return -1;
	}

	cout << "Fin normale du programme " << argv [0] << endl;
	return 0;
}	// main
