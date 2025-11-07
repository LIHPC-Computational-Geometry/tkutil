#include "TkUtil/AnsiEscapeCodes.h"
#include "TkUtil/ApplicationStats.h"
#include "TkUtil/Exception.h"
#include "TkUtil/File.h"
#include "TkUtil/NumericConversions.h"
#include "TkUtil/Process.h"
#include "TkUtil/UTF8String.h"

#include <TkUtil/Date.h>
#include <TkUtil/File.h>
#include <TkUtil/FileLock.h>
#include <TkUtil/Threads.h>
#include <TkUtil/UserData.h>

#include <map>
#include <sstream>

#include <assert.h>
#include <errno.h>
#include <sys/file.h>	// flock
#include <stdio.h>		// fopen, fseek, fscanf, fprintf
#include <string.h>		// strerror
#include <sys/stat.h>	// fchmod
#include <sys/types.h>
#include <sys/param.h>	// MAXPATHLEN
#include <unistd.h>		// fork, setsid

USING_STD

BEGIN_NAMESPACE_UTIL

static const Charset	charset ("àéèùô");


ApplicationStats::ApplicationStats ( )
{
	assert (0 && "ApplicationStats::ApplicationStats is not allowed.");
}	// ApplicationStats::ApplicationStats


ApplicationStats::ApplicationStats (const ApplicationStats&)
{
	assert (0 && "ApplicationStats::ApplicationStats is not allowed.");
}	// ApplicationStats::ApplicationStats


ApplicationStats& ApplicationStats::operator = (const ApplicationStats&)
{
	assert (0 && "ApplicationStats::assignment operator is not allowed.");
	return *this;
}	// ApplicationStats::operator =


ApplicationStats::~ApplicationStats ( )
{
	assert (0 && "ApplicationStats::~ApplicationStats is not allowed.");
}	// ApplicationStats::~ApplicationStats
	

string ApplicationStats::getFileName (const string& appName, const string& logDir, size_t month, size_t year)
{
	UTF8String	fileName (Charset::UTF_8);
	fileName << logDir << "/" << appName << "_" << NumericConversions::toStr (year, 4) << NumericConversions::toStr (month, 2) << ".logs";

	return fileName.utf8 ( );
}	// ApplicationStats::getFileName

	
void ApplicationStats::logUsage (const string& appName, const string& logDir)
{
	FILE*	file	= 0;
	string	fileName;

	try
	{
		if ((true == appName.empty ( )) || (true == logDir.empty ( )))
		{
			{
				
				ConsoleOutput::cerr ( ) << "ApplicationStats::logUsage : nom d'application ou répertoire des logs non renseigné (" << appName << "/" << logDir << ")." << co_endl;
			}

			return;		// Tolérance aux erreurs
		}	// if ((true == appName.empty ( )) || (true == logDir.empty ( )))

		// Le nom du fichier :
		const Date		date;
		const string	user (UserData (true).getName ( ));
		fileName		= getFileName (appName, logDir, (unsigned long)date.getMonth ( ), date.getYear ( ));

		file	= initLogSession (fileName);
		if (0 == file)
			return;			// Process père

		{	// FileLock scope
			FileLock	logLock (fileName, file);

			// Lecture et actualisation des logs existants :
			map<string, size_t>		logs;
			char					name [256];
			size_t					count	= 0, line	= 1;
			bool					found	= false;
			int						flag	= 0;
			errno	= 0;
			while (2 == (flag = fscanf (file, "%s\t%lu", name, &count)))
			{
				line++;
				if (name == user)
				{
					found	= true;
					count++;
				}	// if (name == user)
				logs.insert (pair<string, size_t> (name, count));
				count	= 0;
			}	// while (2 == fscanf (file, "%s\t%lu", name, &count))
			if (0 != errno)
			{
				UTF8String	error (charset);
				error << "Erreur lors de la lecture du fichier de logs " << fileName << " en ligne " << (unsigned long)line << strerror (errno);
				errno	= 0;
				throw Exception (error);
			}	// if (0 != errno)
			else if ((flag < 2) && (EOF != flag))
			{
				UTF8String	error (charset);
				error << "Erreur lors de la lecture du fichier de logs " << fileName << " en ligne " << (unsigned long)line << " : fichier probablement corrompu.";
				throw Exception (error);
			}	// if (flag < 2)
			if (false == found)
				logs.insert (pair<string, size_t> (user, 1));
		
			// Réécriture des logs actualisés :
			errno	= 0;
			if (0 != fseek (file, 0, SEEK_SET))
			{
				UTF8String	error (charset);
				error << "Erreur lors de la réécriture du fichier de logs " << fileName << " : " << strerror (errno);
				errno	= 0;
				throw Exception (error);
			}	// if (0 != fseek (file, 0, SEEK_SET))
	
			for (map<string, size_t>::const_iterator itl = logs.begin ( ); logs.end ( ) != itl; itl++)
			{
				if (fprintf (file, "%s\t%lu\n", (*itl).first.c_str ( ), (*itl).second) < 0)
				{
					UTF8String	error (charset);
					error << "Erreur lors de la réécriture du fichier de logs " << fileName << ".";
					throw Exception (error);
				}	// if (fprintf (file, "%s\t%lu\n", (*itl).first.c_str ( ), (*itl).second) < 0)
			}	// for (map<string, size_t>::const_iterator itl = logs.begin ( ); logs.end ( ) != itl; itl++)

			errno	= 0;
			if (0 != fflush (file))
			{
				UTF8String	error (charset);
				error << "Erreur lors de la réécriture du fichier de logs " << fileName << " : " << strerror (errno);
				errno	= 0;
				throw Exception (error);
			}	// if (0 != fflush (file))

		}	// FileLock scope
	}
	catch (const Exception& exc)
	{
		TermAutoStyle	as (cerr, AnsiEscapeCodes::blueFg);
		ConsoleOutput::cerr ( ) << exc.getFullMessage ( ) << co_endl;
	}
	catch (...)
	{
		TermAutoStyle	as (cerr, AnsiEscapeCodes::blueFg);
		ConsoleOutput::cerr ( ) << "ApplicationStats::logUsage : erreur non renseignée lors de l'écriture d'informations dans le fichier \"" << fileName << "\"." << co_endl;
	}
	
	if (0 != file)
	{
		fclose (file);
		file	= 0;
	}	// if (0 != file)
	
	exit (0);
}	// ApplicationStats::logUsage


