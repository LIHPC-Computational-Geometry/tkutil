#ifndef PROCESS_H
#define PROCESS_H

#include <TkUtil/File.h>
#include <TkUtil/Version.h>

#include <string>
#include <unistd.h>
#include <signal.h>
#if !defined(__GNUC__) && !defined(__ia64__)
#include <siginfo.h>
#endif	// !defined(__GNUC__) && !defined(__ia64__)

#include <vector>

#define	PROCESS_ERROR_MESSAGE_SIZE		(1025)


BEGIN_NAMESPACE_UTIL


/** <P>Classe de lancement de processus fils.</P>
 * <P>
 * Il existe deux manières de l'utiliser selon la valeur donnée à l'argument de la méthode <I>execute</I> :<BR>
 * <UL>
 * <LI>En appelant la méthode <B>execute</B> avec <B>true</B> en argument, le processus se détruira alors automatiquement à la mort du processus fils
 * (<B>Depuis la v 4.4.0 l'instance de <I>Process</I> est marquée comme terminée mais n'est plus détruite),
 * <LI>En utilisant la méthode <B>execute</B> avec <B>false</B> en argument, puis la méthode <B>wait</B>. Il convient alors de détruire explicitement  l'instance.
 * </UL><BR>
 * Quelque soit le type d'utilisation cette classe est a <B>instancier dynamiquement</B> (new).
 * </P>
 * <P>
 * <P>Exemples :</P><BR>
 * 1- Le processus est lancé et se détruira automatiquement :<BR>
 * <CODE>
 * try<BR>
 * {<BR>
 *		Process*	printCommand	= new Process ("lp");<BR>
 *		printCommand->getOptions ( ).addOption ("-d");<BR>
 *		printCommand->getOptions ( ).addOption ("printerName");<BR>
 *		printCommand->getOptions ( ).addOption ("-n");<BR>
 *		printCommand->getOptions ( ).addOption ("10");<BR>
 *		printCommand->getOptions ( ).addOption ("file.ps");<BR>
 *		printCommand->execute (true);<BR>
 * }<BR>
 * catch (const Exception& exc)<BR>
 * {<BR>
 *		// ...<BR>
 * }<BR>
 * catch (...)<BR>
 * {<BR>
 *		// ...<BR>
 * }<BR>
 * </CODE>
 * <BR>
 * 2- Le processus est lancé et est attendu : à détruire :<BR>
 * <CODE>
 * try<BR>
 * {<BR>
 *		Process*	process	= new Process ("command");<BR>
 *		process->getOptions ( ).addOption ("-x");<BR>
 *		process->getOptions ( ).addOption ("-n");<BR>
 *		process->getOptions ( ).addOption ("-z");<BR>
 *		process->execute (false);<BR>
 *		int	status	= process->wait ( );<BR>
 *		if (0 != status)<BR>
 *			cout << "Terminaison anormale de command : "<BR>
 *			     << process->getErrorMessage ( )<BR>
 *			     << " Status de terminaison : " << status<BR>
 *			     << endl;<BR>
 *		delete process;<BR>
 * }<BR>
 * catch (const Exception& exc)<BR>
 * {<BR>
 *		// ...<BR>
 * }<BR>
 * catch (...)<BR>
 * {<BR>
 *		// ...<BR>
 * }<BR>
 * </CODE><BR>
 * </P>
 * <P>Cette classe permet également de connaître le répertoire courrant de l'application à l'aide de la méthode statique <I>getCurrentDirectory</I>.
 * </P>
 * @warning		Attention, cette classe positionne une action sur SIGCHLD.
 * @author	Charles PIGNEROL, CEA/DAM/DSSI
 */
class Process
{
	public :

	/**
	 * Structure contenant les options des processus fils.
	 */
	class ProcessOptions
	{
		public :

		/** 
		 * Constructeur et opérateur =. RAS.
		 */
		ProcessOptions ( );
		ProcessOptions (const ProcessOptions& copied);
		ProcessOptions& operator = (const ProcessOptions&);

