#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <TkUtil/util_config.h>
#include <TkUtil/UTF8String.h>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <string>
#include <vector>


namespace TkUtil
{


/**
 * <P>======================================================================</P>
 * <P><B>ATTENTION :</B> Classe en cours de développement et de mise au point.</P>
 * <P>NE FONCTIONNE PAS avec icpc v 12. A priori OK avec icpc v 15 et 17, gcc 4.4 et 6.2.
 * </P>
 *
 * <P>REQUIERT AU MOINS :<BR>
 * - -std=c++0x
 * - -D_GLIBCXX_USE_SCHED_YIELD (accès à <I>this_thread::yield ( )</I>)
 * </P>
 * <P>======================================================================</P>
 *
 * <P>Cette classe permet de gérer un ensemble de <B>threads</B> exécutants des
 * taches. Ils sont de ce fait appelés <I>travailleurs</I>. Elle lance
 * simultanément autant de threads que nécessaire et possible, chaque
 * travailleur prenant en charge une tache à exécuter. Une fois sa tache
 * exécutée le travailleur redevient disponible et reprend une tache s'il y
 * en a de disponible et pouvant s'exécuter en concurrence de celles en cours.
 * En l'absence de tache le travailleur se met en sommeil. Il est réveillé par 
 * gestionnaire lorsque de nouvelles taches lui sont confiées.
 * </P>
 * <P>La fonction <I>barrier</I> de la classe <I>ThreadPool</I> permet de se
 * <B>synchroniser</B> avec la fin d'exécution des taches confiées.
 * </P>
 * <P>Contrairement à la classe <I>ThreadManager</I> les threads sont en
 * quantité restreinte et réutilisés, on fourni au gestionnaire des taches et
 * non des threads.
 * </P>
 *
 * <P>Cette classe permet de gérer des problèmes de compatibilité entre taches.
 * </P>
 *
 * @author	Charles PIGNEROL, CEA/DAM/DSSI
 * @see		ThreadManager
 */
class ThreadPool
{
	class	WorkerThread;
	friend class WorkerThread;

	public :

	/**
	 * Les taches à exécuter. Par défaut elles ne sont pas détruites en fin d'exécution et peuvent entrer en concurrence avec n'importe quelle autre tache.
	 */
	class TaskIfc
	{
		friend class ThreadPool;

		public :

		/**
		 * Le status de la tache.
		 */
		enum STATUS { WAITING, RUNNING, COMPLETED, CANCELED, IN_ERROR };

		/**
		 * Constructeurs/Opérateur = : RAS.
		 */
		TaskIfc ( );
		TaskIfc (const TaskIfc&);
		TaskIfc& operator = (const TaskIfc&);
		virtual ~TaskIfc ( );

		/**
		 * @return	Le status de la tache.
		 */
		STATUS getStatus ( ) const
		{ return _status; }

		/**
		 * @return	L'éventuel message associé à la tache.
		 */
		TkUtil::UTF8String getMessage ( ) const
		{ return _message; }

		/**
 		 * Le gestionnaire doit il détruire la tache une fois exécutée ?
 		 */
		virtual bool toDeleteAtCompletion ( ) const
		{ return _toDelete; }
		virtual void toDeleteAtCompletion (bool del)
		{ _toDelete	= del; }

		/**
		 * @return		Le drapeau de concurrence de cette tache. Deux taches dont un & sur ce drapeau est non nul ne doivent pas être
		 *				exécutées concurremment.
		 * @see			setConcurrencyFlag
		 */
		virtual size_t getConcurrencyFlag ( ) const
		{ return _concurrencyFlag; }

		/**
		 * @param		Le nouveau drapeau de concurrence de cette tache. S'il est nul cette tache peut s'exécuter en même temps que 
		 *				toute autre tache.
		 * @see			getConcurrencyFlag
		 */
		virtual void setConcurrencyFlag (size_t concurrencyFlag)
		{ _concurrencyFlag	= concurrencyFlag; }

		/**
		 * Méthode à surcharger et contenant le code à exécuter de la tache. Ne fait rien par défaut.
		 */
		virtual void execute ( );


		protected :