/**
 * Structure portant les informations à enregistrer dans un fichier, à savoir la version de l'application, le système d'exploitation, et le chemin 
 * d'accès complet à l'application.
 */
struct OriginInfos
{
	OriginInfos (const string& v, const string& o, const string& p)
		: version (v), os (o), path (p), key ( )
	{
		ostringstream		s;
		s << version << os << path;
		key	= s.str ( );
	}
	OriginInfos (const OriginInfos& oi)
		: version (oi.version), os (oi.os), path (oi.path), key (oi.key)
	{ }
	OriginInfos& operator = (const OriginInfos& oi)
	{
		if (&oi != this)
		{
			version	= oi.version;
			os		= oi.os;
			path	= oi.path;
			key		= oi.key;
		}
		
		return *this;
	}
	string	version, os, path;
	string	key;
};	// struct OriginInfos


static bool operator == (const OriginInfos& left, const OriginInfos& right)
{
	return (left.version == right.version) && (left.os == right.os) && (left.path == right.path) ? true : false;
}	// operator == (const OriginInfos& left, const OriginInfos& right)


static bool operator != (const OriginInfos& left, const OriginInfos& right)
{
	return !(left == right);
}	// operator != (const OriginInfos& left, const OriginInfos& right)


static bool operator < (const OriginInfos& left, const OriginInfos& right)
{
	return left.key < right.key;
}	// operator < (const OriginInfos& left, const OriginInfos& right)



