#include "TkUtil/File.h"
#include "TkUtil/InternalError.h"
#include "TkUtil/UTF8String.h"
#include "TkUtil/Exception.h"


#include <sys/param.h>		// MAXPATHLEN
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <vector>


USING_STD


BEGIN_NAMESPACE_UTIL

static const Charset	charset ("àéèùô");


static string getTmpDir ( )
{
	char*	tmpDir	= ::getenv ("TMPDIR");
	if (NULL != tmpDir)
		return tmpDir;

	return "/tmp";
}	// getTmpDir


File::File (const string& fullFileName)
	: _fullName (fullFileName)
{
}	// File::File (const string&)


File::File (const File& file)
	: _fullName (file._fullName)
{
}	// File::File (const File&)


File& File::operator = (const File& file)
{
	if (&file != this)
		_fullName	= file.getFullFileName ( );

	return *this;
}	// File::operator =


File::~File ( )
{
}	// File::~File


void File::setFullFileName (const string& fullFileName)
{
	_fullName	= fullFileName;
}	// File::setFullFileName


string File::getFileName ( ) const
{
	string::size_type	separator	= getFullFileName ( ).rfind ('/');

	if (string::npos == separator)
		return getFullFileName ( );

	return getFullFileName ( ).substr (separator + 1);
}	// File::getFileName


File File::getPath ( ) const
{
	string::size_type	separator	= getFullFileName ( ).rfind ('/');

	if (string::npos == separator)
		return File (".");

	return getFullFileName ( ).substr (0, separator);
}	// File::getPath


const string File::getExtension ( ) const
{
	string::size_type	lastDot	= getFileName ( ).rfind ('.');
	if (string::npos == lastDot)
		return "";

	return getFileName ( ).substr (lastDot + 1);
}	// File::getExtension


bool File::exists ( ) const
{
	struct stat		stats;

	errno		= 0;
	int	retval	= ::stat (getFullFileName ( ).c_str ( ), &stats);

	if (0 == retval)
		return true;

	if (ENOENT == errno)
		return false;

	UTF8String	errorMsg (charset);
	errorMsg << "Erreur lors du test d'existence du fichier " << getFullFileName ( ) << " :\n" << strerror (errno);
	errno	= 0;
	throw Exception (errorMsg);
}	// File::exists


bool File::isRegularFile ( ) const
{
	struct stat		stats;

	errno		= 0;
	int	retval	= ::stat (getFullFileName ( ).c_str ( ), &stats);

	if (0 != retval)
		return false;

	if (S_IFREG == (stats.st_mode & S_IFMT))
		return true;

	return false;
}	// File::isRegularFile


bool File::isDirectory ( ) const
{
	struct stat		stats;

	errno		= 0;
	int	retval	= ::stat (getFullFileName ( ).c_str ( ), &stats);

	if (0 != retval)
		return false;

//	if (0 != (stats.st_mode & S_IFDIR))	// v 3.2.0
	if (S_IFDIR == (stats.st_mode & S_IFMT))	// v 3.2.0
		return true;

	return false;
}	// File::isDirectory


bool File::isLink ( ) const
{
	struct stat		stats;

	errno		= 0;
	int	retval	= ::lstat (getFullFileName ( ).c_str ( ), &stats);

	if (0 != retval)
		return false;

	return (0 != S_ISLNK (stats.st_mode) ? true : false);
}	// File::isLink


bool File::isFifo ( ) const
{
	struct stat		stats;

	errno		= 0;
	int	retval	= ::stat (getFullFileName ( ).c_str ( ), &stats);

	if (0 != retval)
		return false;

	if (S_IFIFO == (stats.st_mode & S_IFMT))
		return true;

	return false;
}	// File::isFifo


bool File::isSocket ( ) const
{
	struct stat		stats;

	errno		= 0;
	int	retval	= ::stat (getFullFileName ( ).c_str ( ), &stats);

	if (0 != retval)
		return false;

	if (S_IFSOCK == (stats.st_mode & S_IFMT))
		return true;

	return false;
}	// File::isSocket

bool File::isReadable ( ) const
{
	return (0 == ::access (getFullFileName ( ).c_str ( ), R_OK) ? true : false);
}	// File::isReadable