		/**
		 * Ajout d'une option.
		 * @param		option ajoutée
		 */
		virtual void addOption (const IN_STD string& arg);

		/**
		 * @return		Le nombre d'options.
		 */
		virtual size_t getOptionsNum ( ) const
		{ return _options.size ( ); }

		/**
		 * @return		L'option dont l'index est transmis en argument.
		 */
		virtual const IN_STD string& getOption (size_t index) const
		{ return _options [index]; }

		/**
		 * @return		La liste des options.
		 */
		virtual const IN_STD vector<IN_STD string>& getOptions ( ) const;


		private :

		/** Les options du processus fils. */
		IN_STD vector<IN_STD string>		_options;
	};	// class ProcessOptions

	/** 
	 * Constructeur.
	 * @param		Nom de l'exécutable
	 * @warning		Il est indispensable d'utiliser l'opérateur new. 
	 */
	Process (const IN_STD string& processName);

	/**
	 * Destructeur : appelé automatiquement par l'instance en fin d'exécution 
	 * si true est passé en argument d'<I>execute</I> .
	 * @see			execute
	 */
	virtual ~Process ( );

	/**
	 * Créé une instance à partir de la ligne de commande transmise en argument (exécutable puis arguments).
	 * @return		instance créée.
	 * @todo		possibilité de déclarer des variables d'environnement avant l'exécutable avec la synatxe variable=valeur
	 */ 
	static Process* create (const std::string& cmdLine);	// v 6.2.0
	
	/**
	 * @return		Les options du processus.
	 */
	virtual ProcessOptions& getOptions ( );

	/**
	 * @return			La ligne de commande complète qui sera exécutée. A des
	 *					fins de débogage, méthode non utilisée par <I>execute</I>.
	 */
	virtual IN_STD string getCommandLine ( );

	/**
	 * Exécute le process avec ses options.
	 * @param			true si l'instance doit se détruire automatiquement à la fin du processus fils, sinon false.
	 * @exception		Une exception de type Exception est levée en cas  d'erreur.
	 */
	virtual void execute (bool autoDelete = true);

	/**
	 * Attend la fin du processus fils.
	 * @return		status de retour du processus fils.
	 * @see			getCompletionCode
	 * @see			getErrorMessage
	 */
	virtual int wait ( );

	/**
	 * Tue le processus fils.
	 */
	virtual void kill ( );

	/**
	 * @return		le pid de la tâche associée. 
	 */
	virtual pid_t getPid ( ) const
	{ return _processId; }

	/**
	 * @return		Le nom du processus.
	 */
	virtual const IN_STD string& getName ( ) const
	{ return _processName; }

	/**
	 * @return		L'exécutable du processus, chemin d'accès complet compris. Recherche le chemin dans la variable
	 *				d'environnement PATH si nécessaire.
	 * @exception	Une exception est levée si l'exécutable est inaccessible.
	 */
	virtual IN_STD string getPath ( );

	/**
	 * @return		true si le processus fils est achevé, sinon false.
	 * @see			getCompletionCode
	 * @see			getErrorMessage
	 */
	virtual bool isCompleted ( ) const;

	/**
	 * @return		Status de terminaison du processus fils.
	 * @see			getErrorMessage
	 * @see			isCompleted
	 */
	virtual int getCompletionCode ( ) const
	{ return _completionCode; }

	/**
	 * @return		Le message d'erreur associé à une terminaison anormale.
	 * @see			getCompletionCode
	 * @see			isCompleted
	 */
//	virtual const IN_STD string& getErrorMessage ( ) const	// v 4.4.0
//	{ return _errorMessage; }
	virtual const IN_STD string getErrorMessage ( ) const	// v 4.4.0
	{ return std::string (_errorMessage); }

	/**
	 * A invoquer si un pipe doit être ouvert de la sortie standard du
	 * processus fils vers le processus père.
	 * @param		true si la communication doit être en mode bloquant, false
	 *				dans le cas contraire. Le mode non bloquant est irréversible.
	 * @exception	Lève une exception en cas d'erreur
	 * @see			getpipeDescriptors
	 * @see			getChildLine
	 */
	void enableChildToSonCommunications (bool blocking = true);