void ApplicationStats::logUsage (const string& appName, const string& logDir, const string& version, const string& os, const string& path)
{
	FILE*	file	= 0;
	string	fileName;

	try
	{
		if ((true == appName.empty ( )) || (true == logDir.empty ( )))
		{
			{
				
				ConsoleOutput::cerr ( ) << "ApplicationStats::logUsage : nom d'application ou répertoire des logs non renseigné (" << appName << "/" << logDir << ")." << co_endl;
			}

			return;		// Tolérance aux erreurs
		}	// if ((true == appName.empty ( )) || (true == logDir.empty ( )))

		// Le nom du fichier :
		const Date		date;
		const string	user (UserData (true).getName ( ));
		fileName	= getFileName (appName + string ("_usages"), logDir, (unsigned long)date.getMonth ( ), date.getYear ( ));

		file	= initLogSession (fileName);
		if (0 == file)
			return;			// Process père

		{	// FileLock scope
			FileLock	logLock (fileName, file);

			// Lecture et actualisation des logs existants :
			map<OriginInfos, size_t>	logs;
			char						v [255], o [255], p [MAXPATHLEN + 1];
			size_t						count	= 0, line	= 1;
			bool						found	= false;
			int							flag	= 0;
			
			errno	= 0;
			const OriginInfos	addedOi (version, os, path);
			while (4 == (flag = fscanf (file, "%s\t%s\t%s\t%lu", v, o, p, &count)))
			{
				line++;
				const OriginInfos	oi (v, o, p);
				if (addedOi == oi)
				{
					found	= true;
					count++;
				}	// if (addedOi == oi)
				logs.insert (pair<OriginInfos, size_t>(oi, count));
				count	= 0;
			}	// while (4 == (flag = fscanf (file, "%s\t%s\t%s\t%lu", v, o, p, &count)))

			if (0 != errno)
			{
				UTF8String	error (charset);
				error << "Erreur lors de la lecture du fichier de logs " << fileName << " en ligne " << (unsigned long)line << strerror (errno);
				errno	= 0;
				throw Exception (error);
			}	// if (0 != errno)
			else if ((flag < 4) && (EOF != flag))
			{
				UTF8String	error (charset);
				error << "Erreur lors de la lecture du fichier de logs " << fileName << " en ligne " << (unsigned long)line << " : fichier probablement corrompu.";
				throw Exception (error);
			}	// if ((flag < 4) && (EOF != flag))

			if (false == found)
				logs.insert (pair<OriginInfos, size_t>(addedOi, 1));

			// Réécriture des logs actualisés :
			errno	= 0;
			if (0 != fseek (file, 0, SEEK_SET))
			{
				UTF8String	error (charset);
				error << "Erreur lors de la réécriture du fichier de logs " << fileName << " : " << strerror (errno);
				errno	= 0;
				throw Exception (error);
			}	// if (0 != fseek (file, 0, SEEK_SET))

			for (map<OriginInfos, size_t>::const_iterator itl = logs.begin ( ); logs.end ( ) != itl; itl++)
			{
				if (fprintf (file, "%s\t%s\t%s\t%lu\n", (*itl).first.version.c_str ( ), (*itl).first.os.c_str ( ),(*itl).first.path.c_str ( ), (*itl).second) < 0)
				{
					UTF8String	error (charset);
					error << "Erreur lors de la réécriture du fichier de logs " << fileName << ".";
					throw Exception (error);
				}	// if (fprintf (file, "%s\t%s\t%s\t%lu\n", (*itl).first.version.c_str ( ), (*itl).first.os.c_str ( ),(*itl).first.path.c_str ( ), (*itl).second) < 0)
			}	// for (map<OriginInfos, size_t>::const_iterator itl = logs.begin ( ); logs.end ( ) != itl; itl++)

			errno	= 0;
			if (0 != fflush (file))
			{
				UTF8String	error (charset);
				error << "Erreur lors de la réécriture du fichier de logs " << fileName << " : " << strerror (errno);
				errno	= 0;
				throw Exception (error);				
			}	// if (0 != fflush (file))
		}	// FileLock scope
	}
	catch (const Exception& exc)
	{
		TermAutoStyle	as (cerr, AnsiEscapeCodes::blueFg);
		ConsoleOutput::cerr ( ) << exc.getFullMessage ( ) << co_endl;
	}
	catch (...)
	{
		TermAutoStyle	as (cerr, AnsiEscapeCodes::blueFg);
		ConsoleOutput::cerr ( ) << "ApplicationStats::logUsage : erreur non renseignée lors de l'écriture d'informations dans le fichier \"" << fileName << "\"." << co_endl;
	}
	
	if (0 != file)
	{
		fclose (file);
		file	= 0;
	}	// if (0 != file)
	
	exit (0);
}	// ApplicationStats::logUsage