bool File::isWritable ( ) const
{
	if (true == exists ( ))
		return (0 == ::access (getFullFileName( ).c_str( ), W_OK) ? true:false);

	File	path	= getPath ( );
	return (0 == ::access(path.getFullFileName( ).c_str( ), W_OK) ? true:false);
}	// File::isWritable


bool File::isExecutable ( ) const
{
	return (0 == ::access (getFullFileName ( ).c_str ( ), X_OK) ? true : false);
}	// File::isExecutable


mode_t File::getAccessRights ( ) const	// v 5.11.0
{
	errno	= 0;
	struct stat	stats;
	if (0 != stat (getFullFileName ( ).c_str ( ), &stats))
	{
		UTF8String	error (charset);
		error << "Impossibilité d'obtenir les droits d'accès sur le fichier ou répertoire " << getFullFileName ( ) << " :\n"
		      << strerror (errno);
		errno	= 0;
		throw Exception (error);
	}	// if (0 != stat (getFullFileName ( ).c_str ( ), &stats))
	
	return stats.st_mode;
}	// File::getAccessRights


void File::setAccessRights (mode_t rights)	// v 5.11.0
{
	errno	= 0;
	if (0 != chmod (getFullFileName ( ).c_str ( ), rights))
	{
		UTF8String	error (charset);
		error << "Impossibilité de modifier les droits d'accès du fichier ou répertoire " << getFullFileName ( ) << " :\n"
		      << strerror (errno);
		errno	= 0;
		throw Exception (error);
	}	// if (0 != chmod (getFullFileName ( ).c_str ( ), rights))
}	// File::setAccessRights


void File::create (bool asFile)
{
	// Si le fichier existe on vérifie qu'il est de même nature que ce qui est demandé.
	if ((true == exists ( )) && (isDirectory ( ) == asFile))
	{
		UTF8String	message (charset);
		message << "Impossibilité de créer le " << (true == asFile ? "fichier" : "répertoire") << " "
		        << getFullFileName ( ) << " :" << '\n' << "Celui-ci existe déjà et est un "
		        << (true == isDirectory ( ) ? "répertoire" : "fichier") << ".";
		throw Exception (message);
	}	// if ((true == exists ( )) && ...
	if (true == exists ( ))
		return;

	try
	{
		// Créer l'éventuelle arborescence parente :
		File	parent (getPath ( ));
		if (false == parent.exists ( ))
			parent.create (false);

		// Création du fichier/répertoire :
		errno	= 0;
		const mode_t	permissions	= S_IRWXU;
		const mode_t	currentMask	= umask (0);
		const mode_t	mask		= permissions & ~currentMask;
		umask (currentMask);
		if (true == asFile)
		{
			int	descriptor	= ::open (getFullFileName ( ).c_str ( ), O_RDWR |O_CREAT | O_TRUNC, mask);	// v 5.13.2 : + O_RDWR | O_CREAT | O_TRUNC
			if (-1 == descriptor)
				throw Exception (UTF8String (strerror (errno), charset));

			if (0 != ::close (descriptor))
				throw Exception (UTF8String (strerror (errno), charset));
		}	// if (true == asFile)
		else
		{
			if (0 != ::mkdir (getFullFileName ( ).c_str ( ), mask))
				throw Exception (UTF8String (strerror (errno), charset));
		}	// else if (true == asFile)

		// Bogue avec les fichiers, ouvertures type fopen (name, "w") impossible (pb de permissions). Le chmod ci-dessous contourne ce bogue.
		// Test valeur de retour sur conseil coverity.
		errno	= 0;
		if (0 != chmod (getFullFileName ( ).c_str ( ), permissions))
			throw Exception (UTF8String (strerror (errno), charset));
	}
	catch (const Exception& e)
	{
		UTF8String	message (charset);
		message << "Impossibilité de créer le " << (true == asFile ? "fichier" : "répertoire") << " "
		        << getFullFileName ( ) << " :" << '\n' << e.getFullMessage ( );
		throw Exception (message);
	}
	catch (...)
	{
		UTF8String	message (charset);
		message << "Impossibilité de créer le " << (true == asFile ? "fichier" : "répertoire") << " "
		        << getFullFileName ( ) << " :" << '\n' << "Erreur non documentée.";
		throw Exception (message);
	}
}	// File::create


