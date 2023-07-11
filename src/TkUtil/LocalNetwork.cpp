#include <TkUtil/Exception.h>
#include <TkUtil/LocalNetwork.h>
#include <TkUtil/NetworkData.h>
#include <TkUtil/UserData.h>
#include <TkUtil/RemoteProcess.h>
#include <TkUtil/UTF8String.h>

#include <assert.h>
#include <string.h>
#include <stdlib.h>		// getenv
#include <memory>


USING_STD
USING_UTIL


BEGIN_NAMESPACE_UTIL


static const	Charset	charset ("àéèùô");


string									LocalNetwork::_homeCommand ("home");	// v 6.1.0
vector<LocalNetwork::HPComputer>		LocalNetwork::_computers;				// v 5.9.0


LocalNetwork::LocalNetwork ( )
{
	assert (0 && "LocalNetwork constructor is not allowed.");
}	// LocalNetwork::LocalNetwork ( )


LocalNetwork::LocalNetwork (const LocalNetwork&)
{
	assert (0 && "LocalNetwork copy constructor is not allowed.");
}	// LocalNetwork::LocalNetwork (const LocalNetwork&)


LocalNetwork& LocalNetwork::operator = (const LocalNetwork&)
{
	assert (0 && "LocalNetwork assignment operator is not allowed.");

	return *this;
}	// LocalNetwork::operator =


LocalNetwork::~LocalNetwork ( )
{
	assert (0 && "LocalNetwork destructor is not allowed.");
}	// LocalNetwork::~LocalNetwork


void LocalNetwork::initialize (const vector<LocalNetwork::HPComputer>& computers)	// v 5.9.0
{
	_computers		= computers;
}	// LocalNetwork::initialize


LocalNetwork::HPComputer LocalNetwork::getComputerInfos (const string& name)		// v 5.9.0
{
	vector<LocalNetwork::HPComputer>::iterator it	= _computers.begin ( );
	for ( ; _computers.end ( ) != it; it++)
	{
		if (name == (*it).name)
		{
			try
			{	// v 5.10.0 : nom qualifié local
				if (true == (*it).localName.empty ( ))	// Recherche (1 fois) du nom local et qualifié si possible :
				{
					unique_ptr<RemoteProcess>	remoteLS (new RemoteProcess (name, "ls"));
					remoteLS->execute (false);
					// Eviter l'affichage de ls sur stdout :
					remoteLS->enableChildToSonCommunications (false);
					remoteLS->wait ( );
					// Récupération du nom qualifié d'un noeud :
					(*it).localName	= remoteLS->getHostName ( );
				}
			}
			catch (...)
			{
			}
			
			return *it;
		}
	}	// for ( ; _computers.end ( ) != it; it++)
	
	UTF8String		error (charset);
	error << "Absence d'information sur le calculateur \"" << name << "\". Calculateurs recensés : ";
	for (vector<LocalNetwork::HPComputer>::const_iterator it = _computers.begin ( ); _computers.end ( ) != it; it++)
	{
		if (_computers.begin ( ) != it)
			error << ", ";
		error << (*it).name;
	}	// for (vector<LocalNetwork::HPComputer>::const_iterator it = _computers.begin ( ); _computers.end ( ) != it; it++)
	throw Exception (error);
}	// LocalNetwork::getComputerInfos


string LocalNetwork::getFullName (const string& name)
{
	return name;
}	// LocalNetwork::getFullName


string LocalNetwork::getEndPoint (const string& name, const string& endPointSuffix)	// v 5.10.0
{
	if (true == NetworkData::isValidInetAddress (name))
		return name;
		
	if (true == endPointSuffix.empty ( ))
		return name;

	// Actuellement on se connecte au serveur sur l'interface suffixée de inter.
	// node.domain -> nodeinter.domain
	const UTF8String	computer (name, charset);
	const size_t		dot	= computer.find ('.');
	if ((size_t)-1 == dot)
		return name;
		
	UTF8String		endpoint (computer.substring (0, dot - 1), charset);
	endpoint << endPointSuffix << computer.substring (dot);
	return endpoint.utf8 ( );
}	// LocalNetwork::getEndPoint


