#include "TkUtil/AnsiEscapeCodes.h"
#include "TkUtil/ApplicationStats.h"
#include "TkUtil/Exception.h"
#include "TkUtil/File.h"
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
		return;
	}	// if ((pid_t)-1 == sid)
	
	TermAutoStyle	as (cerr, AnsiEscapeCodes::blueFg);
	if ((true == appName.empty ( )) || (true == logDir.empty ( )))
	{
		ConsoleOutput::cerr ( ) << "ApplicationStats::logUsage : nom d'application ou répertoire des logs non renseigné (" << appName << "/" << logDir << ")." << co_endl;
		return;
	}	// if ((true == appName.empty ( )) || (true == logDir.empty ( )))

	// Le nom du fichier :
	const Date		date;
	const string	user (UserData (true).getName ( ));
	UTF8String	fileName;
	fileName << logDir << "/" << appName << "_" << IN_UTIL setw (4) << date.getYear ( ) << setw (2) << (unsigned long)date.getMonth ( ) << ".logs";

	// On ouvre le fichier en lecture/écriture :
	FILE* file	= fopen (fileName.utf8 ( ).c_str ( ), "r+");		// Ne créé pas le fichier => on le créé ci-dessous si nécessaire :
	file		= NULL == file ? fopen (fileName.utf8 ( ).c_str ( ), "a+") : file;
	if (NULL == file)
	{
		try
		{	// On peut avoir des exceptions de levées : chemin non traversable, ...
			File	dir (logDir);
			if ((false == dir.exists ( )) || (false == dir.isDirectory ( )) || (false == dir.isExecutable ( )) || (false == dir.isWritable ( )))
			{
				ConsoleOutput::cerr ( ) << "Erreur, " << logDir << " n'est pas un répertoire existant avec les droits en écriture pour vous." << co_endl;
				return;
			}	// if ((false == dir.exists ( )) || (false == dir.isDirectory ( )) || ...
			File	logFile (fileName.utf8 ( ));
			if (false == logFile.isWritable ( ))
			{
				ConsoleOutput::cerr ( ) << "Erreur lors de l'ouverture du fichier de logs " << fileName << " : absence de droits en écriture." << co_endl;
				return;
			}	// if (false == logFile.isWritable ( ))
		}
		catch (const Exception& exc)
		{
			ConsoleOutput::cerr ( ) << "Erreur lors de l'ouverture du fichier de logs " << fileName << " : " << exc.getFullMessage ( ) << co_endl;
			return;
		}
		catch (...)
		{
		}

		ConsoleOutput::cerr ( ) << "Erreur lors de l'ouverture du fichier de logs " << fileName << " : erreur non documentée." << co_endl;

        return;
	}	// if (NULL == file)

	// Obtenir le descripteur de fichier :
	int	fd	= fileno (file);
	if (-1 == fd)
	{
        ConsoleOutput::cerr ( ) << "Erreur lors de l'ouverture du fichier de logs " << fileName << co_endl;
        return;
	}	// if (-1 == fd)

	// Appliquer un verrou exclusif sur le fichier de logs :
	errno	= 0;
	if (0 != flock (fd, LOCK_EX))
	{
		ConsoleOutput::cerr ( ) << "Erreur lors du verrouillage du fichier de logs " << fileName << " : " << strerror (errno) << co_endl;
		fclose (file);
		return;
	}	// if (0 != flock (fd, LOCK_EX))

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
		return;
	}	// if (0 != errno)
	else if ((flag < 2) && (EOF != flag))
	{
		ConsoleOutput::cerr ( ) << "Erreur lors de la lecture du fichier de logs " << fileName << " en ligne " << (unsigned long)line << " : fichier probablement corrompu." << co_endl;
		fclose (file);
		return;
	}	// if (flag < 2)
	if (false == found)
		logs.insert (pair<string, size_t> (user, 1));
		
	// Ecriture des logs actualisés :
	errno	= 0;
	if (0 != fseek (file, 0, SEEK_SET))
	{
		ConsoleOutput::cerr ( ) << "Erreur lors de la réécriture du fichier de logs " << fileName << " : " << strerror (errno) << co_endl;
		fclose (file);
		return;
	}	// if (0 != fseek (file, 0, SEEK_SET))
	
	for (map<string, size_t>::const_iterator itl = logs.begin ( ); logs.end ( ) != itl; itl++)
	{
		if (fprintf (file, "%s\t%u\n", (*itl).first.c_str ( ), (*itl).second) < 0)
		{
			ConsoleOutput::cerr ( ) << "Erreur lors de la réécriture du fichier de logs " << fileName << "."<< co_endl;
			fclose (file);
			return;
		}
	}	// for (map<string, size_t>::const_iterator itl = logs.begin ( ); logs.end ( ) != itl; itl++)
	errno	= 0;
	if (0 != fflush (file))
	{
			ConsoleOutput::cerr ( ) << "Erreur lors de la réécriture du fichier de logs " << fileName << " : " << strerror (errno) << co_endl;
			fclose (file);
			return;
	}	// if (0 != fflush (file))
	
	// Libération du verrou :
	errno	= 0;
	if (0 != flock (fd, LOCK_UN))
	{
		ConsoleOutput::cerr ( ) << "Erreur lors du déverrouillage du fichier de logs " << fileName << " : " << strerror (errno) << co_endl;
		fclose (file);
	}	// if (0 != flock (fd, LOCK_UN))	
}	// ApplicationStats::logUsage


END_NAMESPACE_UTIL
