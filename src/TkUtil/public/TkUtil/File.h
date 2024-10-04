#ifndef FILE_H
#define FILE_H

#include <TkUtil/util_config.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <ostream>
#include <vector>


BEGIN_NAMESPACE_UTIL


/**
 * Cette classe permet d'accéder à des informations propres à un fichier.
 * @todo	Plein de choses !
 * @author	Charles PIGNEROL, CEA/DAM/DSSI
 */
class File
{
	public :

	/**
	 * Constructeur par défaut. 
	 * @param		Nom complet du fichier
	 */
	File (const IN_STD string& fullFileName);

	/**
	 * Constructeurs de copie et opérateur = : RAS.
	 */
	File (const File&);
	File& operator = (const File&);

	/**
	 * Destructeur : RAS.
	 */
	virtual ~File ( );

	/**
	 * @return		Le nom complet du fichier (répertoire compris).
	 */
	virtual const IN_STD string& getFullFileName ( ) const
	{ return _fullName; }

	/**
	 * @param		Le nom complet du fichier (répertoire compris).
	 * @warning		<B>Ne déplace pas physiquement le fichier, utiliser la fonction <I>rename</I> prévue à cet effet.</B>
	 * @see			rename
	 */
	virtual void setFullFileName (const IN_STD string& fullFileName);

	/**
	 * @return		Le nom du fichier, sans son chemin d'accès.
	 */
	virtual IN_STD string getFileName ( ) const;

	/**
	 * @return		Le répertoire contenant le fichier ou ".".
	 * @see			getBaseName
	 */
	virtual File getPath ( ) const;

	/**
	 * @return		Le nom du fichier sans son extension (ex : "toto" pour "toto.txt").
	 * @see			getExtension
	 * @since		6.6.0
	 */
	const IN_STD string getBaseName ( ) const;
	
	/**
	 * @return		L'extension du fichier (ex : "txt" pour "toto.txt").
	 * @see			getBaseName
	 */
	const IN_STD string getExtension ( ) const;

	/**
	 * @return		true si le fichier existe, sinon false.
	 * @exception	Fonction susceptible de lever une exception.
	 */
	virtual bool exists ( ) const;

	/**
	 * @return		true si le fichier est un fichier régulier, sinon false.
	 */
	virtual bool isRegularFile ( ) const;

	/**
	 * @return		true si le fichier est un répertoire, sinon false.
	 */
	virtual bool isDirectory ( ) const;

	/**
	 * @return		true si le fichier est un lien symbolique, sinon false.
	 */
	virtual bool isLink ( ) const;

	/**
	 * @return		true si le fichier est une FIFO (pipe nommé), sinon false.
	 */
	virtual bool isFifo ( ) const;

	/**
	 * @return		true si le fichier est un socket, sinon false.
	 */
	virtual bool isSocket ( ) const;

	/**
	 * @return		true si le fichier est accessible en lecture, sinon false.
	 */
	virtual bool isReadable ( ) const;

	/**
	 * @return		true si le fichier est accessible en écriture, sinon false. Si le fichier n'existe pas les droits en
	 *				écriture sont testés sur son répertoire.
	 */
	virtual bool isWritable ( ) const;

	/**
	 * @return		true si le fichier est accessible en exécution, sinon false.
	 */
	virtual bool isExecutable ( ) const;
	
	/**
	 * @return		La liste des sous-répertoires (exception faite de "." et "..") et fichiers.
	 * @exception	Une exception est levée en cas d'erreur, notamment si l'instance n'est pas un répertoire.
	 * @since		6.10.0
	 */
	virtual void getChildren (std::vector<File>& directories, std::vector<File>& files) const;

	/**
	 * @return		Les droits d'accès sur le fichier (champ <I>st_mode</I> de la structure <I>stat</I>).
	 *				Combinaison par opérateur | des flags S_IR*, S_IW*, S_IX*, * pouvant prendre pour valeurs USR, GRP, OTH (cf. man chmod).
	 * @exception	Une exception est levée en cas d'erreur.
	 * @see			setAccessRights
	 * @since		5.11.0
	 */
	virtual mode_t getAccessRights ( ) const;
	 
	/**
	 * @param		Nouveaux droits d'accès sur le fichier (champ <I>st_mode</I> de la structure <I>stat</I>).
	 * @exception	Une exception est levée en cas d'erreur.
	 * @see			getAccessRights
	 * @since		5.11.0
	 */
	virtual void setAccessRights (mode_t rigths);
	 
	/**
	 * Créé effectivement le fichier si celui-ci n'existe pas. Créé l'arborescence parente si nécessaire.
	 * @param		true si c'est un fichier, false si ce doit être un répertoire.
	 * @exception	Une exception est levée en cas d'échec de la création du fichier.
	 */
	virtual void create (bool asFile);

	/**
	 * Vide le fichier de son contenu.
	 * @exception	Une exception est levée si le fichier n'existe pas, si c'est un répertoire ou en cas d'erreur.
	 */
	virtual void reset ( );

	/**
	 * Déplace physiquement le fichier.
	 * @param		Nouveau path du fichier.
	 * @param		Si <I>true</I> détruit l'éventuel fichier ayant le path demandé.
	 * 				Si <I>false</I> et qu'un fichier existe déjà alors lève une exception.
	 * 	@exception	Une exception est levée en cas d'impossibilité de déplacer le fichier.
	 * 	@see		setFullFileName
	 */
	virtual void rename (const std::string& newpath, bool erase);	// v 5.4.0

	/**
	 * Détruit le fichier s'il existe.
	 * @warning		<B>Si le fichier est un répertoire le contenu de celui-ci est également détruit.</B>
	 * @exception	Une exception est levée en cas d'échec de la destruction du fichier.
	 */
	virtual void remove ( );

	/**
	 * Affiche les informations sur le fichier dans le flux reçu en argument.
	 */
	virtual void print (IN_STD ostream& stream) const;

	/**
	 * @return		Le masque de permissions utilisé lors de la création de fichiers.
	 */
	static mode_t getUMask ( );

	/**
	 * Créé un nom unique de fichier temporaire.
	 * @param		Préfix du nom de fichier temporaire.
	 * @param		true si le fichier doit être créé dans le répertoire des fichiers temporaires, false si il doit être créé en relatif par rapport à prefix.
	 * @exception	Une exception est levée en cas d'échec.
	 */
	static IN_STD string createTemporaryName (const IN_STD string& prefix, bool inTmpDir = true);


	private :


	/** Le nom complet du fichier (répertoire compris). */
	IN_STD string						_fullName;
};	// class File


IN_STD ostream& operator << (IN_STD ostream& os, const File& file);


END_NAMESPACE_UTIL


#endif	// FILE_H
