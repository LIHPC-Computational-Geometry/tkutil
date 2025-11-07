#include "TkUtil/MachineData.h"
#include "TkUtil/MemoryError.h"
#include "TkUtil/NumericServices.h"
#include "TkUtil/Exception.h"
#include "TkUtil/NumericConversions.h"
#include "TkUtil/Process.h"
#include "TkUtil/UTF8String.h"
#include "TkUtil/UtilInfos.h"

#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>	// getenv
#include <float.h>	// FLT_DIG, DBL_DIG
#include <errno.h>
#include <sys/utsname.h>
#include <limits>
#include <string.h>
#include <iostream>
#include <strstream>
#include <iomanip>
#include <memory>


USING_STD


BEGIN_NAMESPACE_UTIL

static const Charset	charset ("àéèùô");

// ============================================================================
//                          LA CLASSE OperatingSystem
// ============================================================================


OperatingSystem *				OperatingSystem::_currentOS	= 0;


OperatingSystem::OperatingSystem ( )
	: _name ( ), _distribution (OS_DISTRIBUTION), _version ("0.0.0"), _infos ( ), _numericInfos ( )
{
	errno	= 0;

	struct	utsname	osInfos;
	int		retVal	= uname (&osInfos);

	if (-1 == retVal)
	{
		ConsoleOutput::cout ( )
		     << "Erreur lors de l'initialisation des informations sur le système d'exploitation de la machine "
		     << MachineData::instance ( ).getCurrentHostName ( ) << " : " << co_endl << strerror (errno) << ".";
		errno	= 0;
	}	// if (-1 == retVal)
	else
	{
		setName (osInfos.sysname);
		try
		{
			setVersion (Version (osInfos.release));
		}
		catch (const Exception& exc)
		{
			ConsoleOutput::cout ( )
			     << "Erreur lors de l'initialisation des informations sur le système d'exploitation de la machine "
			     << MachineData::instance ( ).getCurrentHostName ( ) << " : " << co_endl << exc.getFullMessage ( ) << ".";
		}
		catch (...)
		{
			ConsoleOutput::cout ( )
			     << "Erreur lors de l'initialisation des informations sur le système d'exploitation de la machine "
			     << MachineData::instance ( ).getCurrentHostName ( ) << " : " << co_endl << "Erreur non documentée." << ".";
		}
		setInformations (osInfos.version);
	}	// else if (0 != retVal)

	try
	{
		UTF8String	numericInfos (charset);
		numericInfos
			<< "Reels double precision : " << '\n'
			<< "Taille en octets .............. : "
			<< NumericServices::doubleSize << '\n'
			<< "Max ........................... : "
			<< NumericServices::doubleMachMax ( )
			<< '\n'
			<< "Min ........................... : "
			<<  NumericServices::doubleMachMin ( )
			<< '\n'
			<< "Infini ........................ : "
			<< NumericServices::doubleMachInfinity ( )
			<< '\n'
			<< "Epsilon ....................... : "
			<< NumericServices::doubleMachEpsilon ( )
			<< '\n'
			<< "Chiffres significatifs garantis : "
			<< NumericServices::doubleDigitsMax
		    << " (mantisse complete)" << '\n'

			<< "Reels double precision longs : "
			<< '\n'
			<< "Taille en octets .............. : "
			<< NumericServices::longDoubleSize << '\n'
			<< "Max ........................... : "
			<< NumericServices::longDoubleMachMax ( )
			<< '\n'
			<< "Min ........................... : "
			<< NumericServices::longDoubleMachMin ( )
			<< '\n'
			<< "Infini ........................ : "
			<< NumericServices::longDoubleMachInfinity ( )
			<< '\n'
			<< "Epsilon ....................... : "
			<< NumericServices::longDoubleMachEpsilon ( )
			<< '\n'
			<< "Chiffres significatifs garantis : "
			<< " (mantisse complete)" << NumericServices::longDoubleDigitsMax
			<< '\n'

			<< "Reels simple precision : " << '\n'
			<< "Taille en octets .............. : "
			<< NumericServices::floatSize << '\n'
			<< "Max ........................... : "
			<< NumericServices::floatMachMax ( )
			<< '\n'
			<< "Min ........................... : "
			<< NumericServices::floatMachMin ( )
			<< '\n'
			<< "Infini ........................ : "
			<< NumericServices::floatMachInfinity ( )
			<< '\n'
			<< "Epsilon ....................... : "
			<< NumericServices::floatMachEpsilon ( )
			<< '\n'
			<< "Chiffres significatifs garantis : "
			<< NumericServices::floatDigitsMax
			<< " (mantisse complete)" << '\n'

			<< "Caracteres (char) : " << '\n'
			<< "Taille en octets .............. : "
			<< NumericServices::charSize << '\n'
			<< "Max ........................... : "
			<< NumericConversions::toStr (
										(short)NumericServices::charMachMax ( ))
			<< '\n'
			<< "Min ........................... : "
			<< NumericConversions::toStr (
										(short)NumericServices::charMachMin ( ))
			<< '\n'
			<< "Chiffres significatifs garantis : "
			<< NumericServices::charDigitsMax
			<< '\n'

			<< "Caracteres non signes (unsigned char) : "
			<< '\n'
			<< "Taille en octets .............. : "
			<< NumericServices::unsignedCharSize
			<< '\n'
			<< "Max ........................... : "
			<< NumericConversions::toStr (
					(unsigned short)NumericServices::unsignedCharMachMax ( ))
			<< '\n'
			<< "Min ........................... : "
			<< NumericConversions::toStr (
					(unsigned short)NumericServices::unsignedCharMachMin ( ))
			<< '\n'
			<< "Chiffres significatifs garantis : "
			<< NumericServices::unsignedCharDigitsMax
			<< '\n'

			<< "Caracteres longs non signes (wchar_t, usage unicode) : "
			<< '\n'
			<< "Taille en octets .............. : "
			<< NumericServices::wcharSize << '\n'
			<< "Max ........................... : "
			<< NumericConversions::toStr (
								(WChar_t)NumericServices::wcharMachMax ( ))
			<< '\n'
			<< "Min ........................... : "
			<< NumericConversions::toStr (
								(WChar_t)NumericServices::wcharMachMin ( ))
			<< '\n'
			<< "Chiffres significatifs garantis : "
			<< NumericServices::wcharDigitsMax
			<< '\n'

			<< "Entiers courts (short) : " << '\n'
			<< "Taille en octets .............. : "
			<< NumericServices::shortSize << '\n'
			<< "Max ........................... : "
			<< NumericServices::shortMachMax ( )
			<< '\n'
			<< "Min ........................... : "
			<< NumericServices::shortMachMin ( )
			<< '\n'
			<< "Chiffres significatifs garantis : "
			<< NumericServices::shortDigitsMax
			<< '\n'

			<< "Entiers courts non signes (unsigned short) : "
			<< '\n'
			<< "Taille en octets .............. : "
			<< NumericServices::unsignedShortSize
			<< '\n'
			<< "Max ........................... : "
			<< NumericConversions::toStr (
									NumericServices::unsignedShortMachMax ( ))
			<< '\n'
			<< "Min ........................... : "
			<< NumericConversions::toStr (
									NumericServices::unsignedShortMachMin ( ))
			<< '\n'
			<< "Chiffres significatifs garantis : "
			<< NumericServices::unsignedShortDigitsMax
			<< '\n'

			<< "Entiers (int) : " << '\n'
			<< "Taille en octets .............. : "
			<< NumericServices::intSize << '\n'
			<< "Max ........................... : "
		    << (long)NumericServices::intMachMax ( )
			<< '\n'
			<< "Min ........................... : "
		    << (long)NumericServices::intMachMin ( )
			<< '\n'
			<< "Chiffres significatifs garantis : "
			<< NumericServices::intDigitsMax
			<< '\n'

			<< "Entiers non signes (unsigned int) : "
			<< '\n'
			<< "Taille en octets .............. : "
			<< NumericServices::unsignedIntSize
			<< '\n'
			<< "Max ........................... : "
			<< (unsigned long)NumericServices::unsignedIntMachMax ( )
			<< '\n'
			<< "Min ........................... : "
			<< (unsigned long)NumericServices::unsignedIntMachMin ( )
			<< '\n'
			<< "Chiffres significatifs garantis : "
			<< NumericServices::unsignedIntDigitsMax
			<< '\n'

			<< "Entiers longs (long) : " << '\n'
			<< "Taille en octets .............. : "
			<< NumericServices::longSize << '\n'
			<< "Max ........................... : "
			<< NumericServices::longMachMax ( )
			<< '\n'
			<< "Min ........................... : "
			<< NumericServices::longMachMin ( )
			<< '\n'
			<< "Chiffres significatifs garantis : "
			<< NumericServices::longDigitsMax
			<< '\n'

			<< "Entiers longs non signes (unsigned long) : "
			<< '\n'
			<< "Taille en octets .............. : "
			<< NumericServices::unsignedLongSize
			<< '\n'
			<< "Max ........................... : "
			<< NumericServices::unsignedLongMachMax ( )
			<< '\n'
			<< "Min ........................... : "
			<< NumericServices::unsignedLongMachMin ( )
			<< '\n'
			<< "Chiffres significatifs garantis : "
			<< NumericServices::unsignedLongDigitsMax
			<< '\n'

			<< "Entiers longs longs (long long) : "
			<< '\n'
			<< "Taille en octets .............. : "
			<< NumericServices::longLongSize << '\n'
			<< "Max ........................... : "
			<< NumericServices::longLongMachMax ( )
			<< '\n'
			<< "Min ........................... : "
			<< NumericServices::longLongMachMin ( )
			<< '\n'
			<< "Chiffres significatifs garantis : "
			<< NumericServices::longLongDigitsMax
			<< '\n'

			<< "Entiers longs longs non signes (unsigned long long) : "
			<< '\n'
			<< "Taille en octets .............. : "
			<< NumericServices::unsignedLongLongSize
			<< '\n'
			<< "Max ........................... : "
			<< NumericServices::unsignedLongLongMachMax ( )
			<< '\n'
			<< "Min ........................... : "
			<< NumericServices::unsignedLongLongMachMin ( )
			<< '\n'
			<< "Chiffres significatifs garantis : "
			<< NumericServices::unsignedLongLongDigitsMax
			<< '\n';

		// numericInfos.utf8 ( ) : copie directe du buffer, mais ne doit contenir que des caractères ASCII.
		setNumericInformations (numericInfos.utf8 ( ));
	}
	catch (const Exception& exc)
	{
		ConsoleOutput::cout ( )
		     << "Erreur lors de l'initialisation des informations numériques sur le système d'exploitation de la machine "
		     << MachineData::instance ( ).getCurrentHostName ( ) << " : " << co_endl << exc.getFullMessage ( ) << ".";
	}
	catch (const exception& e)
	{
		ConsoleOutput::cout ( )
		     << "Erreur lors de l'initialisation des informations numériques sur le système d'exploitation de la machine "
		     << MachineData::instance ( ).getCurrentHostName ( ) << " : " << co_endl << e.what ( ) << ".";
	}
	catch (...)
	{
		ConsoleOutput::cout ( )
		     << "Erreur lors de l'initialisation des informations numériques sur le système d'exploitation de la machine "
		     << MachineData::instance ( ).getCurrentHostName ( ) << " : " << co_endl << "Erreur non documentée." << ".";
	}
}	// OperatingSystem::OperatingSystem


