#ifndef THREADS_H
#define THREADS_H

#include <TkUtil/util_config.h>

#include <string>
#include <pthread.h>
#include <assert.h>

#include <vector>


BEGIN_NAMESPACE_UTIL

class Mutex;


/**
 * <P>Cette interface représente un thread générique. Sa spécialisation en thread détaché ou non intervient dans les classes d'implémentation dérivées.</P>
 *
 * <P>Les méthodes de cette classe sont susceptibles de lever des exceptions de type <I>Exception</I> du module <I>util</I>.</P>
 *
 * <P>Cette classe est compatible avec la classe <I>ThreadManager</I>. L'éventuelle instance de la classe <I>ThreadManager</I> est informée des
 * fins de tâches en vue d'en relancer d'autres.</P>
 *
 * @author	Charles PIGNEROL, CEA/DAM/DSSI
 * @see		DetachedThread
 * @see		JoinableThread
 */
class ThreadIfc
{
	public :

	/**
	 * Destructeur. RAS.
	 */
	virtual ~ThreadIfc ( );

	/**
	 * @return		L'identifiant du thread.
	 */
	virtual pthread_t getId ( ) const;

	/**
	 * @return		<I>true</I> si le thread est en cours d'exécution.
	 */
	virtual bool isRunning ( ) const;

	/**
	 * @return		<I>true</I> si l'annulation du thread a été demandée (par appel à <I>cancel</I>).
	 * @see			cancel
	 */
	virtual bool cancelRequested ( ) const;

	/**
	 * Lancement de la tâche. Appelle <I>execute</I> dans un autre thread.
	 */
	virtual void startTask ( ) = 0;

	/**
	 * @return		true si le thread est joignable, au sens POSIX, false dans le cas contraire.
	 */
	virtual bool isJoinable ( ) const = 0;

	/**
	 * Arrête le thread si cela est possible.
	 * @warning		<B>Méthode très dangereuse, pouvant être à l'origine de fuites mémoires, dead locks (mutex non dévérrouillés), plantages, ... A éviter autant que possible.</B>
	 * @see			cancelRequested
	 */
	virtual void cancel ( );

	/**
	 * @return		Le drapeau de concurrence de cette tâche. Deux tâches dont un & sur ce drapeau est non nul ne doivent pas être exécutées concurremment.
	 * @see			setConcurrencyFlag
	 * @see			ThreadManager
	 */
	virtual size_t getConcurrencyFlag ( ) const
	{ return _concurrencyFlag; }

	/**
	 * @param		Le nouveau drapeau de concurrence de cette tâche. S'il est nul cette tâche peut s'exécuter en même temps que toute autre tâche.
	 * @see			getConcurrencyFlag
	 * @see			ThreadManager
	 */
	virtual void setConcurrencyFlag (size_t concurrencyFlag)
	{ _concurrencyFlag	= concurrencyFlag; }

	/**
	 * @return		L'instance de cette classe associée au thread courrant, s'il y en a une.
	 * @see			withId
	 */
	static ThreadIfc* current ( );

	/**
	 * @return		L'instance de cette classe associée à l'identifiant transmis en argument, s'il y en a une.
	 * @see			current
	 */
	static ThreadIfc* withId (pthread_t id);


	protected :

	/**
	 * Constructeur. RAS.
	 * @param		Drapeau de concurrence de la tâche.
	 * @see			setConcurrencyFlag
	 * @see			getConcurrencyFlag
	 * @see			ThreadManager
	 */
	ThreadIfc (size_t concurrencyFlag);

	/**
	 * Effectue la tâche parallèlisée. A implémenter dans les classes dérivées.
	 * @warning		<B>Cette méthode ne doit pas lever d'exception.</B>
	 */
	virtual void execute ( ) = 0;

	/**
	 * Le thread posix associé.
	 */
	pthread_t& getPosixThread ( )
	{ return _thread; }
	const pthread_t& getPosixThread ( ) const
	{ return _thread; }

	/**
	 * <P>Appelle <I>execute</I> pour le thread reçu en argument. Informe l'éventuelle instance de la classe <I>ThreadManager</I> de la fin d'exécution de la tâche.</P>
	 * @see		ThreadManager
	 */
	static void* run_thread (void*);


	private :

	/**
	 * Constructeur de copie et opérateur = : interdits.
	 */
	ThreadIfc (const ThreadIfc&);
	ThreadIfc& operator = (const ThreadIfc&);

	/** Le thread est il en cours ? */
	bool								_running;

	/** Une demande d'annulation a-t-elle été formulée ? */
	bool								_cancelled;

	/** Le drapeau de concurrence de cette tâche. */
	size_t								_concurrencyFlag;
	pthread_t							_thread;

	/** Enregistrement du thread dans la liste de threads en cours. */
	static void registerThread (ThreadIfc* thread);

	/** Désenregistrement du thread de la liste de threads en cours. */
	static void unregisterThread (ThreadIfc* thread);

	/** La liste des threads en cours. */
	static IN_STD vector<ThreadIfc*>	_threads;

	/** Le mutex protégeant <I>_threads</I>. */
	static Mutex						_mutex;
};	// class ThreadIfc


/**
 *
 * <P>Classe de base implémentant un <B>thread détaché</B>. La méthode <B>execute</B> de cette classe est à surcharger afin d'effectuer la tâche à parallèliser.
 * </P>
 *
 @warning		<B>Classe non totalement implémentée, et non testée.</B>
 *
 */
class DetachedThread : public ThreadIfc
{
	public :

