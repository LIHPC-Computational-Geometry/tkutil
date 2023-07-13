/*
 * Petit programme test visant à afficher des doubles avec la précision
 * souhaitée dans la représentation la plus courte possible et avec notation
 * scientifique.
 */
#include <TkUtil/Exception.h>
#include <TkUtil/NumericConversions.h>

#include <iostream>
#include <string>
#include <vector>


USING_STD
USING_UTIL


static unsigned short	precision		= 6;
static unsigned short	integerPartNum	= 1;
static vector<string>		toConvert;


bool parseArgs (int argc, char* argv[])
{
	for (int i = 1; i < argc; i++)
	{
		if ((string ("-precision") == argv [i]) && (i < argc - 1))
			precision	= NumericConversions::strToULong (argv [++i]);
		else if ((string ("-integerPartNum") == argv [i]) && (i < argc - 1))
			integerPartNum	= NumericConversions::strToULong (argv [++i]);
		else
			toConvert.push_back (argv [i]);
	}	// for (int i = 1; i < argc; i++)

	return true;
}	// parseArgs


int syntax (const string& pgm)
{
	cout << pgm << "[-precision precision][-integerPartNum num] d1 d2 ... dn"
	     << endl
	     << "Convertit les réels transmis en arguments en chaines de "
	     << "caractères au format spécifié." << endl
	     << "-precision precision ......... : nombre de décimales." << endl
	     << "-integerPartNum num .......... : nombre de chiffres de la partie entière" << endl
	     << endl;

	return -1;
}	// syntax


int main (int argc, char* argv[])
{
	try
	{

	if (false == parseArgs (argc, argv))
		return syntax (argv [0]);

	cout << "Précision utilisée : " << precision
	     << endl
	     << "Nombre de chiffres de la partie entière : " << integerPartNum
	     << endl;

	for (vector<string>::const_iterator it = toConvert.begin ( );
	     toConvert.end ( ) != it; it++)
	{
		const double	d	= NumericConversions::strToDouble (*it);
		cout << *it << " --> "
		     << NumericConversions::userRepresentation (
												d, precision, integerPartNum)
		     << endl;
	}	// for (vector<string>::const_iterator it = toConvert.begin ( ); ...

	}
	catch (const Exception& exc)
	{
		cerr << "Erreur : " << exc.getFullMessage ( ) << endl;
		return -1;
	}
	catch (const exception& e)
	{
		cerr << "Erreur : " << e.what ( ) << endl;
		return -1;
	}
	catch (...)
	{
		cerr << "Erreur non documentée." << endl;
		return -1;
	}

	return 0;
}	// main