OperatingSystem::OperatingSystem (const string& name, const Version& version, const string& infos, const string& numericInfos)
	: _name (name), _version (version), _distribution (OS_DISTRIBUTION), _infos (infos), _numericInfos (numericInfos)
{
}	// OperatingSystem::OperatingSystem 


OperatingSystem::OperatingSystem (const OperatingSystem& os)
	: _name (os._name), _distribution (OS_DISTRIBUTION), _version (os._version), _infos (os._infos), _numericInfos (os._numericInfos)
{
}	// OperatingSystem::OperatingSystem
	

const OperatingSystem& OperatingSystem::instance ( )
{
	if (0 == _currentOS)
		_currentOS	= new OperatingSystem ( );

	return *_currentOS;
}	// OperatingSystem::instance


// ============================================================================
//                             LA CLASSE MachineData
// ============================================================================


MachineData*		MachineData::_currentMachine	= 0;
bool				MachineData::_instanciated	= false; // Lib non instanciée

MachineData::MachineData ( )
	: NetworkData ( ), _processorNum ((size_t)-1), _operatingSystem (OperatingSystem::instance ( )), _hardware ( )
{
	// LE CODE DOIT ETRE LE MEME DANS MachineData::MachineData (char)  POUR MachineData::_currentMachine.
	long	nbProcs	= -1;
#ifdef _SC_NPROCESSORS_ONLN
	nbProcs	= sysconf (_SC_NPROCESSORS_ONLN);
#elif defined(_SC_NPROC_ONLN)
	nbProcs	= sysconf (_SC_NPROC_ONLN);
#else
	ConsoleOutput::cout ( )
	     << "ERREUR dans MachineData::MachineData : détermination impossible du nombre de processeurs de la machine " << getHostName ( ) << co_endl << "\a";
#endif	// _SC_NPROCESSORS_ONLN
	_processorNum	= (size_t)nbProcs;

	errno	= 0;
	struct	utsname	osInfos;
	int		retVal	= uname (&osInfos);

	if (-1 == retVal)
	{
		ConsoleOutput::cout ( )
		     << "Erreur lors de l'initialisation des informations sur le système matériel de la machine "
		     << getCurrentHostName ( ) << " : " << co_endl << strerror (errno) << ".";
		errno	= 0;
	}	// if (-1 == retVal)
	else
		_hardware	= osInfos.machine;
}	// MachineData::MachineData ( )


