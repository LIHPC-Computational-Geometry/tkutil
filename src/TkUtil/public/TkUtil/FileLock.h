/**
 * \file	FileLock.h
 *
 * \author	Charles PIGNEROL, CEA/DAM/DCLC
 *
 * \date	07/11/2025
 */
#ifndef FILE_LOCK_H
#define FILE_LOCK_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/file.h>
#include <string>

#include <TkUtil/util_config.h>
#include <TkUtil/Exception.h>


//BEGIN_NAMESPACE_UTIL
namespace TkUtil {

  
/**
 * Classe permettant de verrouiller un fichier contre les accès concurrents.
 * 
 * @warning	<B>ATTENTION :</B> il ne semble pas à ce jour exister de dispositif portable.
 * 			Cette classe est utilisée en environnement Linux.
 *
 * @author	Charles PIGNEROL, CEA/DAM/DCLC
 * @since	6.14.0
 */
class FileLock
{
	public :

	/**
	 * Constructeur.
	 */
	FileLock ( )
		: _path ( ), _file (0), _fd (0)
	{ }

	/** Constructeur.
	 * @param	Chemin d'accès complet au fichier (pour les messages d'erreur).
	 * @param	Pointeur sur le fichier.
	 */
	FileLock (const IN_STD string& path, FILE* file)
		: _path (path), _file (file), _fd (0)
	{
		lock ( );
	}

	/**
	 * Destructeur : libère le verrou.
	 */
	~FileLock ( )
	{
		unlock ( );
	}

	/**
	 * Verrouille l'accès au fichier.
	 * @see	unlock
	 */
	void lock ( )
	{
		if (0 == _fd)
		{
			if (0 == _file)
				throw IN_UTIL Exception (IN_UTIL UTF8String ("FileLock::lock : absence de pointeur sur un fichier à verrouiller.", IN_UTIL Charset::UTF_8));

			_fd	= fileno (_file);
			if (-1 == _fd)
			{
				_fd	= 0;
				throw IN_UTIL Exception (IN_UTIL UTF8String ("FileLock::lock : erreur lors de la récupération d'un descripteur du fichier à verrouiller.", IN_UTIL Charset::UTF_8));
			}	// if (-1 == _fd)
		}	// if (0 == _fd)

		errno	= 0;
		if (0 != flock (_fd, LOCK_EX))
		{
			_fd	= 0;
			IN_UTIL UTF8String	error (IN_UTIL Charset::UTF_8);
			error << "Erreur lors du verrouillage du fichier " << _path << " : " << strerror (errno) << ".";
			errno	= 0;
			throw IN_UTIL Exception (error);
		}	// if (0 != flock (_fd, LOCK_EX))
	}	// lock

	/**
	 * Déverrouille l'accès au fichier.
	 * @see	lock
	 */
	void unlock ( )
	{
		if (0 < _fd)
		{
			errno	= 0;
			if (0 != flock (_fd, LOCK_UN))
			{
				_fd	= 0;
				IN_UTIL UTF8String	error (IN_UTIL Charset::UTF_8);
				error << "Erreur lors du déverrouillage du fichier " << _path << " : " << strerror (errno) << ".";
				errno	= 0;
				throw IN_UTIL Exception (error);
			}	// if (0 != flock (_fd, LOCK_UN))
			_fd	= 0;
		}	// if (0 < _fd)
	}	// unlock


	private :
	
	/** Constructeur de copie, opérateur = : interdits. */
	FileLock (const FileLock&)
	{ }
	FileLock& operator = (const FileLock&)
	{ return *this; }

	IN_STD string	_path;
	FILE*			_file;
	int				_fd;
};	// class FileLock


//END_NAMESPACE_UTIL
}


#endif	// FILE_LOCK_H

