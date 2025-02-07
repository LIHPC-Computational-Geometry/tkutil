// Test du stockage de stats d'utilisation d'un produit.
#include "TkUtil/ApplicationStats.h"

#include <iostream>
#include <string>

using namespace TkUtil;
using namespace std;


int main (int argc, char* argv [])
{
	if (argc < 3)
	{
		cerr << "Syntaxe : " << argv [0] << " NomApp RépertoireLogs" << endl;
		return -1;
	}	// if (argc < 3)
	
	ApplicationStats::logUsage (argv [1], argv [2]);
	
	return 0;
}	// main