MachineData::MachineData (const MachineData& machine)
	: NetworkData (machine), _processorNum (machine._processorNum), _operatingSystem (machine._operatingSystem), _hardware (machine._hardware)
{
}	// MachineData::MachineData (const MachineData&)


MachineData::MachineData (char)
	: NetworkData ( ), _processorNum ((size_t)-1), _operatingSystem (OperatingSystem::instance ( )), _hardware ( )
{
	if (true == _instanciated)	// v 2.27.0
	{	// Protection contre les (dé)chargements multiples de la bibliothèque
		UTF8String	message (charset);
		message << "ATTENTION. Chargement de la version " << UtilInfos::getVersion ( ).getVersion ( )
		     << " de la bibliothèque TkUtil. Cette bibliothèque est déjà chargée, et ce second chargement risque de provoquer un "
		     << "comportement anormal, voire un plantage, du logiciel."
		     << "\nErreur détectée en ligne " << (unsigned long)__LINE__ << " du fichier " << (unsigned long)__FILE__ << "\n";
		ConsoleOutput::cout ( ) << message << co_endl;
	}	// if (true == _instanciated)

	_instanciated	= true;

// CODE IDENTIQUE MachineData::MachineData ( ) :
	long	nbProcs	= -1;
#ifdef _SC_NPROCESSORS_ONLN
	nbProcs	= sysconf (_SC_NPROCESSORS_ONLN);
#elif defined(_SC_NPROC_ONLN)
	nbProcs	= sysconf (_SC_NPROC_ONLN);
#else
	ConsoleOutput::cout ( )
	     << "ERREUR dans MachineData::MachineData : détermination impossible du nombre de processeurs de la machine " << getHostName ( ) << co_endl << "\a";
#endif	// _SC_NPROCESSORS_ONLN
	_processorNum	= (size_t)nbProcs;

	errno	= 0;
	struct	utsname	osInfos;
	int		retVal	= uname (&osInfos);

	if (-1 == retVal)
	{
		ConsoleOutput::cout ( )
		     << "Erreur lors de l'initialisation des informations sur le système matériel de la machine "
		     << getCurrentHostName ( ) << " : " << co_endl << strerror (errno) << ".";
		errno	= 0;
	}	// if (-1 == retVal)
	else
		_hardware	= osInfos.machine;
// FIN CODE IDENTIQUE MachineData::MachineData ( ) :
}	// MachineData::MachineData


