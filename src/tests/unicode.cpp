#include "TkUtil/UnicodeString.h"
#include "TkUtil/Exception.h"

#include <values.h>
#include <limits.h>
#include <iostream>


USING_UTIL
USING_STD


int main (int, char*[])
{
	try
	{
		UnicodeString	str1 ("Test");
		string			dela ("de la");
		char			ws	= ' ';
		str1			+= ws + dela + ws;
		str1 << "classe UnicodeString" << ws;
		const string	with ("avec"), chars ("caractères");
		cout << "La chaine \"" << str1.ascii ( )
			<< (true == str1.isAscii ( ) ? "\" est ASCII" : "\" n'est pas ASCII") 
			<< endl;
		cout << "La chaine \"" << str1
			<< (true == str1.isIso ( ) ? 
				"\" est ISO 8859" : "\" n'est pas ISO 8859") 
			<< endl;
		str1 << with << ws << "des" << ws << chars << ws << "accentués" << '.';
		cout << "La chaine \"" << str1 .ascii ( )
			<< (true == str1.isAscii ( ) ? "\" est ASCII" : "\" n'est pas ASCII") 
			<< endl;
		cout << "La chaine \"" << str1
			<< (true == str1.isIso ( ) ? 
				"\" est ISO 8859" : "\" n'est pas ISO 8859") 
			<< endl;
		cout << "La chaine \"" << str1 << " est composée de "
			<< (unsigned long)str1.length ( ) << " caractères." << endl;
		const UnicodeString	of ("de");
		cout << "Position de \"" << with << "\" : " << str1.find (with) << endl;
		cout << "Position de \"" << chars << "\" : " << str1.find (chars) << endl;
		cout << "Position de \"" << of.iso ( ) << "\" : " << str1.find (of) << endl;
		const UnicodeString last ('.');	
		cout << "Position de \"" << last.iso ( ) << "\" : " << str1.find (last)
			<< endl;
		cout << "Dernière position de \"" << with << "\" : " << str1.rfind (with) 
			<< endl;
		cout << "Dernière position de \"" << chars << "\" : " << str1.rfind (chars) 
			<< endl;
		cout << "Dernière position de \"" << of << "\" : " << str1.rfind (of) 
			<< endl;

		char	nl	= '\n';
		str1 << nl << "Saut à la ligne effectué.";
		cout << str1 << endl;
		const WChar_t	ecut	= (unsigned char)'é';
		cout << "Positions de \"" << (char)ecut << "\" : ";
		size_t	pos	= 0;
		for (; pos < str1.length ( ); )
		{
			pos	= str1.find (ecut, pos);
			if ((size_t)-1 == pos)
				break;
			cout << pos << " ";
			pos++;
		}
		cout << endl;
		const char	a	= 'a';
		cout << "Positions de \"" << (char)ecut << "\" en partant de la fin\" : ";
		for (pos = (size_t)-1; pos >= 0; )
		{
			pos	= str1.rfind (ecut, pos);
			if ((size_t)-1 == pos)
				break;
			cout << pos << " ";
			if (0 == pos)
				break;
			pos--;
		}
		cout << endl;
		cout << "Chaine caractère par caractère :" << endl;
		for (pos = 0; pos < str1.length ( ); pos++)
			cout << (char)str1 [pos];
		cout << endl;
		cout << "Contient des espaces : " 
			<< (true == str1.hasWhiteSpace ( ) ? "oui" : "non") << endl;
		UnicodeString	classStr	= str1.substring (8, 30);
		cout << "Sous chaine (8, 30) : " << classStr << endl;
		UnicodeString	endStr	= str1.substring (8);
		cout << "Sous chaine (8, fin) : " << endStr << endl;
		cout << "\n\n" << endl;
		size_t			colNum	= 50;
		cout << "Test de la fonction format : formatage sur " << colNum 
			<< " colonnes :\n";
		UnicodeString	numbers;
		for (pos = 0; pos < 15; pos++)
			numbers	+= "0123456789";
		cout << "Avant formatage :\n" << numbers << endl;
		cout << "Après formatage :\n" << numbers.format (colNum) << endl;
		numbers.clear ( );
		for (pos = 0; pos < 15; pos++)
			numbers	+= "0123456789 ";
		cout << "Avant formatage :\n" << numbers << endl;
		cout << "Après formatage :\n" << numbers.format (colNum) << endl;

		cout << endl << endl
			<< "TESTS AVEC LES NOMBRES" << endl;
		cout << endl << "SHORTS :" << endl;
		short	s	= 0;
		numbers	= UnicodeString (s);
		for (s = 1; s < 5; s++)
			numbers	+= s;
		for (s = 6; s < 10; s++)
			numbers << s;
		cout << numbers << endl;
		s	= MAXSHORT;
		int	columns	= 10;
		cout << "FORMATAGE DE " << s << " SUR " << columns << " COLONNES : ";
		numbers.clear ( );
		numbers << IN_UTIL setw (columns) << s;
		numbers << IN_UTIL setw (0);	// Reinitialisation
		cout << numbers << endl;

		cout << endl << "LONGS :" << endl;
		long	l	= -10;
		numbers	= UnicodeString (l);
		for (l = -9; l < 0; l++)
			numbers	+= l;
		for (l = 0; l < 10; l++)
			numbers << l;
		cout << numbers << endl;
		l		= LONG_MAX;
		columns	= 15;
		cout << "FORMATAGE DE " << l << " SUR " << columns << " COLONNES : ";
		numbers.clear ( );
		numbers << IN_UTIL setw (columns) << l;
		numbers << IN_UTIL setw (0);	// Reinitialisation
		cout << numbers << endl;

		cout << endl << "UNSIGNED LONGS :" << endl;
		unsigned long	ul	= 0;
		numbers	= UnicodeString (ul);
		for (ul = 1; ul < 5; ul++)
			numbers	+= ul;
		for (ul = 5; ul < 10; ul++)
			numbers << ul;
		cout << numbers << endl;
		ul	= ULONG_MAX;
		cout << "FORMATAGE DE " << ul << " SUR " << columns << " COLONNES : ";
		numbers.clear ( );
		numbers << IN_UTIL setw (columns) << ul;
		numbers << IN_UTIL setw (0);	// Reinitialisation
		cout << numbers << endl;

		cout << endl << "FLOATS :" << endl;
		float	f				= 1./3;
		int		decimalNumber	= 8;
		numbers	= UnicodeString (f);
		cout << "1 / 3 = " << f << endl;
		numbers.clear ( );
		numbers << IN_UTIL setprecision (decimalNumber) << f;
		cout << "NOMBRE DE DECIMALES = " << decimalNumber << " 1 / 3 = " 
			<< numbers << endl;
		numbers.clear ( );
		numbers << ios_base::fixed << f;
		cout << "NOTATION FIXE 1 / 3 " << numbers << endl;

		cout << endl << "DOUBLE :" << endl;
		double	d			= MINFLOAT / 5;
		numbers	= UnicodeString (d);
		cout << "MINFLOAT / 5 = " << d << endl;
		numbers.clear ( );
		numbers << IN_UTIL setw (0);
		d	= 0.;
		//	numbers << IN_UTIL setprecision (decimalNumber) << d;
		numbers << IN_UTIL setprecision (8) << d;
		cout << "NOMBRE DE DECIMALES = " << decimalNumber << " MINFLOAT / 5 = " 
			<< numbers << endl;
		numbers.clear ( );
		numbers << ios_base::scientific << d;
		cout << "NOTATION SCIENTIFIQUE MINFLOAT / 5 " << numbers << endl;
	}
	catch (const Exception& exc)
	{
		cout << "Exception levée : " << exc.getFullMessage ( ) << endl;
		return -1;
	}
	catch (...)
	{
		cout << "Exception innattendue." << endl;
		return -1;
	}

	return 0;
}	// main
