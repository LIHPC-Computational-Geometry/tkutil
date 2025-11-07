#ifndef APPLICATION_STATS_H
#define APPLICATION_STATS_H

#include <TkUtil/util_config.h>

#include <map>
#include <string>

#include <stdio.h>


BEGIN_NAMESPACE_UTIL


/**
 * Classe de services destinés à enregistrer des logs pour statistiques sur l'utilisation d'applications.
 * Ces services sont à vocation non bloquante, ils ne doivent pas occasionner la moindre gêne à l'application utilisatrice,
 * même en cas de défaillance : ça marche, tant mieux, ça ne marche pas, tant pis !
 * @author	Charles PIGNEROL, CEA/DAM/DSSI
 * @since	6.11.0
 */
class ApplicationStats
{
	public :

	/**
	 * <U><B>Les fichiers simplifié d'utilisation d'application.</B></U><BR>
	 * <U>Nom du fichier :</U> NomApplication_YYYYMM où YYYY représente l'année et MM le mois en cours,<BR>
	 * <U>Format :</U><BR>
	 * utilisateur	nbutilisations
	 */
	//@{	Fichier simplifié d'utilisation d'applications

	/**
 	 * @param	appName	est le nom de l'application
	 * @param	logDir	est le répertoire où sont stockés les fichiers de logs
	 * @param	Mois et année concernés
	 * @return	Un nom de fichier simplifié de logs.
	 */
	static std::string getFileName (const std::string& appName, const std::string& logDir, size_t month, size_t year);

	/**
	 * Ajoute une utilisation de cette application à l'utilisateur courrant. L'opération se fait dans un processus détaché.
	 * Toute erreur rencontrée est affichée dans la console de lancement de l'application.
	 * @param	appName	est le nom de l'application
	 * @param	logDir	est le répertoire où sont stockés les fichiers de logs
	 */
	static void logUsage (const std::string& appName, const std::string& logDir);

	/**
	 * Ajoute une utilisation de cette version de l'application pour un operating system et un chemin d'accès donnés.
	 * L'opération se fait dans un processus détaché. Toute erreur rencontrée est affichée dans la console de lancement de l'application.
	 * @param	appName	est le nom de l'application
	 * @param	logDir	est le répertoire où sont stockés les fichiers de logs
	 * @param	version est la version utilisée de l'application
	 * @param	os est le nom du système d'exploitation (ou tout autre moyen de l'identifier)
	 * @param	path est le chemin d'accès complet de l'application
	 * @since	6.14.0
	 */
	static void logUsage (const std::string& appName, const std::string& logDir, const std::string& version, const std::string& os, const std::string& path);

	/**
	 * Rassemble les utilisations d'une application sur la période demandée et en affiche la synthèse dans le flux transmis en argument.
	 * Toute erreur rencontrée est affichée dans la console de lancement de l'application.
	 * @param	flux où est écrit la synthèse
	 * @param	appName	est le nom de l'application
	 * @param	est la date de début au format MMYYYY
	 * @param	est la date de fin au format MMYYYY
	 * @param	logDir	est le répertoire où sont stockés les fichiers de logs
	 */
	static void logStats (std::ostream& output, const std::string& appName, const std::string& logDir, const std::string& from, const std::string& to);
	//@}	Fichier simplifié d'utilisation d'applications


	private :

	/**
	 * Effectue un fork, ouvre le fichier en lecture/réécriture avec les bons droits, ...
	 * @return	un pointeur sur le fichier ouvert pour y écrire des informations (processus fils) ou 0 (processus père). Appelle exit (0) en cas d'erreur dans le
	 *			processus fils (tolérance aux fautes).
	 * @since	6.14.0
	 */
	static FILE* initLogSession (const std::string fullFileName);

	/**
	 * Charge les logs du fichiers ouvert transmis en argument dans la map transmise en second argument.
	 * @return	0 en cas de succès. En cas d'erreur errno est positionné.
	 */
	static int readLogs (const std::string& fileName, std::map<std::string, size_t>& logs);
	static int readLogs (FILE& file, const std::string& fileName, std::map<std::string, size_t>& logs);
	 
	/**
	 * Constructeurs/Destructeur : interdits.
	 */
	ApplicationStats ( );
	ApplicationStats (const ApplicationStats&);
	ApplicationStats& operator = (const ApplicationStats&);
	~ApplicationStats ( );
};	// class ApplicationStats


END_NAMESPACE_UTIL


#endif	// APPLICATION_STATS_H