MachineData& MachineData::operator = (const MachineData& machine)
{
	if (&machine != this)
	{
		_processorNum		= machine._processorNum;
		_operatingSystem	= machine._operatingSystem;
		_hardware			= machine._hardware;
	}	// if (&machine != this)

	return *this;
}	// MachineData::operator =


MachineData::~MachineData ( )
{
	if (_currentMachine == this)	// v 2.27.0
	{	// Protection contre les (dé)chargements multiples de la bibliothèque
		if (false == _instanciated)
		{
			ConsoleOutput::cout ( )
			     << "ATTENTION. Déchargement de la version " << UtilInfos::getVersion ( ).getVersion ( )
			     << " de la bibliothèque TkUtil. Cette bibliothèque est déjà déchargée, et ce second déchargement risque de provoquer "
			     << "comportement anormal, voire un plantage, du logiciel." << co_endl
			     << "Erreur détectée en ligne " << (unsigned long)__LINE__ << " du fichier " << (unsigned long)__FILE__ << co_endl;
		}	// if (false == _instanciated)

		_instanciated	= false;
	}	// if (_currentMachine == this)
}	// MachineData::~MachineData


const MachineData& MachineData::instance ( )	// v 4.0.0
{
	if (0 == _currentMachine)
		_currentMachine	= new MachineData ((char)0);

	return *_currentMachine;
}	// MachineData::instance


