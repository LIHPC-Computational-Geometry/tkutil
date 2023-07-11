#include <TkUtil/Exception.h>
#include <TkUtil/File.h>
#include <TkUtil/NetworkData.h>
#include <TkUtil/Process.h>
#include <TkUtil/NumericConversions.h>
#include <TkUtil/UTF8String.h>

#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>

#define BUFFER_SIZE 5000000

USING_UTIL

static const Charset	charset ("àéèùô");

static bool parseArgs (int argc, char* argv []);
static int syntax (const std::string& pgmNamme);
static void transfer (int from, int to);

static std::string	readyMessage;
static std::string	srvHost;
static int		srvPort = -1, listenPort = -1;
static int		srvType = AF_INET, listenType = AF_INET;
static File		srvUnixFile (""), cltUnixFile ("");
static char*	buffer	= 0;
static size_t	bufferSize	= BUFFER_SIZE;
static bool		debug	= false;


int main (int argc, char* argv [], char* envp[])
{

	try
	{
		Process::initialize (argc, argv, envp);

		if (false == parseArgs (argc, argv))
			return syntax (argv [0]);

		buffer	= new char [bufferSize + 1];
		memset (buffer, 0, bufferSize + 1);

		// Connexion au serveur :
		errno		= 0;
		int	srvDesc	= -1;
		switch (srvType)
		{
			case AF_INET	:
			{
				std::cout << "Etablissement de la connexion avec le serveur sur le "
			     	<< "port " << srvPort << " de la machine " << srvHost
				    << " ..." << std::endl;
				const struct hostent*	ipserv= gethostbyname(srvHost.c_str( ));
				if ((0 == ipserv) || (0 == ipserv->h_addr_list))
				{
					std::cerr << "Inpossibilité de localiser la machine "
					     << srvHost << "." << std::endl;
					return -1;
				}	// if ((0 == ipserv) || (0 == ipserv->h_addr_list))
				struct sockaddr_in	addr;
				memset (&addr, 0, sizeof (struct sockaddr_in));
				addr.sin_family	= ipserv->h_addrtype;
				addr.sin_port	= htons (srvPort);
				memcpy ((char*)&addr.sin_addr.s_addr, ipserv->h_addr,
				        ipserv->h_length);
				srvDesc	= socket (AF_INET, SOCK_STREAM, 0);
				if ((srvDesc < 0) ||
				    (-1 == connect (srvDesc, (const struct sockaddr*)&addr,
				                    sizeof (struct sockaddr_in))))
				{
					std::cerr << "Impossibilité de se connecter au serveur sur "
					     << "le port " << srvPort << " de la machine "
					     << srvHost << " : " << strerror (errno) << std::endl;
					return -1;
				}	// if ((srvDesc < 0) || ...
			}	// case AF_INET
			break;
			case AF_UNIX	:
			{
				std::cout << "Etablissement de la connexion avec le serveur sur le "
				     << "socket Unix " << srvUnixFile.getFullFileName ( ) << "."
				     << std::endl;
				struct sockaddr_un	addr;
				memset (&addr, 0, sizeof (struct sockaddr_un));
				addr.sun_family	= AF_UNIX;
				strcpy (addr.sun_path, srvUnixFile.getFullFileName( ).c_str( ));
				srvDesc	= socket (AF_UNIX, SOCK_STREAM, 0);
				if ((srvDesc < 0) ||
				    (-1 == bind (srvDesc, (const struct sockaddr*)&addr,
				                    sizeof (struct sockaddr_un))))
				{
					std::cerr << "Impossibilité de se connecter au serveur sur "
					     << "le socket Unix " << srvUnixFile.getFullFileName ( )
					     << " : " << strerror (errno) << std::endl;
					return -1;
				}	// if ((srvDesc < 0) || ...
			}
			break;	// case AF_UNIX 
			default	:
				std::cerr << "Impossibilité de se connecter au serveur, type "
				     << "de socket non traité (" << srvType << ")." <<std::endl;
				return -1;
		}	// switch (srvType)
		// On se met en mode non bloquant en écoute :
		errno	= 0;
		const int	srvFlags	= fcntl (srvDesc, F_GETFL);
		const int	srvRes		= fcntl (srvDesc, F_SETFL, srvFlags|O_NONBLOCK);
		if (-1 == srvRes)
		{
			std::cerr << "Impossibilité de recevoir en mode non bloquant des "
			     << "données du serveur : " << strerror (errno) << std::endl;
			return -1;
		}	// if (-1 == srvRes)
		std::cout << "Connexion au serveur effectuée." << std::endl;

		// Attente d'un client à relier au serveur :
		int	listenDesc	= -1;
		switch (listenType)
		{
			case AF_INET :
			{
				struct sockaddr_in	addr;
				std::cout << "Ouverture d'un port d'écoute sur le port "
				     << listenPort << " ..." << std::endl;
				memset (&addr, 0, sizeof (struct sockaddr_in));
				addr.sin_family			= AF_INET;
				addr.sin_port			= htons (listenPort);
				addr.sin_addr.s_addr	= htonl (INADDR_ANY);
				listenDesc	= socket (listenType, SOCK_STREAM, 0);
				if ((listenDesc < 0) ||
				    (0 != bind (listenDesc, (const struct sockaddr*)&addr,
		                    sizeof (struct sockaddr_in))) ||
				    (-1 == listen (listenDesc, 1)))
				{
					std::cerr << "Impossibilité d'attendre des connections "
					      << " entrantes sur le port " << listenPort
					      << " de cette machine." << std::endl;
					return -1;
				}	// if ((listenDesc < 0) || ...
			}
			break;	// case AF_INET
			case AF_UNIX :
			{
				struct sockaddr_un	addr;
				std::cout << "Ouverture d'un port d'écoute sur le socket "
				     << " de domaine Unix " << cltUnixFile.getFullFileName ( )
                     << " ..." << std::endl;
                try
                {
                    File    socketFile (cltUnixFile.getFullFileName( ));
                    if (true == socketFile.exists ( ))
                    {
                        if(true == debug)
                            std::cout << "Destruction du fichier "
                                 << cltUnixFile.getFullFileName ( ) << ".";
                        socketFile.remove ( );
                    }
                }
                catch (const Exception& exc)
                {
                    std::cerr << "Echec de la destruction du fichier "
                         << cltUnixFile.getFullFileName ( ) << " : "
                         << exc.getFullMessage ( ) << std::endl;
                }
                catch (...)
                {
                    std::cerr << "Echec de la destruction du fichier "
                         << cltUnixFile.getFullFileName ( ) << " : "
                         << "erreur non documentée." << std::endl;
                }
				memset (&addr, 0, sizeof (struct sockaddr_un));
				addr.sun_family			= AF_UNIX;
				strcpy (addr.sun_path, cltUnixFile.getFullFileName( ).c_str( ));
				listenDesc	= socket (AF_UNIX, SOCK_STREAM, 0);
				if ((listenDesc < 0) ||
				    (0 != bind (listenDesc, (const struct sockaddr*)&addr,
		                    sizeof (struct sockaddr_un))) ||
				    (-1 == listen (listenDesc, 1)))
				{
					std::cerr << "Impossibilité d'attendre des connections "
					      << " entrantes sur le socket UNIX "
					      << cltUnixFile.getFullFileName ( ) 
					      << " de cette machine : " << strerror (errno)
					      << std::endl;
					return -1;
				}	// if ((listenDesc < 0) || ...
			}
			break;	// case AF_UNIX
			default	:
				std::cerr << "Impossibilité d'attendre des connections "
				     << "entrantes, type de socket non traité (" << listenType
				     << ")." << std::endl;
				return -1;
		}	// switch (listenType)

		NetworkData			host;
		const std::string	proxyInfos	= host.getNetworksIdentifications ( );
		std::cout << "Informations proxy : " << std::endl << proxyInfos
		          << std::endl;

		std::cout << "Proxy sur la machine " << host.getFullHostName ( )
		     << " en attente de connexion au serveur (";
		switch (srvType)
		{
			case AF_INET	:
				std::cout << srvHost << ":" << srvPort << ") ";
				break;
			case AF_UNIX	:
				std::cout << "socket UNIX " << srvUnixFile.getFullFileName ( )
				     << ") ";
				break;
		}	// switch (srvType)
		switch (listenType)
		{
			case AF_INET	:
				std::cout << "sur le port " << listenPort << std::endl;
				break;
			case AF_UNIX	:
				std::cout << "sur le socket UNIX "
				          << cltUnixFile.getFullFileName ( ) << "."
				          << std::endl;
				break;
		}	// switch (listenType)

        // Connexion avec le client :
        errno		   	= 0;
        int 	cltDesc = -1;
		char	cltBuffer [2049];
        switch (listenType)
        {
            case AF_INET    :
            {
	        	struct sockaddr_in	cltAddr;
        		socklen_t			length		= sizeof (sockaddr_in);	
        		cltDesc	= accept (
						listenDesc, (struct sockaddr*)&cltAddr, &length);
				if (-1 == cltDesc)
					break;

				// On récupère autant que possible la provenance du client :
				sprintf (cltBuffer, "%s", inet_ntoa (cltAddr.sin_addr));
				in_addr_t	laddr	= inet_addr (cltBuffer);
				if (INADDR_NONE != laddr)
				{
					struct hostent*	hp	=
						gethostbyaddr ((char*)&laddr,sizeof(in_addr_t),AF_INET);
					if ((0 != hp) && (0 != hp->h_name))
						sprintf (cltBuffer, "%s:%d", hp->h_name,
												(int)ntohs (cltAddr.sin_port));
				}
            };
            break;  // case AF_INET
            case AF_UNIX    :
            {
				/* From Unix System Programing Using C++ p 381
	        	struct sockaddr	cltAddr;
        		socklen_t		length		= sizeof (sockaddr);	*/
				// From CP, seems OK :
				sockaddr_un	cltAddr;
				socklen_t	length	 = sizeof (sockaddr_un);
        		cltDesc	= accept (
							listenDesc, (struct sockaddr*)&cltAddr, &length);
				if (-1 == cltDesc)
					break;

				// On récupère autant que possible la provenance du client :
//				strncpy (cltBuffer, cltAddr.sa_data, length);
				sprintf (cltBuffer, "%s", cltAddr.sun_path);
            };
            break;  // case AF_UNIX
			default	:
				std::cerr << "Impossibilité d'attendre des connections "
				     << "entrantes, type de socket non traité (" << listenType
				     << ")." << std::endl;
				return -1;
        }   // switch (listenType)
		if (-1 == cltDesc)
		{
			std::cerr << "Erreur de la connexion du client : "
			          << strerror (errno) << std::endl;
			return -1;
		}	// if (-1 == cltDesc)
		// On se met en mode non bloquant en éstd::coute :
		errno	= 0;
		const int	cltFlags	= fcntl (cltDesc, F_GETFL);
		const int	cltRes		= fcntl (cltDesc, F_SETFL, cltFlags|O_NONBLOCK);
		if (-1 == cltRes)
		{
			std::cerr << "Impossibilité de recevoir en mode non bloquant des "
			     << "données du client sur le port " << listenPort
			     << " : " << strerror (errno) << std::endl;
			return -1;
		}	// if (-1 == cltRes)
		std::cout << "Client connecté. Informations client : " << cltBuffer
		          << std::endl;
		if (false == readyMessage.empty ( ))
			std::cout << readyMessage << std::endl;

		// On n'attend plus de connection entrante :
		close (listenDesc);		listenDesc	= -1;

		// Boucle infinie de transfert des messages client <=> serveur
		bool	completed	= false;
		while (false == completed)
		{
			// ==> Hyper-important, refaire selectSet et timeout à chaque appel
			// de select, sinon absence de réponse <==
			// man select
			// ...
			// On success, select() and pselect() return the number of file
			// descriptors contained in the three returned descriptor sets
			// (that is, the total number of bits that are set in readfds,
			// writefds, exceptfds) which may be zero if the timeout expires
			// before anything interesting happens.  On error, -1 is returned,
			// and errno is set appropriately; the sets and timeout become
			// undefined, so do not rely on their contents after an error.
			fd_set	selectSet;
			FD_ZERO (&selectSet);
			FD_SET (srvDesc, &selectSet);
			FD_SET (cltDesc, &selectSet);
			const int	answer	= select (FD_SETSIZE, &selectSet, 0, 0, NULL);
			if (0 < answer)
			{
				const bool	clt2Srv	= 0 != FD_ISSET (cltDesc, &selectSet);
				const bool	srv2Clt	= 0 != FD_ISSET (srvDesc, &selectSet);
				if (true == clt2Srv)
				{
					if (true == debug)
						std::cout << argv [0] << " SELECT => client vers serveur"
						     << std::endl;
					transfer (cltDesc, srvDesc);
					if (true == debug)
						std::cout << argv [0]
						     << " Transfert client vers serveur effectué"
						     << std::endl;
				}	// if (true == clt2Srv)
				if (true == srv2Clt)
				{
					if (true == debug)
						std::cout << argv [0]
					              << " SELECT => serveur vers client"
						     << std::endl;
					transfer (srvDesc, cltDesc);
					if (true == debug)
						std::cout << argv [0]
						     << " Transfert serveur vers client effectué"
						     << std::endl;
				}	// if (true == srv2Clt)
			}	// if (0 < answer)
		}	// while (false == completed)
	}
	catch (const Exception& exc)
	{
		std::cout << "Exception caught : " << exc.getFullMessage ( )
		          << std::endl;
		return -1;
	}
	catch (const std::exception& e)
	{
		std::cout << "Standard exception caught : " << e.what ( ) << std::endl;
		return -1;
	}
	catch (...)
	{
		std::cout << "Undocumented exception caught." << std::endl;
		return -1;
	}

	return 0;
}	// main