string LocalNetwork::getEndPoint (const string& name)	// v 3.34.0
{
	if (false == isComputer (name))
		return name;

	// On récupère les infos sur le calculateur demandé :
	LocalNetwork::HPComputer	infos	= LocalNetwork::getComputerInfos (name);

	return LocalNetwork::getEndPoint (name, infos.endPointSuffix);
}	// LocalNetwork::getEndPoint


string LocalNetwork::getNATedEndPoint (const string& name)	// v 5.10.0
{
	if (false == isComputer (name))
		return name;
		
		// On récupère les infos sur le calculateur demandé :
	LocalNetwork::HPComputer	infos	= LocalNetwork::getComputerInfos (name);
	if (true == infos.endPointSuffix.empty ( ))
		return name;
		
	vector<LocalNetwork::HPComputer>::iterator it	= _computers.begin ( );
	for ( ; _computers.end ( ) != it; it++)
	{
		if (name == (*it).name)
		{
			if (true == (*it).endPointSuffix.empty ( ))
				return name;
				
			try
			{	// v 5.10.0 : nom qualifié local
				if (true == (*it).localName.empty ( ))	// Recherche (1 fois) du nom local et qualifié si possible :
				{
					unique_ptr<RemoteProcess>	remoteLS (new RemoteProcess (name, "ls"));
					remoteLS->execute (false);
					// Eviter l'affichage de ls sur stdout :
					remoteLS->enableChildToSonCommunications (false);
					remoteLS->wait ( );
					// Récupération du nom qualifié d'un noeud :
					(*it).localName	= remoteLS->getHostName ( );
				}
			}
			catch (...)
			{
			}
			
			const string	n	= true == (*it).localName.empty ( ) ? (*it).name : (*it).localName;
			return LocalNetwork::getEndPoint (n, (*it).endPointSuffix);
		}
	}	// for ( ; _computers.end ( ) != it; it++)
	
	UTF8String		error (charset);
	error << "Absence d'information sur le calculateur \"" << name << "\". Calculateurs recensés : ";
	for (vector<LocalNetwork::HPComputer>::const_iterator it = _computers.begin ( ); _computers.end ( ) != it; it++)
	{
		if (_computers.begin ( ) != it)
			error << ", ";
		error << (*it).name;
	}	// for (vector<LocalNetwork::HPComputer>::const_iterator it = _computers.begin ( ); _computers.end ( ) != it; it++)
	throw Exception (error);

}	// LocalNetwork::getNATedEndPoint


bool LocalNetwork::isComputer (const string& name)
{
	for (vector<LocalNetwork::HPComputer>::const_iterator it = _computers.begin ( ); _computers.end ( ) != it; it++)
	{
		if (string::npos != (*it).name.find (name))
			return true;
	}	// for (vector<LocalNetwork::HPComputer>::const_iterator it = _computers.begin ( ); _computers.end ( ) != it; it++)

	return false;
}	// LocalNetwork::isComputer


string LocalNetwork::getMachineName (const string& name)
{
	// Un nom de machine est nécessairement ASCII :
	UTF8String		ustr (name.c_str ( ), charset);
	UTF8String		machineName (charset);
	size_t			length	= ustr.length ( );
	size_t			dotPos	= ustr.find ('.');
	if ((size_t)-1 == dotPos)
		machineName	= name;
	else if (0 == dotPos)
		machineName	= "";
	else
		machineName	= ustr.substring (0, dotPos - 1);
	machineName	= machineName.trim ( );

	return machineName;
}	// LocalNetwork::getMachineName