void ApplicationStats::logStats (std::ostream& output, const std::string& appName, const string& from, const string& to, const std::string& logDir)
{
	map<string, size_t> logs;
	const size_t		fromMonth	= NumericConversions::strToULong (UTF8String (from).substring (0, 1));
	const size_t		fromYear	= NumericConversions::strToULong (UTF8String (from).substring (2));
	const size_t		toMonth		= NumericConversions::strToULong (UTF8String (to).substring (0, 1));
	const size_t		toYear		= NumericConversions::strToULong (UTF8String (to).substring (2));

	if (fromYear == toYear)
	{
		for (size_t m = fromMonth; m <= toMonth; m++)
		{
			cout << "Performing " << m << "/" << fromYear << endl;
			UTF8String	fileName (getFileName (appName, logDir, m, fromYear), Charset::UTF_8);

			if (0 != readLogs (fileName, logs))
				ConsoleOutput::cerr ( ) << "Erreur lors de la lecture du fichier de logs " << fileName << " : " << strerror (errno) << co_endl;
		}	// for (size_t m = fromMonth; m <= toMonth; m++)
	}	// if (fromYear == toYear)
	else
	{
		for (size_t m = fromMonth; m <= 12; m++)
		{
			cout << "Performing " << m << "/" << fromYear << endl;
			UTF8String	fileName (getFileName (appName, logDir, m, fromYear), Charset::UTF_8);

			if (0 != readLogs (fileName, logs))
				ConsoleOutput::cerr ( ) << "Erreur lors de la lecture du fichier de logs " << fileName << " : " << strerror (errno) << co_endl;
		}
		for (size_t y = fromYear + 1; y < toYear; y++)
			for (size_t m = 1; m <= 12; m++)
			{
				cout << "Performing " << m << "/" << y << endl;
				UTF8String	fileName (getFileName (appName, logDir, m, y), Charset::UTF_8);

				if (0 != readLogs (fileName, logs))
					ConsoleOutput::cerr ( ) << "Erreur lors de la lecture du fichier de logs " << fileName << " : " << strerror (errno) << co_endl;
			}
		for (size_t m = 1; m <= toMonth; m++)
		{
			cout << "Performing " << m << "/" << toYear << endl;
			UTF8String	fileName (getFileName (appName, logDir, m, toYear), Charset::UTF_8);

			if (0 != readLogs (fileName, logs))
				ConsoleOutput::cerr ( ) << "Erreur lors de la lecture du fichier de logs " << fileName << " : " << strerror (errno) << co_endl;
		}
	}	// else if (fromYear == toYear)
	
	// On imprime les résultats dans le flux :
	for (map<string, size_t>::const_iterator itl =  logs.begin ( ); logs.end ( ) != itl; itl++)
		output << (*itl).first << "\t\t" << (*itl).second << endl;
}	// ApplicationStats::logStats


