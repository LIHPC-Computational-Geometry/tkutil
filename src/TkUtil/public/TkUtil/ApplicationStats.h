#ifndef APPLICATION_STATS_H
#define APPLICATION_STATS_H

#include <TkUtil/util_config.h>

#include <string>


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
	 * Ajoute une utilisation de cette application à l'utilisateur courrant. L'opération se fait dans un processus détaché.
	 * Toute erreur rencontrée est affichée dans la console de lancement de l'application.
	 * @param	appName	est le nom de l'application
	 * @param	logDir	est le répertoire où sont stockés les fichiers de logs
	 */
	static void logUsage (const std::string& appName, const std::string& logDir);
	
	//@}	Fichier simplifié d'utilisation d'applications


	private :
	
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
