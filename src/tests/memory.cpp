#include "TkUtil/ProcessMemory.h"
#include "TkUtil/Exception.h"

#include <iostream>


USING_UTIL
USING_STD


int main (int, char*[])
{
	try
	{
		ProcessMemory	memory;
		size_t			smallBlocksCount	= 1000, i = 0;
		size_t			smallBlocksSize		= 1024;
		int**			smallBlocks			= 0;
		size_t			bigBlocksCount		= 10;
		size_t			bigBlocksSize		= 1024*1024;
		int**			bigBlocks			= 0;

		// mallopt (M_MMAP_THRESHOLD) permet de jouer sur la manière dont est
		// allouée la mémoire par malloc. En dessous de la valeur renseignée la
		// mémoire allouée est rapide et a priori non restituable au système.
		// Au dessus de cette valeur malloc utilise mmap : la mémoire utilise un
		// fichier et est totalement restituée au système lors de l'appel de free.
		//mallopt (M_MMAP_THRESHOLD, 1);
		cout << "Etat de la mémoire du processus au démarrage :" << endl
			<< memory.getReport ( ) << endl;

		cout << endl << "Allocation de " << (unsigned long)smallBlocksCount
			<< " petits blocs de " << (unsigned long)smallBlocksSize
			<< " entiers." << endl;
		smallBlocks	= new int* [smallBlocksCount];
		for (i = 0; i < smallBlocksCount; i++)
			smallBlocks [i]	= new int [smallBlocksSize];
		memory.reset ( );
		cout << "Etat de la mémoire du processus :" << endl
			<< memory.getReport ( ) << endl;

		cout << endl << "Allocation de " << (unsigned long)bigBlocksCount
			<< " gros blocs de " << (unsigned long)bigBlocksSize
			<< " entiers." << endl;
		bigBlocks	= new int* [bigBlocksCount];
		for (i = 0; i < bigBlocksCount; i++)
			bigBlocks [i]	= new int [bigBlocksSize];
		memory.reset ( );
		cout << "Etat de la mémoire du processus :" << endl
			<< memory.getReport ( ) << endl;
	//while (1);	// Pour comparaison avec top

		cout << endl << "Suppression des petits blocs de mémoire." << endl;
		for (i = 0; i < smallBlocksCount; i++)
			delete [] smallBlocks [i];
		delete [] smallBlocks;	smallBlocks	= 0;
		memory.reset ( );
		cout << "Etat de la mémoire du processus :" << endl
			<< memory.getReport ( ) << endl;

		cout << endl << "Suppression des gros blocs de mémoire." << endl;
		for (i = 0; i < bigBlocksCount; i++)
			delete [] bigBlocks [i];
		delete [] bigBlocks;	bigBlocks	= 0;
		memory.reset ( );
		cout << "Etat de la mémoire du processus :" << endl
			<< memory.getReport ( ) << endl;
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