bool MachineData::isRemoteHost ( )
{
	size_t	pos	= 0;
	// SSH_CLIENT :
	// En cas de connexion sur une autre machine :
	// IP_départ Port_départ Port_machine où départ est la machine
	// d'où l'on vient et machine celle où est effectué le getenv.
	const char*		sshClient		= getenv ("SSH_CLIENT");
	const string	strSClient (NULL == sshClient ? "" : sshClient);
	// SSH_CONNECTION :
	// En cas de connexion sur une autre machine :
	// IP_départ Port_départ IP_machine Port_machine où départ est la machine
	// d'où l'on vient et machine celle où est effectué le getenv.
	const char*		sshConnection	= getenv ("SSH_CONNECTION");
	const string	strSConnection (NULL == sshConnection ? "" : sshConnection);
	// SSH_AUTH_SOCK :
	// Identifie le socket de communication avec l'agent ssh => semble
	// unique sur la station de login, quelque soit le screen, et absent si on
	// est logué sur une autre machine.
	// Non testé
//	const char*		sshAuthSock		= getenv ("SSH_AUTH_SOCK");
//	const string	strSAuthSock (NULL == sshAuthSock ? "" : sshAuthSock);
	// Cette machine :
	NetworkData				localHost;
	const vector<string>	localIPs	= localHost.getInetAddr ( );

	// On souhaite être sous SSH :
	if ((NULL != sshClient) || (NULL != sshConnection))
	{
		string	cltIP;
		if (false == strSClient.empty ( ))
		{
			pos	= strSClient.find (' ');
			if (string::npos != pos)
				cltIP	= strSClient.substr (0, pos);
		}	// if (false == strSClient.empty ( ))
		if (true == cltIP.empty ( ))
		{
			pos	= strSConnection.find (' ');
			if (string::npos != pos)
				cltIP	= strSConnection.substr (0, pos);
		}	// if (true == cltIP.empty ( ))

		if (false == cltIP.empty ( ))
		{
			for (vector<string>::const_iterator it = localIPs.begin ( );
			     it != localIPs.end ( ); it++)
				if (*it == cltIP)
					// Hautement improbable, suppose qu'on ait fait ssh départ
					return false;

			// Pas trouvé => on s'est logué sur une autre machine.
			return true;
		}	// if (false == cltIP.empty ( ))
	}	// if ((NULL != sshClient) || (NULL != sshConnection))

	// Peut être n'est on pas sous SSH. 
	const char*		display		= getenv ("DISPLAY");
	const string	strDisplay (NULL == display ? "" : display);
	if (0 == strDisplay.find (":0"))
		return false;

	pos	= strDisplay.find (":");
	string	machine	= strDisplay.substr (0, pos);
	if (false == machine.empty ( ))
	{
		if (machine == "localhost")	// SSH pour X11 tunneling
			return true;

		NetworkData				displayHost (machine);
		const vector<string>	displayIPs	= displayHost.getInetAddr ( );
		for (vector<string>::const_iterator itd = displayIPs.begin ( );
		     displayIPs.end ( ) != itd; itd++)
			for (vector<string>::const_iterator itl = localIPs.begin ( );
			     itl != localIPs.end ( ); itl++)
				if (*itl == *itd)
					return false;

		// Pas d'IP commune alors qu'elles devraient toutes être communes :
		return true;
	}	// if (false == machine.empty ( ))

	return false;
}	// MachineData::isRemoteHost


