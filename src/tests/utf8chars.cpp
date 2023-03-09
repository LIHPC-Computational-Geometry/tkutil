#include "TkUtil/Exception.h"
#include <iostream>


using namespace std;
using namespace TkUtil;


const Charset	localCharset ("àéèùô");

int main (int, char*[])
{
	try
	{
		UTF8String	str1 ("Chaîne de caractères comportant des caractères spéciaux t.q. àéëù et le c cédille \'ç\'.", localCharset);	// 86 caractères sur 96 octets
		cout << "La chaine de caracteres \"" << str1.utf8 ( ) << "\" comporte "
			<< (unsigned long)str1.charNum ( ) << " caracteres encodes sur "
			<< (unsigned long)str1.length ( ) << " octets." << endl;
		const string asciiForm	= str1.convertedToAscii ( );	// 86 caractères
		cout << "Chaine convertie en ASCII :\"" << asciiForm << "\" ("
			<< (unsigned long)asciiForm.length ( ) << " caracteres)." << endl;

		UTF8String	e1 ("é", localCharset);
		UTF8String	e2 ("è", localCharset);
		UTF8String	e ("e", localCharset);
		UTF8String	special ("spéciaux", localCharset);
		UTF8String	accentuated ("accentués", localCharset);
		str1.replace (special, accentuated, true);
		str1.replace (e1, e, true);
		str1.replace (e2, e, true);
		cout << "Chaine apres divers remplacements : \"" << str1 << "\"." << endl;
	}
	catch (const Exception& exc)
	{
		cout << "ERREUR : " << exc.getFullMessage ( ) << endl;
		return -1;
	}
	catch (const exception& e)
	{
		cout << "ERREUR : " << e.what ( ) << endl;
		return -1;
	}
	catch (...)
	{
		cout << "Erreur non documentée." << endl;
		return -1;
	}

	return 0;
}	// main