		/**
		 * Méthode appelée par le gestionnaire (ou la tache) pour modifier le status et son éventuel message associé.
		 */
		virtual void setStatus (STATUS status);
		virtual void setStatus (STATUS status, const TkUtil::UTF8String& msg);


		private :

		/** Le status de la tache. */
		STATUS				_status;

		/** Un éventuel message d'erreur. */
		TkUtil::UTF8String	_message;

		/** Faut il détruire la tache en fin d'exécution. */
		bool				_toDelete;

		/** Le drapeau de concurrence. */
		size_t				_concurrencyFlag;
	};	// class TaskIfc

	/**
	 * Initialise cette classe.
	 * @param		Nombre de taches à exécuter au maximum en concurrence. Si 0, prend le nombre de processeurs de la machine.
	 * @warning		Fonction à appeler avant toute utilisation de cette classe.
	 */
	static void initialize (size_t tasksNum = 0);

	/**
	 * Arrête le service de travailleurs et détruit le gestionnaire.
	 * @warning		Ne plus utiliser cette classe après invocation de cette fonction.
	 */
	static void finalize ( );
	
	/**
	 * @return		une référence sur l'unique instance de cette classe.
	 */
	static ThreadPool& instance ( );


	/** Le délai de répis en l'absence de tâche à exécuter (en nanosecondes). */
	static size_t yieldDelay ( );				// v 6.7.0
	static void setYieldDelay (size_t delay);	// v 6.7.0

	/**
	 * <P>Ajoute la tache transmise en argument à la liste des taches à exécuter.
	 * </P>
	 * @param		tache à effectuer
	 * @param		<I>true</I> si le thread appelant attend la fin des taches en cours via un appel à <I>barrier</I> (synchronisation),
	 * 				et sera à débloquer.
	 * @see			taskCompleted
	 */
	void addTask (TaskIfc& task, bool joinable = false);

	/**
	 * <P>Ajoute les taches transmises en argument à la liste des taches à
	 * exécuter.
	 * </P>
	 * @param		taches à effectuer
	 * @param		<I>true</I> si le thread appelant attend la fin des taches en cours via un appel à <I>barrier</I> (synchronisation),
	 * 				et sera à débloquer.
	 * @see			taskCompleted
	 */
	void addTasks (const std::vector<TaskIfc*>& task, bool joinable = false);

	/**
	 * En retour des statistiques sur l'état d'avancement. C'est à l'appelant de savoir à quoi comparer ...
	 * @param		Le nombre de taches en cours de traitement
	 * @param		Le nombre de taches en attente
	 */
	void stats (size_t& running, size_t& queued) const;

	/**
	 * Attend que toutes les taches joignables recensées soient accomplies.
	 */
	void barrier ( );


	protected :

	/**
	 * Méthode appelée par <I>initialize</I>. Instancie les travailleurs.
	 */
	void init ( );

	/**
	 * Boucle infinie de recensement/exécution des taches.
	 * @see		stop
	 */
	void execute ( );

	/**
	 * Demande d'arrêt du service de travailleurs.
	 * @see		execute
	 * @see		stopWorkers
	 */
	void stop ( );

	/**
	 * Arrêt du service de travailleurs.
	 * @see		execute
	 * @see		stop
	 */
	void stopWorkers ( );

	/**
	 * @return		Une tache a exécuter ou 0 s'il n'y en a pas (absence de tache en file d'attente pouvant s'exécuter en concurrence
	 * 				avec celles en cours).
	 * @see			taskCompleted
	 */
	TaskIfc* getTask ( );

	/**
	 * Appelé par les threads travailleurs lorsque leurs taches sont achevées.
	 * @see			getTask
	 * @see			deleteDeadTasks
	 */
	void taskCompleted (TaskIfc&);

	/**
	 * Détruit les taches accomplies pour lesquelles <I>toDeleteAtCompletion</I>
	 * retournait <I>true</I>.
	 */
	 void deleteDeadTasks ( );

	/**
	 * Détruit les travailleurs.
	 */
	 void deleteWorkers ( );

