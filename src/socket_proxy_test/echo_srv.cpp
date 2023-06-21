//
// Serveur TCP à l'écoute sur un port transmis en 1er argument.
// Retourne les chaines de caractères reçues sur ce port.
//

// /opt/bin/echo_srv-debug 11111
// Noeud de login toto du calculateur :
// socket_proxy -connect toto:11111 -listen 20000 -bufferSize 5000
// Client sur station :
// echo_clt toto 20000


#include <TkUtil/NetworkData.h>
#include <TkUtil/NumericConversions.h>

#include <iostream>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

using namespace TkUtil;
using namespace std;


int main (int argc, char* argv[], char* envp[])
{
	const int	port	= 2 != argc ?
						  -1 : NumericConversions::strToLong (argv [1]);
	if (0 >= port)
	{
		cerr << "Syntax : " << argv [0] << " port_tcp" << endl;
		return -1;
	}	// if (0 >= port)

	int					descriptor	= socket (AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in	addr;
	memset (&addr, 0, sizeof (struct sockaddr_in));
	addr.sin_family	= AF_INET;
	addr.sin_port	= htons (port);
	addr.sin_addr.s_addr	= htonl (INADDR_ANY);

	if ((descriptor < 0) ||
	    (0 != bind (descriptor, (const struct sockaddr*)&addr,
		            sizeof (struct sockaddr_in))) ||
	    (-1 == listen (descriptor, 1)))
	{
		cerr << "Could not open a listen socket on port " << port << "." <<endl;
		return -1;
	}	// if ((descriptor < 0) || ...

	NetworkData		host;
	const string	srvInfos	= host.getNetworksIdentifications ( );
	cout << "Informations serveur : " << endl << srvInfos << endl;

	cout << "Server is listening on port " << port << " of machine "
	     << host.getFullHostName ( ) << " for incomming connections ..."
	     << endl;

	struct sockaddr_in	clientAddr;
	socklen_t			length	= sizeof (sockaddr_in);
	const int clientDescriptor	=
				accept (descriptor, (struct sockaddr*)&clientAddr, &length);
	if (-1 == clientDescriptor)
	{
		cerr << "Connexion with client failed." << endl;
		return -1;
	}	// if (-1 == clientDescriptor)
	cout << "Connected with client." << endl;
	close (descriptor);		descriptor	= -1;

	// Boucle ~infinie réception message/retour en écho.
	bool	completed	= false;
	while (false == completed)
	{
		char buffer [2049];
		memset (buffer, '\0', 2049);
		int	l	= 0;
		if (-1 == recv (clientDescriptor, &l, sizeof (int), 0))
			continue;

		ssize_t	dataRead	= 0;
		while (dataRead < l)
		{
		    ssize_t	read	=
				recv (clientDescriptor, buffer + dataRead, l - dataRead, MSG_WAITALL);
			if (-1 == read)
			{
				cerr << "Error during message reception." << endl;
				close (clientDescriptor);
				return -1;
			}	// if (-1 == recv (clientDescriptor, buffer, l, 0))
			dataRead	+= read;
		}	// while (dataRead < l)
		if (0 != strlen (buffer))
			cout << "REC:" << buffer << endl;

		// On envoie un A/R au client :
		char	buffer2 [2049];
		memset (buffer2, '\0', 2049);
		sprintf (buffer2, "SRV HAS RECEIVED : %s", buffer);
		const int	length	= strlen (buffer2);
		send (clientDescriptor, &length, sizeof (int), 0);
		send (clientDescriptor, buffer2, strlen (buffer2), 0);

		if (string ("shutdown") == buffer)
		{
			cout << "Shutting down server." << endl;
			completed	= true;
		}	// if (string ("shutdown") == buffer)
	}	// while (false == completed)
	close (clientDescriptor);

	return 0;
}	// main

