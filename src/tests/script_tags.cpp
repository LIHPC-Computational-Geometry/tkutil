//
// Recherche des couples (balise, valeur) dans un script. Ces couples sont
// précédés de balises les annonçant.
//
#include "TkUtil/ScriptingTools.h"
#include "TkUtil/Exception.h"

#include <fstream>


USING_STD
USING_UTIL


int main (int argc, char* argv [])
{
	if (3 != argc)
	{
		cerr << argv [0] << " : erreur de syntaxe." << endl
		     << argv [0] << "script_entrée balise"
		     << endl;
		return -1;
	}	// if (3 != argc)

	try
	{
		cout << "Recherche des couples (balise, valeur) dans le fichier "
		     << argv [1] << ". Ces couples sont annoncés par la balise "
		     << argv [2] << "." << endl;
		ifstream	in (argv [1]);
		vector <TaggedValue>	taggedValues	= ScriptingTools::findTags (
							in, argv [2], ScriptingTools::pythonCommentLine);
		cout << "Valeurs balisées trouvées : " << endl;
		for (vector <TaggedValue>::const_iterator
			it = taggedValues.begin ( ); taggedValues.end ( ) != it; it++)
			cout << (*it).toString ( ) << endl;
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

