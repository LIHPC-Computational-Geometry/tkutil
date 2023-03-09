//
// Extrait d'un fichier script python des fichiers insérés sous forme de
// commentaires.
//
#include "TkUtil/ScriptingTools.h"
#include "TkUtil/Exception.h"
#include "TkUtil/File.h"

#include <fstream>


USING_STD
USING_UTIL

static const Charset	charset ("àéèùô");

static const string	tagDeclaration	= ScriptingTools::tagDeclaration;
static const string	beginingTag		= ScriptingTools::fileBeginingTag;
static const string	endTag			= ScriptingTools::endOfFileTag;


int main (int argc, char* argv [])
{
	if (3 != argc)
	{
		cerr << argv [0] << " : erreur de syntaxe." << endl
		     << argv [0] << " script_entrée répertoire_sorties"
		     << endl;
		return -1;
	}	// if (3 != argc)
	const File	script (argv [1]);
	const File	directory (argv [2]);

	try
	{
		if ((false == script.exists ( )) || (true == script.isDirectory ( )) ||
		    (false == script.isReadable ( )))
		{
			UTF8String	message (charset);
			message << "Le fichier " << script.getFullFileName ( )
			        << " n'existe pas, n'est pas accessible en lecture ou est "
			        << "un répertoire.";
			throw Exception (message);
		}
		if ((false == directory.exists ( )) ||
		    (false == directory.isDirectory ( )) ||
		    (false == directory.isWritable ( )))
		{
			UTF8String	message (charset);
			message << "Le répertoire " << directory.getFullFileName ( )
			        << " n'existe pas, n'est pas accessible en écriture ou "
			        << "n'est pas un répertoire.";
			throw Exception (message);
		}

		cout << "Recherche des fichiers incorporés dans le script "
		     << argv [1] << "." << endl;
		ifstream	in (script.getFullFileName ( ).c_str ( ));
		vector <TaggedValue>	taggedValues	=
			ScriptingTools::findTags (
						in, tagDeclaration, ScriptingTools::pythonCommentLine);
		in.close ( );
		cout << "Ecriture des fichiers dans le répertoire "
		     << directory.getFullFileName ( ) << " :" << endl;
		for (vector <TaggedValue>::const_iterator
			it = taggedValues.begin ( ); taggedValues.end ( ) != it; it++)
			if ((*it).getTag ( ) == beginingTag)
			{
				File			file ((*it).getName ( ));
				UTF8String		outFileName (charset);
				outFileName << directory.getFullFileName ( ) << '/'
				            << file.getFileName ( );
				ifstream	ins (script.getFullFileName ( ).c_str ( ));
				ofstream	ofs (outFileName.iso ( ).c_str ( ));
				cout << ' ' << file.getFileName ( );
				ScriptingTools::extractFile (
					ins, ofs, (*it).getName ( ), tagDeclaration, beginingTag,
					endTag, ScriptingTools::pythonCommentLine,
					ScriptingTools::uncommentPythonLine);
			}	// if ((*it).getTag ( ) == beginingTag)
		cout << endl;
	}
	catch (const Exception& exc)
	{
		cout << endl << "Erreur : " << exc.getFullMessage ( ) << endl;
		return -1;
	}
	catch (...)
	{
		cout << endl << "Erreur non documentée." << endl;
		return -1;
	}

	return 0;
}	// main