bool MachineData::isVirtualMachine ( )	// v 5.12.0
{
	static bool	first	= true;
	static bool	vm		= false;

	if (true == first)
	{
		string	output, line;
		
		if (false == vm)
		{
			try
			{	// v 5.12.1 : facter
				unique_ptr<Process>	facter (new Process ("facter"));
				CHECK_NULL_PTR_ERROR (facter.get ( ))
				facter->getOptions ( ).addOption ("virtual");
				facter->enableChildToSonCommunications ( );
				facter->execute (false);
				line	= facter->getChildLine ( );	// physical, kvm, ovirt, ...
				output.clear ( );
				while (false == line.empty ( ))
				{
					output	+= line;
					try
					{
						line	= facter->getChildLine ( );
					}
					catch (...)
					{
						line.clear ( );
					}
				}	// while (false == line.empty ( ))
				if (NULL != strcasestr (output.c_str ( ), "physical"))
					vm	= false;
			}
			catch (...)
			{
				cerr << "Détection du caractère virtuel de la machine courante : commande facter non trouvée." << endl;
			}

		}	// if (false == vm)
		
		if (false == vm)
		{
			try
			{
				unique_ptr<Process>	dmesg (new Process ("dmesg"));
				CHECK_NULL_PTR_ERROR (dmesg.get ( ))
				/* Rem : le | ne semble pas fonctionner via la classe Process. Enchaînement de processus ?
				dmesg->getOptions ( ).addOption ("|");
				dmesg->getOptions ( ).addOption ("grep");
				dmesg->getOptions ( ).addOption ("-i");
				dmesg->getOptions ( ).addOption ("hypervisor detected"); */
				dmesg->enableChildToSonCommunications ( );
				dmesg->execute (false);
				line	= dmesg->getChildLine ( );
				output.clear ( );
				while (false == line.empty ( ))
				{
					output	+= line;
					try
					{
						line	= dmesg->getChildLine ( );
					}
					catch (...)
					{
						line.clear ( );
					}
				}	// while (false == line.empty ( ))
				if (NULL != strcasestr (output.c_str ( ), "hypervisor detected"))
					vm	= true;
			}
			catch (...)
			{
				cerr << "Détection du caractère virtuel de la machine courante : commande dmesg non trouvée." << endl;
			}
		}	// if (false == vm)
		
		if (false == vm)
		{
			try
			{
				
				unique_ptr<Process>	lshw (new Process ("lshw"));
				CHECK_NULL_PTR_ERROR (lshw.get ( ))
				/* Rem : le | ne semble pas fonctionner via la classe Process. Enchaînement de processus ?
				lshw->getOptions ( ).addOption ("|");
				lshw->getOptions ( ).addOption ("grep");
				lshw->getOptions ( ).addOption ("-i");
				lshw->getOptions ( ).addOption ("qemu"); */
				lshw->enableChildToSonCommunications ( );
				lshw->execute (false);
				string	line	= lshw->getChildLine ( );
				output.clear ( );
				while (false == line.empty ( ))
				{
					output	+= line;
					try
					{
						line	= lshw->getChildLine ( );
					}
					catch (...)
					{
						line.clear ( );
					}
				}	// while (false == line.empty ( ))
				if (NULL != strcasestr (output.c_str ( ), "qemu"))
					vm	= true;
			}
			catch (...)
			{
				cerr << "Détection du caractère virtuel de la machine courante : commande lshw non trouvée." << endl;
			}
		}	// if (false == vm)
		
		first	= false;
	}	// if (true == first)
	
	return vm;
}	// MachineData::isVirtualMachine


unsigned short MachineData::floatSignificantNumeral ( )
{
	return FLT_DIG;
}	// MachineData::floatSignificantNumeral


unsigned short MachineData::doubleSignificantNumeral ( )
{
	return DBL_DIG;
}	// MachineData::doubleSignificantNumeral



END_NAMESPACE_UTIL
