#ifndef MACHINE_DATA_H
#define MACHINE_DATA_H

#include <TkUtil/util_config.h>
#include <TkUtil/Version.h>
#include <TkUtil/NetworkData.h>



BEGIN_NAMESPACE_UTIL


/**
 * Classe décrivant un système d'exploitation.
 */
class OperatingSystem
{
	public :

	/**
	 * Constructeur par défaut. S'initialise avec les valeurs de la machine sur lequel s'exécute le processus.
	 */
	OperatingSystem ( );

	/**
	 * Constructeur.
	 * @param		Nom du système d'exploitation.
	 * @param		Version du système d'exploitation.
	 * @param		Diverses informations sur le système d'exploitation.
	 * @param		Diverses informations concernant les aspects numériques.
	 */
	OperatingSystem (const IN_STD string& name, const Version& version, const IN_STD string& infos = "", const IN_STD string& numericInfos = "");

	/**
	 * Constructeur de copie, opérateur = : RAS.
	 */
	OperatingSystem (const OperatingSystem& os);
	OperatingSystem& operator = (const OperatingSystem& os)
	{
		if (&os != this)
		{
			_name			= os._name;
			_version		= os._version;
			_distribution	= os._distribution;
			_infos			= os._infos;
			_numericInfos	= os._numericInfos;
		}
		return *this;
	}	// OperatingSystem::operator =

	/**
	 * Destructeur : RAS.
	 */
	virtual ~OperatingSystem ( )
	{ }

	/**
	 * @return		Le nom du système d'exploitation.
	 * @see			setName
	 */
	virtual const IN_STD string& getName ( ) const
	{ return _name; }

	/**
	 * @param		Le nom du système d'exploitation.
	 * @see			getName
	 */
	virtual void setName (const IN_STD string& name)
	{ _name	= name; }

	/**
	 * @return		La version du système d'exploitation.
	 * @see			setVersion
	 */
	virtual const Version& getVersion ( ) const
	{ return _version; }

	/**
	 * @return		La version du système d'exploitation.
	 * @see			getVersion
	 */
	virtual void setVersion (const Version& version)
	{ _version	= version; }
	
	/**
	 * @return		La distibution du système d'exploitation (RedHat, Ubuntu, ...).
	 * @warning		<B>Elle est obtenue à la compilation de la bibliothèque.</B>
	 * @see			getName
	 * @since		6.14.0
	 */
	virtual const IN_STD string& getDistribution ( ) const
	{ return _distribution; }

	/**
	 * @return		Diverses informations sur le système d'exploitation.
	 * @see			setInformations
	 */
	virtual const IN_STD string& getInformations ( ) const
	{ return _infos; }

	/**
	 * @param		Diverses informations sur le système d'exploitation.
	 * @see			getInformations
	 */
	virtual void setInformations (const IN_STD string& infos)
	{ _infos	= infos; }

	/**
	 * @return		Diverses informations concernant les aspects numériques.
	 * @see			setNumericInformations
	 */
	virtual const IN_STD string& getNumericInformations ( ) const
	{ return _numericInfos; }

	/**
	 * @param		Diverses informations concernant les aspects numériques.
	 * @see			getNumericInformations
	 */
	virtual void setNumericInformations (const IN_STD string& numericInfos)
	{ _numericInfos	= numericInfos; }

	/**
	 * @return		Le système d'exploitation sur lequel s'exécute ce processus.
	 */
	static const OperatingSystem& instance ( );


	private :

	/** Le nom du système d'exploitation. */
	IN_STD string					_name;
	
	/** La distibution du système d'exploitation (RedHat, Ubuntu, ...). */
	IN_STD string					_distribution;		// v 6.14.0

	/** La version du système d'exploitation. */
	Version							_version;

	/** Diverses infos. */
	IN_STD string					_infos;

	/** Informations sur les aspectss numériques. */
	IN_STD string					_numericInfos;

	/** Le système d'exploitation de la machine sur lequel s'exécute ce
	 * processus. */
	static OperatingSystem*			_currentOS;
};	// class OperatingSystem


/**
 * Cette classe permet d'accéder à des informations relatives à la machine
 * où s'exécute le logiciel.
 * @author	Charles PIGNEROL, CEA/DAM/DSSI
 * @todo	De nombreuses choses.
 */