void File::reset ( )
{
	if (false == exists ( ))
	{
		UTF8String	message (charset);
		message << "Impossibilité de vider le fichier " << getFullFileName ( ) << " : Celui-ci n'existe pas.";
		throw Exception (message);
	}	// if (false == exists ( ))
	if (true == isDirectory ( ))	
	{
		UTF8String	message (charset);
		message << "Impossibilité de vider le fichier " << getFullFileName ( ) << " : Celui-ci est un répertoire.";
		throw Exception (message);
	}	// if (true == isDirectory ( ))
	if (false == isWritable ( ))	
	{
		UTF8String	message (charset);
		message << "Impossibilité de vider le fichier " << getFullFileName ( ) << " : Absence de droits en écriture.";
		throw Exception (message);
	}	// if (true == isWritable ( ))

	try
	{
		errno	= 0;
		struct stat	stats;
		if (0 != ::stat (getFullFileName ( ).c_str ( ), &stats))
			throw Exception (UTF8String (strerror (errno), charset));
		int	descriptor	= ::open (getFullFileName ( ).c_str ( ), O_WRONLY | O_TRUNC);
		if (-1 == descriptor)
			throw Exception (UTF8String (strerror (errno), charset));

		if (0 != ::close (descriptor))
			throw Exception (UTF8String (strerror (errno), charset));
		// Bogue avec les fichiers, ouvertures type fopen (name, "w") impossible (pb de permissions). Le chmod ci-dessous contourne ce bogue.
		// Test valeur de retour sur conseil coverity.
		errno	= 0;
		if (0 != chmod (getFullFileName ( ).c_str ( ), stats.st_mode))
			throw Exception (UTF8String (strerror (errno), charset));
	}
	catch (const Exception& e)
	{
		UTF8String	message (charset);
		message << "Impossibilité de vider le fichier " << getFullFileName ( ) << " :" << '\n' << e.getFullMessage ( );
		throw Exception (message);
	}
	catch (...)
	{
		UTF8String	message (charset);
		message << "Impossibilité de vider le fichier " << getFullFileName ( ) << " :" << '\n' << "Erreur non documentée.";
		throw Exception (message);
	}
}	// File::reset


void File::rename (const string &newpath, bool erase)	// v 5.4.0
{	// Rem : certains paths peuvent être relatifs, d'autres absolus ...
	const File newdest (newpath);
	if (newdest.getFullFileName ( ) == getFullFileName ( ))
		return;

	if (false == newdest.isWritable ( ))
	{
		UTF8String	error (charset);
		error << "Absence de droit en écriture pour déplacer le fichier " << getFullFileName ( ) << " en "
		      << newdest.getFullFileName ( ) << ".";
		throw Exception (error);
	}	// if (false == newdest.isWritable ( ))
	if ((true == newdest.exists ( )) && (false == erase))
	{
		UTF8String	error (charset);
		error << "Impossible de déplacer le fichier " << getFullFileName ( ) << " en " << newdest.getFullFileName ( )
		      << ", ce fichier existe déjà";
		throw Exception (error);
	}	// if ((true == newdest.exists ( )) && (false == erase))

	errno	= 0;
	const int success	= ::rename (getFullFileName ( ).c_str ( ), newdest.getFullFileName ( ).c_str ( ));
	if (0 != success)
	{
		UTF8String	error (charset);
		error << "Erreur lors du déplacement du fichier " << getFullFileName ( ) << " en " << newdest.getFullFileName ( )
			  << " : " << strerror (errno);
		throw Exception (error);
	}	// if (0 != success)
}	// File::rename


