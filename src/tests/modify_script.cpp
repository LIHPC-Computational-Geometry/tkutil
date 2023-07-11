//
// Lit le fichier transmis en premier argument et le recopie dans le fichier 
// dont le nom est transmis en second argument, en ajoutant _MOD à chaque mot
// clé transmis en argument suivant.
//
#include "TkUtil/ScriptingTools.h"
#include "TkUtil/Exception.h"

#include <fstream>


USING_STD
USING_UTIL


int main (int argc, char* argv [])
{
	if (4 > argc)
	{
		cerr << argv [0] << " : erreur de syntaxe." << endl
		     << argv [0] << "script_entrée script_sortie mot1 [mot2] ... [motn]"
		     << endl;
		return -1;
	}	// if (4 > argc)

	try
	{	
		ifstream	in (argv [1]);
		ofstream	out (argv [2]);
		vector < pair <string, string> >	words;
		cout << "Recopie du fichier " << argv [1] << " dans le fichier "
		     << argv [2] << " avec les remplacements suivants : " << endl;
		for (int i = 3; i < argc; i++)
		{
			const string	word	= string (argv [i]) + "_MOD";
			cout << argv [i] << " par " << word << endl;
			words.push_back (pair <string, string> (argv [i], word));
		}	// for (int i = 3; i < argc; i++)

		ScriptingTools::replace (in, out, words);
		cout << "Traitement effectué." << endl;
	}
	catch (const Exception& exc)
	{
		cout << "Erreur : " << exc.getFullMessage ( ) << endl;
	}
	catch (...)
	{
		cout << "Erreur non documentée." << endl;
	}

	return 0;
}	// main