static bool parseArgs (int argc, char* argv [])
{
	for (int i = 1; i < argc; i++)
	{
		bool	processed	= true;

		if (i < argc - 1)
		{
			if (std::string ("-connect") == argv [i])
			{
				const std::string		host (argv [i + 1]);
				std::string				sport;
				std::string::size_type	comma	= host.find (':');
				if (std::string::npos != comma)
				{
					srvHost	= host.substr (0, comma);
					sport	= host.substr (comma + 1);
					if ((0 == srvHost.length ( )) || (0 == sport.length ( )))
					{
						UTF8String	error (charset);
						error << "Adresse invalide de serveur auquel se "
						      << "connecter : " << argv [i + 1] << ".";
						throw Exception (error);
					}	// if ((0 == srvHost.length ( )) || ...
					srvPort	= NumericConversions::strToULong (sport);
				}	// if (std::string::npos != comma)
				else
				{
					srvType		= AF_UNIX;
					srvUnixFile	= File (host);
					if ((false == srvUnixFile.exists ( )) ||
					    (false == srvUnixFile.isReadable ( )))
					{
						UTF8String	error (charset);
						error << "Adresse invalide de serveur auquel se "
						      << "connecter : " << argv [i + 1]
						      << " (socket UNIX inexistant ou accès en lecture "
						      << "interdit).";
						throw Exception (error);
					}	// if ((false == srvUnixFile.exists ( )) || ...
				}	// else if (std::string::npos != comma)
			}	// if (std::string ("-connect") == argv [i])
			else if (std::string ("-listen") == argv [i])
			{
				try
				{
					listenPort	= NumericConversions::strToULong (argv [i + 1]);
				}
				catch (...)
				{
					listenType		= AF_UNIX;
					cltUnixFile	= File (argv [i + 1]);
					if (false == cltUnixFile.isWritable ( ))
					{
						UTF8String	error (charset);
						error << "Port ou socket UNIX invalide (droits en "
						      << "écriture interdits) pour éstd::couter des "
						      << "connexions clientes : " << argv [i + 1] <<".";
						throw Exception (error);
					}	// if (false == cltUnixFile.isWritable ( ))
				}
			}	// else if (std::string ("-listen") == argv [i])
			else if (std::string ("-bufferSize") == argv [i])
			{
				bufferSize	= NumericConversions::strToULong (argv [i + 1]);
			}	// else if (std::string ("-bufferSize") == argv [i])
			else if (std::string ("-message") == argv [i])
			{
				readyMessage	= argv [i + 1];
			}	// else if (std::string ("-message") == argv [i])
			else
				processed	= false;
		}	// if (i < argc - 1)
		else
			processed	= false;

		if (true == processed)
		{
			i	+= 1;
			continue;
		}	// if (true == processed)

		if (std::string ("-debug") == argv [i])
		{
			debug	= true;
		}	// else if (std::string ("-message") == argv [i])
		else
		{
			UTF8String	error (charset);
			error << "Argument non traité : " << argv [i] << ".";
			throw Exception (error);
		}
	}	// for (int i = 1; i < argc; i++)

	if ((0 != (srvHost.length ( )) && (srvPort <= 0)) ||
        ((0 == srvHost.length ( )) &&
							(0 == srvUnixFile.getFileName ( ).length ( ))))
        return false;

	if ((listenPort <= 0) && (0 == cltUnixFile.getFileName ( ).length ( )))
		return false;

	return true;
}	// parseArgs