	/**
	 * Constructeur. RAS.
	 * @param		Drapeau de concurrence de la tâche.
	 */
	DetachedThread (size_t concurrencyFlag = 0);

	/**
	 * Destructeur. RAS.
	 */
	virtual ~DetachedThread ( );

	/**
	 * Lance la tâche à parallèliser.
	 */
	virtual void startTask ( );

	/**
	 * @return		false.
	 */
	virtual bool isJoinable ( ) const
	{ return false; }


	private :

	/**
	 * Constructeur de copie et opérateur = : interdits.
	 */
	DetachedThread (const DetachedThread&);
	DetachedThread& operator = (const DetachedThread&);
};	// class DetachedThread


/**
 * <P>Classe de base implémentant un thread non détaché. La fin d'exécution de la tâche de ce thread (méthode <B>execute</B>) peut être attendue par
 * invocation de sa méthode <B>join</B>.
 * </P>
 * <P>La méthode <B>execute</B> de cette classe est à surcharger afin d'effectuer la tâche à parallèliser.</P>
 */
class JoinableThread : public ThreadIfc
{
	public :

	/**
	 * Constructeur. RAS.
	 * @param		Drapeau de concurrence de la tâche.
	 */
	JoinableThread (size_t concurrencyFlag = 0);

	/**
	 * Destructeur. RAS.
	 */
	virtual ~JoinableThread ( );

	/**
	 * Lance la tâche à parallèliser.
	 */
	virtual void startTask ( );

	/**
	 * @return		true.
	 */
	virtual bool isJoinable ( ) const
	{ return true; }

	/**
	 * Le thread appelant attend que la méthode <B>execute</B> de celui-ci soit
	 * achevée.
	 */
	virtual void join ( );


	private :

	/**
	 * Constructeur de copie et opérateur = : interdits.
	 */
	JoinableThread (const JoinableThread&);
	JoinableThread& operator = (const JoinableThread&);
};	// class JoinableThread


/**
 * Classe destinée à véhiculer des données utilables par un thread et à fournir en fin de tâche des informations sur l'accomplissement de celle-ci.
 * @see		ReportedJoinableThread
 */
class ThreadData
{
	public :

	/**
	 * Constructeur. RAS.
	 */
	ThreadData ( )
		: _success (false), _completionMessage ( )
	{ }

	/**
	 * Constructeur de copie et opérateur =. RAS.
	 */
	ThreadData (const ThreadData& td)
		: _success (td.hasSucceeded ( )), _completionMessage (td.getCompletionMessage ( ))
	{ }
	ThreadData& operator = (const ThreadData& td)
	{
		if (&td != this)
		{
			_success			= td.hasSucceeded ( );
			_completionMessage	= td.getCompletionMessage ( );
		}
		return *this;
	}	// operator =

	/**
	 * Destructeur : RAS.
	 */
	virtual ~ThreadData ( )
	{ }

	/**
	 * @return	true en cas de succès de la tâche, false dans le cas contraire.
	 * @see		setCompletionStatus
	 */
	virtual bool hasSucceeded ( ) const
	{ return _success; }

	/**
	 * @return	un éventuel message concernant l'accomplissement de la tâche.
	 * @see		setCompletionStatus
	 */
	virtual const IN_STD string& getCompletionMessage ( ) const
	{ return _completionMessage; }

	/**
	 * @param	Status de terminaison de la tâche : true (succès) ou false (échec).
	 * @param	Message sur l'accomplissement de la tâche.
	 * @see		hasSucceeded
	 * @see		getCompletionMessage
	 */
	virtual void setCompletionStatus (bool success, const IN_STD string& msg = "")
	{
		_success			= success;
		_completionMessage	= msg;
	}	// setCompletionStatus


	private :

	/** Le status de terminaison. */
	bool					_success;

	/** Le message associé à la terminaison de la tâche. */
	IN_STD string			_completionMessage;
};	// class ThreadData


/**
 * <P>Classe de thread joignable véhiculant une structure de données utilisable par la méthode <I>execute</I>. Cette structure de données peut notamment
 * véhiculer en retour un code d'erreur, en dérivant pas exemple de la classe <I>ThreadData</I>.</P>
 *
 * <P>Les données associées au thread sont à allouer dynamiquement par le thread appelant, et leur destruction sont également à sa charge après la mort du thread.
 * </P>
 */
template <class T> class ReportedJoinableThread : public JoinableThread
{
	public :

	/**
	 * Constructeur.
	 * @param		Données associées au thread. Ces données sont à la charge du thread appelant.
	 */
	ReportedJoinableThread (T* data)
		: JoinableThread ( ), _data (data)
	{ }

	/**
	 * Destructeur. RAS.
	 */
	virtual ~ReportedJoinableThread ( )
	{ }

	/**
	 * @return		Un pointeur sur les données associées au thread.
	 */
	T* getData ( )
	{ return _data; }
	const T* getData ( ) const
	{ return _data; }


	private :

	/**
	 * Constructeur de copie et opérateur = : interdits.
	 */
	ReportedJoinableThread (const ReportedJoinableThread&)
		: JoinableThread ( )
	{
		assert (0 && "ReportedJoinableThread copy constructor is not allowed.");
	}
	ReportedJoinableThread& operator = (const ReportedJoinableThread&)
	{
		assert (0 && "ReportedJoinableThread operator = is not allowed.");
		return *this;
	}

	/** Les données associées au thread. */
	T*					_data;
};	// class ReportedJoinableThread


END_NAMESPACE_UTIL


#endif	// THREADS_H
