#include "TkUtil/AnsiEscapeCodes.h"
#include "TkUtil/ApplicationStats.h"
#include "TkUtil/Exception.h"
#include "TkUtil/File.h"
#include "TkUtil/NumericConversions.h"
#include "TkUtil/UTF8String.h"

#include <TkUtil/Date.h>
#include <TkUtil/File.h>
#include <TkUtil/Threads.h>
#include <TkUtil/UserData.h>

#include <map>

#include <assert.h>
#include <errno.h>
#include <sys/file.h>	// flock
#include <stdio.h>		// fopen, fseek, fscanf, fprintf
#include <string.h>		// strerror
#include <sys/stat.h>	// fchmod
#include <sys/types.h>
#include <unistd.h>		// fork, setsid

USING_STD

BEGIN_NAMESPACE_UTIL


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
	// En vue de ne pas altérer le comportement de l'application tout est effectuée dans un processus fils.
	// Par ailleurs on quitte le processus fils par return et non exit pour passer dans le destructeur des variables automatiques créées (TermAutoStyle, ...).
	errno	= 0;
	const pid_t	pid	= fork ( );
	if ((pid_t)-1 == pid)
	{
		ConsoleOutput::cerr ( ) << "ApplicationStats::logUsage : échec de fork : " << strerror (errno) << co_endl;
		return;
	}	// if ((pid_t)-1 == pid)
	if (0 != pid)
		return;	// Parent

	// On détache complètement le fils du parent => peut importe qui fini en premier, l'autre ira jusqu'au bout :
	const pid_t	sid	= setsid ( );
	if ((pid_t)-1 == sid)
	{
		ConsoleOutput::cerr ( ) << "ApplicationStats::logUsage : échec de setsid : " << strerror (errno) << co_endl;
		exit (0);
	}	// if ((pid_t)-1 == sid)
	
	TermAutoStyle	as (cerr, AnsiEscapeCodes::blueFg);
	if ((true == appName.empty ( )) || (true == logDir.empty ( )))
	{
		ConsoleOutput::cerr ( ) << "ApplicationStats::logUsage : nom d'application ou répertoire des logs non renseigné (" << appName << "/" << logDir << ")." << co_endl;
		exit (0);
	}	// if ((true == appName.empty ( )) || (true == logDir.empty ( )))

	// Le nom du fichier :
	const Date		date;
	const string	user (UserData (true).getName ( ));
	UTF8String	fileName (getFileName (appName, logDir, (unsigned long)date.getMonth ( ), date.getYear ( )), Charset::UTF_8);

	// On ouvre le fichier en lecture/écriture :
	FILE* 		file	= fopen (fileName.utf8 ( ).c_str ( ), "r+");		// Ne créé pas le fichier => on le créé ci-dessous si nécessaire :
	const bool	created	= NULL == file ? true : false;
	file		= NULL == file ? fopen (fileName.utf8 ( ).c_str ( ), "a+") : file;
	if (NULL == file)
	{
		try
		{	// On peut avoir des exceptions de levées : chemin non traversable, ...
			File	dir (logDir);
			if ((false == dir.exists ( )) || (false == dir.isDirectory ( )) || (false == dir.isExecutable ( )) || (false == dir.isWritable ( )))
			{
				ConsoleOutput::cerr ( ) << "Erreur, " << logDir << " n'est pas un répertoire existant avec les droits en écriture pour vous." << co_endl;
				exit (0);
			}	// if ((false == dir.exists ( )) || (false == dir.isDirectory ( )) || ...
			File	logFile (fileName.utf8 ( ));
			if (false == logFile.isWritable ( ))
			{
				ConsoleOutput::cerr ( ) << "Erreur lors de l'ouverture du fichier de logs " << fileName << " : absence de droits en écriture." << co_endl;
				exit (0);
			}	// if (false == logFile.isWritable ( ))
		}
		catch (const Exception& exc)
		{
			ConsoleOutput::cerr ( ) << "Erreur lors de l'ouverture du fichier de logs " << fileName << " : " << exc.getFullMessage ( ) << co_endl;
			exit (0);
		}
		catch (...)
		{
		}

		ConsoleOutput::cerr ( ) << "Erreur lors de l'ouverture du fichier de logs " << fileName << " : erreur non documentée." << co_endl;

        exit (0);
	}	// if (NULL == file)

	// Obtenir le descripteur de fichier :
	int	fd	= fileno (file);
	if (-1 == fd)
	{
        ConsoleOutput::cerr ( ) << "Erreur lors de l'ouverture du fichier de logs " << fileName << co_endl;
        exit (0);
	}	// if (-1 == fd)

	// Appliquer un verrou exclusif sur le fichier de logs :
	errno	= 0;
	if (0 != flock (fd, LOCK_EX))
	{
		ConsoleOutput::cerr ( ) << "Erreur lors du verrouillage du fichier de logs " << fileName << " : " << strerror (errno) << co_endl;
		fclose (file);
		exit (0);
	}	// if (0 != flock (fd, LOCK_EX))
	
	// Conférer aufichier les droits en écriture pour tous le monde si il vient d'être créé :
	if (true == created)
	{
		if (0 != fchmod (fd, S_IRWXU | S_IRWXG | S_IRWXO))
		{
			ConsoleOutput::cerr ( ) << "Erreur lors du confèrement à autrui des droits en écriture sur le fichier de logs " << fileName << " : " << strerror (errno) << co_endl;
			fclose (file);
			exit (0);

		}	// if (0 != fchmod (fd, S_IRWXU | S_IRWXG | S_IRWXO))
	}	// if (true == created)

	// Lecture et actualisation des logs existants :
	map<string, size_t>		logs;
	char					name [256];
	size_t					count	= 0, line	= 1;
	bool					found	= false;
	int						flag	= 0;
	errno	= 0;
	while (2 == (flag = fscanf (file, "%s\t%u", name, &count)))
	{
		line++;
		if (name == user)
		{
			found	= true;
			count++;
		}	// if (name == user)
		logs.insert (pair<string, size_t> (name, count));
		count	= 0;
	}	// while (2 == fscanf (file, "%s\t%u", name, &count))
	if (0 != errno)
	{
		ConsoleOutput::cerr ( ) << "Erreur lors de la lecture du fichier de logs " << fileName << " en ligne " << (unsigned long)line << " : " << strerror (errno) << co_endl;
		fclose (file);
		exit (0);
	}	// if (0 != errno)
	else if ((flag < 2) && (EOF != flag))
	{
		ConsoleOutput::cerr ( ) << "Erreur lors de la lecture du fichier de logs " << fileName << " en ligne " << (unsigned long)line << " : fichier probablement corrompu." << co_endl;
		fclose (file);
		exit (0);
	}	// if (flag < 2)
	if (false == found)
		logs.insert (pair<string, size_t> (user, 1));
		
	// Ecriture des logs actualisés :
	errno	= 0;
	if (0 != fseek (file, 0, SEEK_SET))
	{
		ConsoleOutput::cerr ( ) << "Erreur lors de la réécriture du fichier de logs " << fileName << " : " << strerror (errno) << co_endl;
		fclose (file);
		exit (0);
	}	// if (0 != fseek (file, 0, SEEK_SET))
	
	for (map<string, size_t>::const_iterator itl = logs.begin ( ); logs.end ( ) != itl; itl++)
	{
		if (fprintf (file, "%s\t%u\n", (*itl).first.c_str ( ), (*itl).second) < 0)
		{
			ConsoleOutput::cerr ( ) << "Erreur lors de la réécriture du fichier de logs " << fileName << "."<< co_endl;
			fclose (file);
			exit (0);
		}
	}	// for (map<string, size_t>::const_iterator itl = logs.begin ( ); logs.end ( ) != itl; itl++)
	errno	= 0;
	if (0 != fflush (file))
	{
			ConsoleOutput::cerr ( ) << "Erreur lors de la réécriture du fichier de logs " << fileName << " : " << strerror (errno) << co_endl;
			fclose (file);
			exit (0);
	}	// if (0 != fflush (file))
	
	// Libération du verrou :
	errno	= 0;
	if (0 != flock (fd, LOCK_UN))
	{
		ConsoleOutput::cerr ( ) << "Erreur lors du déverrouillage du fichier de logs " << fileName << " : " << strerror (errno) << co_endl;
		fclose (file);
	}	// if (0 != flock (fd, LOCK_UN))
	
	fclose (file);
	file	= NULL;
	fd		= -1;
	
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
	while (2 == (flag = fscanf (&file, "%s\t%u", name, &count)))
	{
		line++;
		map<string, size_t>::iterator	itl	= logs.find (name);
		if (logs.end ( ) == itl)
			logs.insert (pair<string, size_t> (name, count));
		else
			(*itl).second	+= count;
		count	= 0;
	}	// while (2 == fscanf (file, "%s\t%u", name, &count))
	if ((flag < 2) && (EOF != flag))
	{
		ConsoleOutput::cerr ( ) << "Erreur lors de la lecture du fichier de logs " << fileName << " en ligne " << (unsigned long)line << " : fichier probablement corrompu." << co_endl;
		return -1;
	}	// if (flag < 2)

	return errno;
}	// ApplicationStats::readLogs


END_NAMESPACE_UTIL