string LocalNetwork::getComputerName (const string& name)
{
	// Un nom de machine est nécessairement ASCII :
	UTF8String		ustr (name.c_str ( ), charset);
	UTF8String		machineName (charset);
	size_t			length	= ustr.length ( );
	size_t			dotPos	= ustr.find ('.');
	if ((size_t)-1 == dotPos)
	{
		machineName	= name;
	}	// if ((size_t)-1 == dotPos)
	else if (dotPos == length - 1)	// nom.
	{
		machineName	= ustr.substring (0, dotPos);
	}	// else if (dotPos == length - 1)
	else
	{
		// On se positionne après le point :
		dotPos++;
		size_t	secondDotPos	= dotPos >= length ?	 (size_t)-1 : ustr.find ((char)'.', dotPos);
		machineName	= ustr.substring (dotPos + 1, secondDotPos);
	}
	machineName	= machineName.trim ( );

	// A machineName on enlève un éventuel numéro de noeud :
	UTF8String		computerName (charset);
	string			asciiMachineName	= machineName.ascii ( );
	for (size_t i = 0; i < asciiMachineName.length ( ); i++)
	{
		if (0 != isalpha (asciiMachineName [i]))
			computerName << asciiMachineName [i];
	}	// for (size_t i = 0; i < asciiMachineName.length ( ); i++)

	computerName	= computerName.trim ( );	
	if (true == isComputer (computerName))
		return computerName;

	return "";
}	// LocalNetwork::getComputerName


string LocalNetwork::getInetAddress (const string& name)	// v 3.1.1
{
	NetworkData	machine (name);
	vector<string>	addresses	= machine.getInetAddr ( );

	for (vector<string>::const_iterator it = addresses.begin ( );
	     addresses.end ( ) != it; it++)
	{
		try
		{
			NetworkData	machineByIP	= NetworkData::getMachineInfos (*it);

			return *it;
		}
		catch (...)
		{
			cout << "LocalNetwork::getInetAddress. Adresse IP " << *it << " invalide pour accéder à la machine " << name
			     << " depuis cette machine (" << NetworkData ( ).getFullHostName ( ) << ")" << endl;
		}
	}	// for (vector<string>::const_iterator it = addresses.begin ( );

	UTF8String	message (charset);
	message << "La machine " << name << " n'a pas d'adresse IP accessible depuis cette machine (" << NetworkData ( ).getFullHostName ( ) << ")";
	throw Exception (message);
}	// LocalNetwork::getInetAddress


string LocalNetwork::getUserHome (const string& computerName)	// Réécriture en 5.2.1
{
	if (NetworkData::getLocalHost( ).is (computerName))
		return UserData ( ).getHome ( );

	string	home;
	int 	status	= 0;
	// Pourquoi RemoteProcess ("echo") avec getOptions.addOption ("$HOME") ne fonctionne pas sous ubuntu ???
	unique_ptr<RemoteProcess>	pwdProcess (new RemoteProcess (computerName, "pwd"));
	bool	ok	= false;
	try
	{
		pwdProcess->enableChildToSonCommunications (true);
		pwdProcess->execute (false);
		// v 5.7.5 : recherche de la ligne home dans le flux :
		while (false == ok)	// v 5.7.5
		{
			home   = pwdProcess->getChildLine ( );
			File	path (home);
			if (path.getFileName ( ) == UserData ( ).getName ( ))
				ok	= true;			
		}	// while (false == ok)
		if (0 != (status = pwdProcess->getCompletionCode ( )))
			status = pwdProcess->wait ( );
	}
	catch (...)
	{
		status = -1;
	}

	if (true == ok)	// even if status != 0
		return home;
		
	if (-1 == status)
	{
		UTF8String	error (charset);
		error << "Echec de LocalNetwork::getUserHome (" << computerName << ") :\n" << pwdProcess->getErrorMessage ( );
		throw Exception (error);
	}

	return home;
}	// LocalNetwork::getUserHome


bool LocalNetwork::isRemoteHost ( )
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
}	// LocalNetwork::isRemoteHost


string LocalNetwork::getHomeCommand ( )
{
	return _homeCommand;	// v 6.1.0
}	// LocalNetwork::getHomeCommand


void LocalNetwork::setHomeCommand (const string& command)	// v 6.1.0
{
	_homeCommand	= command;
}	// LocalNetwork::setHomeCommand

END_NAMESPACE_UTIL