static int syntax (const std::string& pgmName)
{
	std::cout << "Syntaxe : " << pgmName
	     << " [-connect machine:port] [-connect file] "
	     << "[-listen port] [-listen file] "
	     << "[-bufferSize size] [-message message][-debug]"
	     << std::endl
	     << "machine:port              : machine et port où le serveur est à l'éstd::coute"
	     << std::endl
	     << "file                      : fichier pour socket de domaine unix (connexion serveur ou éstd::coute de client) "
	     << std::endl
	     << "port                      : port sur lequel se mettre à l'éstd::coute"
	     << std::endl
	     << "bufferSize                : taille du tampon utilisé pour les opérations de lecture/écriture de sockets (défaut : " << BUFFER_SIZE << " octets)"
	     << std::endl
	     << "message                   : chaîne écrite sur la sortie standard lorsque la connection entre client et serveur est établie"
	     << std::endl;

	return -1;
}	// syntax


static void transfer (int from, int to)
{
	// NB : les tampons peuvent être de tailles différentes entre client et
	// serveur => on lit et renvoit les infos en plusieurs passes si nécessaire.
	// Il peut y avoir temporairement des problèmes réeseau => on retente tant
	// qu'il faut car sinon select ne sera pas déclenché et on risque de se
	// retrouver dans une situation où client et serveur sont en attente
	// d'infos ...
	bool	completed	= false;
	while (false == completed)
	{
		memset (buffer, 0, bufferSize + 1);
		errno	= 0;
		ssize_t	received = recv (from, buffer, bufferSize, 0);
		if (-1 == received)
		{
			if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
			{	// Récupérable, erreur a priori temporaire
				completed = true;
				received  = 0;
				if (true == debug)
					std::cout << "Echec réception de données, nouvelle "
					          << "tentative." << std::endl;
			}	// if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
			else
			{	// Erreur irrécupérable
				std::cerr << "Erreur durant la réception de données :"
				     << std::endl << "ERRNO=" << errno << std::endl
				      << strerror (errno) << std::endl;
			}	// else if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
			break;	// C'est mal engagé ...
		}	// if (-1 == received)

		ssize_t	sent	= 0;
		errno	= 0;
		while (sent < received)
		{
			ssize_t	s	= send (to, buffer + sent, received - sent, 0);

			if (-1 == s)
			{
                if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
                {
					if (true == debug)
						std::cout << "Echec réception de données, nouvelle "
						     << "tentative." << std::endl;
//					sleep (1);	// Surtout pas, ça va être très très lent
					s    = 0;
				}	// if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
                else
                {
					std::cerr << "Erreur durant l'envoi de données :"
					     << std::endl << strerror (errno) << std::endl;
					break;
                }	// else if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
			}	// if (-1 == s)
			sent	+= s;
		}	// while (sent < received)
	}	// while (false == completed)
}	// transfer