	/**
	 * @return		true si un pipe doit être ouvert de la sortie standard du
	 *				processus fils vers le processus père.
	 * @see			enableChildToSonCommunications
	 */
	bool isChildToSonCommunicationsEnabled ( ) const
	{ return _childToSonCommunications; }

	/**
	 * Lit une ligne dans le pipe de communication entre les processus père et fils.
	 * @return		la ligne lue
	 * @exception	Une exception est levée en cas d'impossibilité de lire une ligne
	 */
	virtual IN_STD string getChildLine ( );

	/**
	 * @return		les descripteurs du pipe de communication entre le processus père et le processus fils.
	 * @see			enableChildToSonCommunications
	 */
	int* getPipeDescriptors ( )
	{ return _fifo; }

	/** 
	 * Initialisation.
	 * @param		<I>argc</I> du main.
	 * @param		<I>argv</I> du main.
	 * @param		environnement d'execution (3eme argument du main de  l'application, où main est défini comme suit :
	 *				int main (int argc, char* argv[], char* envp [])
	 * @warning		il est essentiel d'appeler cette fonction avant le lancement de tout process fils. En son absence
	 *				il est possible que l'exécution du process fils  échoue.
	 */
	static void initialize (int argc, char* argv [], char* envp []);

	/** 
	 * Initialisation.
	 * @param		environnement d'execution (3eme argument du main de  l'application, où main est défini comme suit :
	 *				int main (int argc, char* argv[], char* envp [])
	 * @warning		il est essentiel d'appeler cette fonction avant le lancement de tout process fils. En son absence
	 *				il est possible que l'exécution du process fils  échoue.
	 * @deprecated	Utiliser la version utilisant également <I>argc</I> et
	 *				<I>argv</I> du main.
	 */
	static void initialize (char* envp []);

	/**
	 * @return		Le répertoire courrant de l'application, ou, en cas d'erreur, une chaine vide.
	 */
	static IN_STD string getCurrentDirectory ( );

	/**
	 * @return		Le chemin d'accès à l'exécutable du processus courrant.
	 *				Requiert une initialisation préalable par <I>initialize</I>.
	 * @exception	Une exception est levée si l'information n'existe pas.
	 */
	static File getCurrentExecutable ( );

	/**
	 * Pour les messages de cette API et des APIs qui l'utilisent, permet de donner davantage de précisions.
	 * @param		Nom du logiciel courant
	 * @param		Version du logiciel courant
	 * @see			getCurrentSoftware
	 * @see			getCurrentSoftwareVersion
	 */
	static void setCurrentSoftware (
		const IN_STD string& name, const Version& version = Version ("0.0.0"));

	/**
	 * @return		Le nom du logiciel courrant, ou une chaine vide s'il n'a pas été renseigné par setCurrentSoftware.
	 * @see			setCurrentSoftware
	 * @see			getCurrentSoftwareVersion
	 */
	static const IN_STD string& getCurrentSoftware ( );

	/**
	 * @return		La version du logiciel courrant, ou "0.0.0" si elle n'a pas été renseignée par setCurrentSoftware.
	 * @see			setCurrentSoftware
	 * @see			getCurrentSoftware
	 */
	static const Version& getCurrentSoftwareVersion ( );


	protected :

	/**
	 * Affecte le pid du processus fils.
	 */
	virtual void setPid (pid_t pid)
	{ _processId	= pid; }

	/**
	 * Affecte le caractère "terminé" à l'instance.
	 */
	virtual void setCompleted ( );

	/**
	 * @param		Status de terminaison du processus fils.
	 */
	virtual void setCompletionCode (int code)
	{ _completionCode	= code; }

