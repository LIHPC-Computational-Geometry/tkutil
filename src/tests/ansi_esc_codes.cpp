/**
 * Teste l'utilisations des séquences ANSI ESC lors d'écritures dans la sortie
 * standard.
 *
 * @author	Charles PIGNEROL, sur une idée de Guénolé HAREL
 */
#include "TkUtil/Exception.h"
#include "TkUtil/AnsiEscapeCodes.h"

#include <iostream>


USING_UTIL
USING_STD


int main (int argc, char* argv[])
{
	try 
	{
		static const char*	asciiLine	= "La ligne test avec encodage  ASCII.";
		cout << AnsiEscapeCodes::reset;
		cout << "On efface complement l'ecran => cette ligne doit disparaitre." << endl;
		cout << AnsiEscapeCodes::clearScreen;
		cout << "Ligne avec jeu de caractères par défaut." << endl;
		cout << asciiLine << endl;
		cout << AnsiEscapeCodes::bold << asciiLine << AnsiEscapeCodes::boldOff << endl;
		cout << "Encodage italique (pas toujours supporte) : ";
		{ TermAutoStyle (cout, AnsiEscapeCodes::italic); cout << asciiLine << endl; }
		cout << "Encodage bleu sur fond jaune souligne : ";
		{ TermAutoStyle as1 (cout, AnsiEscapeCodes::underline);
		TermAutoStyle as2 (cout, AnsiEscapeCodes::blueFg);
		TermAutoStyle as3 (cout, AnsiEscapeCodes::yellowBg);
		cout << asciiLine; }
		cout << endl;	// maintenant car Bg.
		cout << "Encodage video inverse clignotant fonte 5 :";
		{ TermAutoStyle as1 (cout, AnsiEscapeCodes::reverseVideo);
		TermAutoStyle as2 (cout, AnsiEscapeCodes::slowBlink);
		TermAutoStyle as3 (cout, AnsiEscapeCodes::font5);
		cout << asciiLine << endl; }
		cout << "Encodage encadre magenta fond cyan : ";
		{ TermAutoStyle as1 (cout, AnsiEscapeCodes::framed);
		TermAutoStyle as2 (cout, AnsiEscapeCodes::magentaFg);
		TermAutoStyle as3 (cout, AnsiEscapeCodes::cyanBg);
		cout << asciiLine; }
		cout << endl;	// maintenant car Bg.
		cout << "Encodage entoure blanc fond vert brillant : ";
		{ TermAutoStyle as1 (cout, AnsiEscapeCodes::encircled);
		TermAutoStyle as2 (cout, AnsiEscapeCodes::whiteFg);
		TermAutoStyle as3 (cout, AnsiEscapeCodes::brightGreenBg);
		cout << asciiLine; }
		cout << endl;	// maintenant car Bg.
		cout << "Encodage sur ligne jaune : ";
		{ TermAutoStyle as1 (cout, AnsiEscapeCodes::overlined);
		cout << asciiLine << endl; }
	//	cout << endl;	// maintenant car Bg.

		cout << AnsiEscapeCodes::yellowBg << AnsiEscapeCodes::blueFg;
		cout << "0000000000000000000000000000000000000000000000000000" << endl;
		cout << "1111111111111111111111111111111111111111111111111111" << endl;
		cout << "2222222222222222222222222222222222222222222222222222" << endl;
		cout << "3333333333333333333333333333333333333333333333333333" << endl;
		cout << "4444444444444444444444444444444444444444444444444444" << endl;
		cout << "5555555555555555555555555555555555555555555555555555" << endl;
		cout << "6666666666666666666666666666666666666666666666666666" << endl;
		cout << "7777777777777777777777777777777777777777777777777777" << endl;
		cout << "8888888888888888888888888888888888888888888888888888" << endl;
		cout << "9999999999999999999999999999999999999999999999999999";

		cout << AnsiEscapeCodes::defaultFg;
		cout << AnsiEscapeCodes::defaultBg;
		TermCursorPos::move (cout, TermCursorPos::UP, 10);
		TermCursorPos::move (cout, TermCursorPos::DOWN, 3);
		TermCursorPos::move (cout, TermCursorPos::LEFT, 20);
		cout << AnsiEscapeCodes::whiteBg << AnsiEscapeCodes::redFg;
		cout << "ATTENTION";
		cout << AnsiEscapeCodes::defaultFg;
		cout << AnsiEscapeCodes::defaultBg;
		TermCursorPos::moveTo (cout, 5, 2);
		cout << AnsiEscapeCodes::whiteBg << AnsiEscapeCodes::redFg;
		cout << "POSITION ABSOLUE (5,2) ATTENTION";
		cout << AnsiEscapeCodes::defaultFg;
		cout << AnsiEscapeCodes::defaultBg;

		cout << AnsiEscapeCodes::reset;
		cout << "Fin programme : encodage par defaut." << endl;
	}
	catch (const Exception& exc)
	{
		cout << "Exception caught : " << exc.getFullMessage ( ) << endl;
		return -1;
	}
	catch (const exception& e)
	{
		cout << "Standard exception caught : " << e.what ( ) << endl;
		return -1;
	}
	catch (...)
	{
		cout << "Undocumented exception caught." << endl;
		return -1;
	}

	return 0;
}	// main
