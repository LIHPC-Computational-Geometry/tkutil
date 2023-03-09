#include "TkUtil/NetworkData.h"
#include "TkUtil/MachineData.h"
#include "TkUtil/RemoteProcess.h"
#include "TkUtil/Exception.h"

#include <iostream>
#include <stdexcept>


USING_STD
USING_UTIL


int main (int argc, char* argv [], char* envp[])
{
	MachineData	machine (MachineData::instance ( ));

	cout << "Machine locale         : " << NetworkData::getCurrentHostName ( )
	     << endl
	     << "Système d'exploitation : "
	     << machine.getOperatingSystem ( ).getName ( ) << ' '
	     << machine.getOperatingSystem ( ).getVersion ( ).getVersion ( ) << ' '
	     << machine.getOperatingSystem ( ).getInformations ( )
	     << endl
	     << "Matériel               : " << machine.getHardware ( )
	     << endl
	     << "Nombre processeurs     : " << machine.getProcessorsNum ( )
	     << endl
	     << "Précision float        : " << machine.floatSignificantNumeral ( )
					<< " (mantisse complète)"
	     << endl
	     << "Précision double       : " << machine.doubleSignificantNumeral ( )
					<< " (mantisse complète)"
	     << endl
	     << "Informations numériques :" << endl
	     << machine.getOperatingSystem ( ).getNumericInformations ( )
	     << endl;
	cout << "Machine virtuelle       : " << (true == machine.isVirtualMachine ( ) ? "oui" : "non") << endl;

	if (argc <= 1)
	{
		cout << "No hostName or TCP/IP address provided as argument ==> no remote connection test" << endl;
	}
	else
	{
		try
		{
			NetworkData	host (argv [1]);
			cout << argv [1] << " : " << host.getFullHostName ( ) << endl
				<< "Nom : " << host.getHostName ( ) << endl << "Alias :" << endl;
			vector<string>	aliases	= host.getAliases ( );
			for (vector<string>::iterator	ita = aliases.begin ( );
				aliases.end ( ) != ita; ita++)
				cout << *ita << endl;
			cout << "Adresses TCP/IP :" << endl;
			vector<string>	ipAddr	= host.getInetAddr ( );
			for (vector<string>::iterator	itip = ipAddr.begin ( );
				ipAddr.end ( ) != itip; itip++)
				cout << *itip << endl;

			Process::initialize (envp);
			RemoteProcess::remoteShell	= "ssh";
			RemoteProcess::launcher		= "TkUtilLauncher";
			NetworkData	usableInfos	= NetworkData::getMachineInfos (argv [1]);
			cout << "Informations utilisables depuis cette machine :" << endl
				<< "Nom : " << usableInfos.getHostName ( ) << endl
				<< "Adresses TCP/IP : " << endl;
			ipAddr	= usableInfos.getInetAddr ( );
			for (vector<string>::iterator	itip2 = ipAddr.begin ( );
				ipAddr.end ( ) != itip2; itip2++)
				cout << *itip2 << endl;
		}
		catch (const Exception& exc)
		{
			cout << "Exception : " << exc.getFullMessage ( ) << endl;
			return -1;
		}
		catch (const exception& e)
		{
			cout << "Exception standard : " << e.what ( ) << endl;
			return -1;
		}
		catch (...)
		{
			cout << "Exception non documentée." << endl;
			return -1;
		}
	}

	return 0;
}	// main