class MachineData : public NetworkData
{
	public :

	/**
	 * Constructeur par défaut. Initialisation avec les valeurs locales au
	 * processus.
	 */
	MachineData ( );

	/**
	 * Constructeurs de copie et opérateur = : RAS.
	 */
	MachineData (const MachineData&);
	MachineData& operator = (const MachineData&);

	/**
	 * Destructeur : RAS.
	 */
	virtual ~MachineData ( );

	/**
	 * @return		Le système matériel.
	 */
	virtual const IN_STD string& getHardware ( ) const
	{ return _hardware; }

	/**
	 * @param		Le système matériel.
	 */
	virtual void setHardware (const IN_STD string& hardware)
	{ _hardware	= hardware; }

	/**
	 * @return		Le nombre de processeurs de la machine. Retourne (size_t)-1
	 *				en cas d'erreur d'initialisation.
	 */
	virtual size_t getProcessorsNum ( ) const
	{ return _processorNum; }

	/**
	 * @param		Le nombre de processeurs de la machine.
	 */
	virtual void setProcessorsNum (size_t num)
	{ _processorNum	= num; }

	/**
	 * @return		Le système d'exploitation de la machine.
	 */
	virtual const OperatingSystem& getOperatingSystem ( ) const
	{ return _operatingSystem; }
	virtual OperatingSystem& getOperatingSystem ( )
	{ return _operatingSystem; }

	/**
	 * @return		La machine où fonctionne ce processus.
	 */
	static const MachineData& instance ( );

	/**
	 * Méthode pouvant être utilisée pour prévenir le lancement d'applications
	 * X/GL sur des machines autres que celle de login (risque de plantage de
	 * station en cas d'incompatibilité).
	 * @return		<I>true</I> si le processus tourne sur une machine autre
	 *				que celle utilisée par l'utilisateur, <I>false</I> dans le
	 *				cas contraire.
	 * @warning		Méthodes de détection assez rudimentaires (SSH_CLIENT,
	 *				SSH_CONNECTION, DISPLAY).
	 */
	static bool isRemoteHost ( );
	
	/**
	 * @return		<I>true</I> si la machine courrante est une machine virtuelle, <I>false</I> dans le cas contraire.
	 * @warning		Méthode de détection assez rudimentaire, repose sur <I>dmesg | grep Hypervisor detected</I> et 
	 * 				<I>lshw | grep QEMU</I>.
	 * @warning		<I>Process::initialize ( )</I> doit avoir été invoqué au préalable.
	 */
	static bool isVirtualMachine ( );	// v 5.12.0

	/**
	 * @return		Le nombre de chiffres significatifs (hors signe, hors
	 *				séparateur décimal . ou ,, hors exposant) pour les réels
	 *				simples précision <I>float</I>. A priori 6 pour des réels
	 *				codés sur 32 bits.
	 */
	static unsigned short floatSignificantNumeral ( );

	/**
	 * @return		Le nombre de chiffres significatifs (hors signe, hors
	 *				séparateur décimal . ou ,, hors exposant) pour les réels
	 *				double précision <I>double</I>. A priori 15 pour des réels
	 *				codés sur 64 bits.
	 */
	static unsigned short doubleSignificantNumeral ( );


	protected :

	/**
	 * Constructeur spécial blindage visant, en cas de chargement de 
	 * multiples instances de la bibliothèque, à alerter l'utilisateur d'un
	 * risque de comportement anormal, voire de plantage.
	 * @param			inutilisé, juste pour avoir une signature différente.
	 * @warning			Ce constructeur n'est à utiliser qu`à des fins de
	 *					blindage de la bibliothèque.
	 */
	MachineData (char);


	private :


	/** Le nombre de processeurs de la machine. */
	size_t								_processorNum;

	/** Le système d'exploitation local. */
	OperatingSystem						_operatingSystem;

	/** Le système matériel. */
	IN_STD string						_hardware;

	/** La machine sur laquelle toutrne ce processus. */
	static MachineData*					_currentMachine;

	/** Blindage contre les instanciations multiples de la bibliothèque. */
	static bool							_instanciated;
};	// class MachineData



END_NAMESPACE_UTIL


#endif	// MACHINE_DATA_H
