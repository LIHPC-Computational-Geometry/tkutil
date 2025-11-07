// Test du stockage de stats d'utilisation d'un produit.
#include <TkUtil/ApplicationStats.h>
#include <TkUtil/MachineData.h>
#include <TkUtil/Process.h>
#include <TkUtil/UtilInfos.h>

#include <iostream>
#include <string>

using namespace TkUtil;
using namespace std;


int main (int argc, char* argv [], char* envp [])
{
	if (argc < 3)
	{
		cerr << "Syntaxe : " << argv [0] << " NomApp RépertoireLogs" << endl;
		return -1;
	}	// if (argc < 3)

	Process::initialize (argc, argv, envp);
	ApplicationStats::logUsage (argv [1], argv [2]);
	const File	exe (argv [0]);
	ApplicationStats::logUsage (argv [1], argv [2], UtilInfos::getVersion ( ).getVersion ( ), OperatingSystem::instance ( ).getDistribution ( ), exe.getPath ( ).getFullFileName ( ));
	Process::finalize (1);
	
	return 0;
}	// main