	/**
	 * Accepte de rejoindre le thread maître si aucune tache n'est en cours ou file d'attente.
	 */
	void checkBarrier ( );

	/**
	 * @return		true si une tache dont le drapeau de concurrence transmis en argument peut être lancée, false dans le cas contraire.
	 * @return		getNextTask
	 */
	bool validateConcurrency (size_t flag) const;

	/**
	 * Attente de la fin d'exécution du thread associé à l'instance.
	 */
	void join ( );


	private :

	/**
	 * Constructeur. RAS.
	 * @param		Nombre de taches à exécuter au maximum en concurrence.
	 */
	ThreadPool (size_t tasksNum);

	/**
	 * Constructeur de copie et opérateur = : interdits.
	 */
	ThreadPool (const ThreadPool&);
	ThreadPool& operator = (const ThreadPool&);

	/**
	 * Destructeur. RAS.
	 */
	~ThreadPool ( );


	/** L'éventuelle unique instance de cette classe. */
	static ThreadPool*					_instance;
	static bool							_completed;

	/** Le thread dans lequel fonctionne l'instance. */
	std::thread*						_thread;

	 /** Nombre de taches à exécuter au maximum en concurrence. */
	size_t								_tasksNum;

	/** Les travailleurs sont ils arrêtés ? False à l'initialisation. */
	mutable std::mutex					_haltMutex;
	bool								_halted;

	/** Un point de rendez-vous sera il demandé pour les taches en cours ? False par défaut. */
	mutable std::mutex					_barrierMutex;
	bool								_barrier;

	/** La liste des taches à lancer. */
	IN_STD deque<TaskIfc*>				_queuedTasks;

	/** La liste des taches en cours d'exécution. */
	IN_STD vector<TaskIfc*>				_runningTasks;

	/** La liste des taches à détruire. */
	IN_STD vector<TaskIfc*>				_deadTasks;

	/** Les travailleurs utilisés. */
	IN_STD vector<WorkerThread*>		_workerThreads;

	/** Le délai de répis en l'absence de tâche à exécuter (en nanosecondes). */
	static size_t						_yieldDelay;		// v 6.7.0

	/** Le mutex de protection des taches. */
	mutable std::mutex					_tasksMutex;
	std::condition_variable				_tasksCond;

	/** L'arrivée d'une tache => reveiller au moins un travailleur si nécessaire. */
	std::mutex							_wakeUpCondMutex;
	std::condition_variable				_wakeUpCond;

	/** Les rendez-vous. */
	std::mutex							_barrierCondMutex;
	std::condition_variable				_barrierCond;

	/**
	 * Les travailleurs accomplissant les taches.
	 */
	class WorkerThread
	{
		public :

		/**
		 * Constructeur. RAS.
		 */
		WorkerThread ( );

		/**
		 * Destructeur. RAS.
		 */
		~WorkerThread ( );

		/**
		 * La boucle d'attente/accomplissement de taches. Ces taches sont récupérées auprès du gestionnaire de travailleurs.
		 */
		void execute ( );

		/**
		 * Met en route le travailleur (<I>execute</I>).
		 */
		void start ( );

		/**
		 * Demande l'arrêt (dès que possible) du travailleur.
		 */
		void stop ( );

		/**
		 * @return		<I>true</I> si l'instance a fini de travailler.
		 */
		bool completed ( ) const;

		/**
 		 * Synchronisation avec l'arrêt du travailleur.
 		 */
		void join ( );


		private :

		/**
		 * Constructeur de copie, opérateur = : interdits.
		 */
		WorkerThread (const WorkerThread&);
		WorkerThread& operator = (const WorkerThread&);

		/** Le thread dédié au travailleur. */
		std::thread*			_thread;

		/** La tache courrante. */
		TaskIfc*				_task;

		/** L'arrêt (dès que possible) du travailleur est il demandé ? */
		mutable std::mutex		_haltMutex;
		bool					_halted;

		/** L'instance a t'elle fini de travailler ? */
		mutable std::mutex		_completedMutex;	// not mandatory
		bool					_completed;
	};	// class WorkerThread
};	// class ThreadPool


}	// namespace TkUtil


#endif	// THREAD_POOL_H