void File::remove ( )
{
	if (true == exists ( ))
	{
		const	bool	isDir	= isDirectory ( );
		errno	= 0;
		try
		{
			if (true == isDir)
			{
				DIR*	dir	= ::opendir (getFullFileName ( ).c_str ( ));
				if (NULL == dir)
					throw strerror (errno);

				struct dirent*	entry	= NULL;
				vector<string>	children;
				while (NULL != (entry = readdir (dir)))
				{
					if ((0 == strcmp (".", entry->d_name)) ||
					    (0 == strcmp ("..", entry->d_name)))
						continue;

					string	child	= getFullFileName ( ) + "/" + entry->d_name;
					children.push_back (child);
				}	// while (NULL != (entry = readdir (dir)))
				::closedir (dir);

				for (vector<string>::const_iterator it = children.begin ( );
					 children.end ( ) != it; it++)
					File (*it).remove ( );
			}	// if (true == isDir)

			if (0 != ::remove (getFullFileName ( ).c_str ( )))
				throw Exception (UTF8String (strerror (errno), charset));
		}
		catch (const Exception& e)
		{
			UTF8String	message (charset);
			message << "Impossibilité de supprimer le "
			        << (true == isDir ? "répertoire" : "fichier") << " "
			        << getFullFileName ( ) << " :"
			        << '\n' << e.getFullMessage ( );
			throw Exception (message);
		}
		catch (...)
		{
			UTF8String	message (charset);
			message << "Impossibilité de supprimer le "
			        << (true == isDir ? "répertoire" : "fichier") << " "
			        << getFullFileName ( ) << " :"
			        << '\n'
			        << "Erreur non documentée.";
			throw Exception (message);
		}
	}	// if (true == exists ( ))
}	// File::remove


void File::print (IN_STD ostream& stream) const
{
	bool	exist	= exists ( );

	stream << "Nom complet             : " << getFullFileName ( ) << endl
	       << "Nom                     : " << getFileName ( ) << endl
	       << "Répertoire              : " << getPath ( ).getFullFileName ( )
	       << endl
	       << "Existe                  : "
	       << (true == exist ? "oui" : "non") << endl;
	if (true == exist)
	{
	stream << "Accès en lecture        : "
	       << (true == isReadable ( ) ? "oui" : "non") << endl
	       << "Est un fichier régulier : " 
	       << (true == isRegularFile ( ) ? "oui" : "non") << endl
	       << "Est un répertoire       : " 
	       << (true == isDirectory ( ) ? "oui" : "non") << endl
	       << "Est un lien symbolique  : " 
	       << (true == isLink ( ) ? "oui" : "non") << endl
	       << "Est un pipe nommé (FIFO): " 
	       << (true == isFifo ( ) ? "oui" : "non") << endl
	       << "Est un socket           : " 
	       << (true == isSocket ( ) ? "oui" : "non") << endl;
	}	// if (true == exist)
	stream << "Accès en écriture       : "
	       << (true == isWritable ( ) ? "oui" : "non") << endl;
	stream << "Accès en exécution      : "
	       << (true == isExecutable ( ) ? "oui" : "non") << endl;
}	// File::print


string File::createTemporaryName (const string& prefix, bool inTmpDir)
{
	UTF8String				path (charset);
	static	const string	tmpDir	= getTmpDir ( );

	if (true == inTmpDir)
		path << tmpDir << '/';
	path << prefix << "XXXXXX";	// cf. man mktemp

	if (path.length ( ) > MAXPATHLEN)
	{	// Thanks to coverity !
		INTERNAL_ERROR (exc, "tmpdir > MAXPATHLEN", "File::createTemporaryName")
		throw exc;
	}	// if (path.length ( ) > MAXPATHLEN)
	char	tmpName [MAXPATHLEN + 1];
	strcpy (tmpName, path.ascii ( ).c_str ( ));

	errno	= 0;
	char*	tmpPath	= ::mktemp (tmpName);
	if (NULL == tmpPath)
	{
		UTF8String	errorMsg (charset);
		errorMsg << "Impossibilité de créer un nom de fichier temporaire à "
		         << "partir de \"" << path.iso ( ) << "\" :"
		         << '\n' << strerror (errno);
		throw Exception (errorMsg);
	}	// if (NULL == tmpPath)

//	return tmpPath;
	return tmpName;	// Portage RHEL_5__x86_64, version 2.8.1
}	// File::createTemporaryName


mode_t File::getUMask ( )
{
	// Pb : umask modifie le masque, et getumask est specifique GNU ...
	mode_t	currentMask	= ::umask (0);
	::umask (currentMask);
	return currentMask;
}	// File::getUMask


ostream& operator << (ostream& os, const File& file)
{
	file.print (os);
	return os;
}	// operator << (ostream&, const File&)



END_NAMESPACE_UTIL