	/**
	 * @param		Le message d'erreur associé à une terminaison anormale.
	 * 				Sa recopie se limite aux PROCESS_ERROR_MESSAGE_SIZE-1
	 * 				premiers caractères. Depuis la version 4.4.0 la gestion
	 * 				du message d'erreur est géré dans un tampon pré-alloué car
	 * 				ce message peut être issu d'un <I>signal émis</I> et recopié
	 * 				dans un handler sur signal où il ne fait pas bon d'utiliser
	 * 				malloc et free.
	 */
//	virtual void setErrorMessage (const IN_STD string& message)	// v 4.4.0
//	{ _errorMessage	= message; }
	virtual void setErrorMessage (const char* message);			// v 4.4.0

	/**
	 * @return	True si la terminaison du processus fils est attendue,
	 *			sinon false.
	 * @see		setWaited
	 * @see		waited
	 */
	virtual bool isWaited ( ) const
	{ return _waited; }

	/**
	 * @param	Nouveau caractère attendu du processus fils.
	 * @see		isWaited
	 * @see		waited
	 */
	virtual void setWaited (bool waited)
	{ _waited	= waited; }

	/**
	 * Lit les l'ensemble des données dans le pipe de communication avec
	 * le processus fils et les stocke dans l'attente d'appels à 
	 * getChildLine.
	 */
	virtual void fillChildBuffer ( );

	/**
	 * Ajoute la chaine reçue en argument au tampon de communication entre le processus fils et celui-ci.
	 * @param		Chaine à ajouter
	 * @param		Si true, ajoute un caractère de fin de ligne après
	 *				la chaine.
	 */
	virtual void appendChildBuffer (
							const IN_STD string& str, bool newLine = true);

	/**
	 * Appelé lorsqu'un processus fils est fini. Marque l'instance comme terminée.
	 *i<B>Depuis la version 4.4.0 ne détruit plus l'instance associée si isWaited retourne false.</B>
	 * @param		nature du signal reçu.
	 * @param		informations sur l'origine du signal
	 * @param		contexte d'appel du signal
	 * @see			isWaited
	 * @see			setWaited
	 * @see			fillChildBuffer
	 */
	static void childDeath (int sigNum, siginfo_t* infos, void*);


	private :

	/** Constructeur de copie et operateur = : interdits. */
	Process (const Process&);
	Process& operator = (const Process&);

	/** Le nom du processus fils. */
	IN_STD string						_processName;

	/** Les options du processus fils. */
	ProcessOptions						_options;

	/** Le pid du processus fils. */
	pid_t								_processId;

	/** Le caractère "terminé" de l'instance. */
	bool								_completed;

	/** Vaut true si un pipe de communication doit être ouvert de la sortie
	 * standard du processus fils vers le processus père. */
	bool								_childToSonCommunications;

	/** Vaut true si les communications du processus fils vers le processus
	 * père sont bloquantes, false dans le cas contraire. Vaut true par
	 * défaut.
	 */
	bool								_blockingCommunications;

	/** Le pipe utilisé pour les communications entre le processus père et le processus fils. */
	int									_fifo [2];

	/**
	 * Informations lues à l'occasion d'un appel à getChildLine mais non encore traitées.
	 * @see		getChildLine
	 */
	IN_STD string						_childBuffer;

	/**
	 * Attend-t-on la fin du processus fils ?
	 */
	bool								_waited;

	/**
	 * Le status de terminaison du processus fils.
	 */
	int									_completionCode;

	/** Le message d'erreur associé à une terminaison anormale. */
//	IN_STD string						_errorMessage;			// v 4.4.0
	char								_errorMessage [PROCESS_ERROR_MESSAGE_SIZE];// v 4.4.0

	/** La table comprenant les ids des tâches et les pointeurs sur les  instances associées de cette classe.
	 */
	static IN_STD vector<Process*>		_tasks;

	/** Le nombre d'arguments de lancement de cette application. */
	static int							_argc;

	/** Les arguments de lancement de cette application. */
	static char**						_argv;

	/**
	 * Les paramètres d'environnement de l'application. 
	 */
	static char**						_envp;

	 /** Le nom du logiciel courrant. */
	static IN_STD string				_currentSoftware;

	 /** La version du logiciel courrant. */
	static Version						_currentSoftwareVersion;
};	// class Process


END_NAMESPACE_UTIL


#endif	// PROCESS_H