FILE* ApplicationStats::initLogSession (const string fullFileName)
{
	// En vue de ne pas altérer le comportement de l'application tout est effectuée dans un processus fils => exit (0) en toutes circonstances.
	errno	= 0;
	const pid_t	pid	= fork ( );
	if ((pid_t)-1 == pid)
	{
		TermAutoStyle	as (cerr, AnsiEscapeCodes::blueFg);
		ConsoleOutput::cerr ( ) << "ApplicationStats::initLogSession : échec de fork : " << strerror (errno) << co_endl;
		return 0;
	}	// if ((pid_t)-1 == pid)
	if (0 != pid)
	{
		Process::killAtEnd (pid);
		return 0;	// Parent
	}

	// On détache complètement le fils du parent => peut importe qui fini en premier, l'autre ira jusqu'au bout :
	const pid_t	sid	= setsid ( );
	if ((pid_t)-1 == sid)
	{
		{
			TermAutoStyle	as (cerr, AnsiEscapeCodes::blueFg);
			ConsoleOutput::cerr ( ) << "ApplicationStats::initLogSession : échec de setsid : " << strerror (errno) << co_endl;
		}
		exit (0);
	}	// if ((pid_t)-1 == sid)
	
	// On ouvre le fichier en lecture/écriture :
	FILE* 		file	= fopen (fullFileName.c_str ( ), "r+");		// Ne créé pas le fichier => on le créé ci-dessous si nécessaire :
	const bool	created	= NULL == file ? true : false;
	file		= NULL == file ? fopen (fullFileName.c_str ( ), "a+") : file;
	if (NULL == file)
	{
		try
		{	// On peut avoir des exceptions de levées : chemin non traversable, ...
			File	logFile (fullFileName);
			File	dir		= logFile.getPath ( );
			if ((false == dir.exists ( )) || (false == dir.isDirectory ( )) || (false == dir.isExecutable ( )) || (false == dir.isWritable ( )))
			{
				{
					TermAutoStyle	as (cerr, AnsiEscapeCodes::blueFg);
					ConsoleOutput::cerr ( ) << "Erreur, " << dir.getFullFileName ( ) << " n'est pas un répertoire existant avec les droits en écriture pour vous." << co_endl;
				}
				exit (0);
			}	// if ((false == dir.exists ( )) || (false == dir.isDirectory ( )) || ...
			if (false == logFile.isWritable ( ))
			{
				{
					TermAutoStyle	as (cerr, AnsiEscapeCodes::blueFg);
					ConsoleOutput::cerr ( ) << "Erreur lors de l'ouverture du fichier de logs " << fullFileName << " : absence de droits en écriture." << co_endl;
				}
				exit (0);
			}	// if (false == logFile.isWritable ( ))
		}
		catch (const Exception& exc)
		{
			{
				TermAutoStyle	as (cerr, AnsiEscapeCodes::blueFg);
				ConsoleOutput::cerr ( ) << "Erreur lors de l'ouverture du fichier de logs " << fullFileName << " : " << exc.getFullMessage ( ) << co_endl;
			}
			exit (0);
		}
		catch (...)
		{
		}

		{
			TermAutoStyle	as (cerr, AnsiEscapeCodes::blueFg);
			ConsoleOutput::cerr ( ) << "Erreur lors de l'ouverture du fichier de logs " << fullFileName << " : erreur non documentée." << co_endl;
		}

        exit (0);
	}	// if (NULL == file)

	// Obtenir le descripteur de fichier :
	int	fd	= fileno (file);
	if (-1 == fd)
	{
		{
			TermAutoStyle	as (cerr, AnsiEscapeCodes::blueFg);
			ConsoleOutput::cerr ( ) << "Erreur lors de l'ouverture du fichier de logs " << fullFileName << co_endl;
		}
        exit (0);
	}	// if (-1 == fd)
	
	// Conférer aufichier les droits en écriture pour tous le monde si il vient d'être créé :
	if (true == created)
	{
		if (0 != fchmod (fd, S_IRWXU | S_IRWXG | S_IRWXO))
		{
			{
				TermAutoStyle	as (cerr, AnsiEscapeCodes::blueFg);
				ConsoleOutput::cerr ( ) << "Erreur lors du confèrement à autrui des droits en écriture sur le fichier de logs " << fullFileName << " : " << strerror (errno) << co_endl;
			}
			fclose (file);
			exit (0);

		}	// if (0 != fchmod (fd)
	}	// if (true == created)
	
	return file;
}	// ApplicationStats::initLogSession


int ApplicationStats::readLogs (const string& fileName, map<string, size_t>& logs)
{
	File	logFile (fileName);
	if (false == logFile.isReadable ( ))
		return 0;

	int	retval	= 0;
	errno		= 0;
	FILE*	file	= fopen (fileName.c_str ( ), "r");
	if (NULL != file)
	{
		retval	= readLogs (*file, fileName, logs);
		fclose (file);
	}	// if (NULL != file)

	return retval;
}	// ApplicationStats::readLogs


int ApplicationStats::readLogs (FILE& file, const string& fileName, map<string, size_t>& logs)
{
	errno			= 0;
	size_t	line	= 1, count	= 0;
	int		flag	= 0;
	char	name [256];
	while (2 == (flag = fscanf (&file, "%s\t%lu", name, &count)))
	{
		line++;
		map<string, size_t>::iterator	itl	= logs.find (name);
		if (logs.end ( ) == itl)
			logs.insert (pair<string, size_t> (name, count));
		else
			(*itl).second	+= count;
		count	= 0;
	}	// while (2 == fscanf (file, "%s\t%lu", name, &count))
	if ((flag < 2) && (EOF != flag))
	{
		ConsoleOutput::cerr ( ) << "Erreur lors de la lecture du fichier de logs " << fileName << " en ligne " << (unsigned long)line << " : fichier probablement corrompu." << co_endl;
		return -1;
	}	// if (flag < 2)

	return errno;
}	// ApplicationStats::readLogs


END_NAMESPACE_UTIL
