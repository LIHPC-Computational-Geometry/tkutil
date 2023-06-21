#ifndef LOCAL_NETWORK_H
#define LOCAL_NETWORK_H

#include "TkUtil/util_config.h"
#include <string>
#include <vector>




BEGIN_NAMESPACE_UTIL


/**
 * <P>Cette classe permet d'effectuer des services spécifiques au réseau local.
 * </P>
 *
 * <P>Dans la version actuelle, un nom de machine est du type :
 * <UL>
 * <LI><I>nom</I> : station de travail ou calculateur ou noeud de calculateur,
 * <LI><I>nom.calculateur</I> : noeud du calculateur mentionné, <I>nom</I> étant supposé contenir le numéro du noeud.
 * </UL>
 * </P>
 * @author	Charles PIGNEROL, CEA/DAM/DSSI
 */
class LocalNetwork
{
	public :
	
	struct HPComputer
	{
		HPComputer ( )
			: name ( ), localName ( ), endPointSuffix ( ), noLimit (false)
		{ }
		
		HPComputer (const HPComputer& hpc)
			: name (hpc.name), localName (hpc.localName), endPointSuffix (hpc.endPointSuffix), noLimit (hpc.noLimit)
		{ }
		
		HPComputer& operator = (const HPComputer& hpc)
		{
			if (&hpc != this)
			{
				 name			= hpc.name;
				 localName		= hpc.localName;
				 endPointSuffix	= hpc.endPointSuffix;
				 noLimit		= hpc.noLimit;
			 }
			 return *this;
		 }	// HPComputer::operator =
		
		/** Nom public du calculateur. */
		std::string		name;
		
		/** Nom privé du calculateur, c'est à dire vu de lui même. */
		std::string		localName;
		
		/** Suffixe appliqué à l'adresse du noeud sur laquelle ouvrir un port d'écoute côté serveur en cas de NAT. */
		std::string		endPointSuffix;
		
		/** Est-ce un noeud no-limit ? */
		bool			noLimit;
	};	// struct HPComputer

	/**
	 * @param		Infos des calculateurs du réseau local.
	 * @see			getEndPoint
	 * @see			isComputer
	 */
	static void initialize (const std::vector<HPComputer>& computerNames);	// v 5.9.0
	 
	/**
	 * @return		Informations sur le calculateur dont le nom est transmis en argument.
	 * @exception	Une exception est levée si le calculateur demandé n'est pas recensé.
	 * @see			initialize
	 */
	static HPComputer getComputerInfos (const IN_STD string& name);		// v 5.9.0
	 
	/**
	 * @param		Nom simple de la machine à laquelle on souhaite se connecter.
	 * @return		Le nom complet de la machine, nom de domaine éventuellement compris, pour que l'on puisse se connecter directement à
	 *				cette machine via <I>ssh</I> (ou autre).
	 */
	static IN_STD string getFullName (const IN_STD string& name);

	/**
	 * @param		Nom de machine ou calculateur à laquelle on va se connecter.
	 * @param		Suffixe appliqué à l'adresse du noeud sur laquelle ouvrir un port d'écoute côté serveur en cas de NAT.
	 * @return		Le endpoint CORBA correspondant à cette machine selon les règles actuellement en vigueur. Si <I>name</I> est
	 * 				une adresse TCP/IP elle est retournée.
	 * @since		version 5.10.0
	 * @see			initialize
	 */
	static IN_STD string getEndPoint (const IN_STD string& name, const IN_STD string& endPointSuffix);
	
	/**
	 * @param		Machine ou calculateur à laquelle on va se connecter.
	 * @return		Le endpoint CORBA correspondant à cette machine selon les règles actuellement en vigueur.
	 * @since		version 3.34.0
	 * @see			initialize
	 */
	static IN_STD string getEndPoint (const IN_STD string& name);

	/**
	 * @param		Machine ou calculateur à laquelle on va se connecter (=> serveur).
	 * @return		Le endpoint CORBA vu du serveur correspondant à cette machine selon les règles actuellement en vigueur.
	 *				Lance un processus sur le serveur pour récupére le nom du noeud du calculateur qui peut ne pas être public,
	 *				c.a.d. vu de l'extérieur (NAT). A ce nom est ensuite appliqué <I>getEndPoint (const std::string&)</I>.
	 * @since		version 5.10.0
	 */
	static IN_STD string getNATedEndPoint (const IN_STD string& name);
	
	/**
	 * @param		Nom de machine évalué.
	 * @return		<I>true</I> si la machine est un calculateur, <I>false</I> dans le cas contraire.
	 */
	static bool isComputer (const IN_STD string& name);

	/**
	 * @param		Nom de machine (noeud d'un calculateur ?).
	 * @return		le nom de machine (noeud) auquel est rattaché la machine dont le nom est passé en argument ou une chaîne vide.
	 */
	static IN_STD string getMachineName (const IN_STD string& name);

	/**
	 * @param		Nom de calculateur.
	 * @return		le nom du calculateur auquel est rattaché la machine dont le nom est passé en argument ou une chaîne vide.
	 * @see			initialize
	 */
	static IN_STD string getComputerName (const IN_STD string& name);

	/**
	 * @param		Nom de machine évalué (idéalement nom qualifié pour un calculateur).
	 * @return		Adresse IP valide pour accéder à cette machine depuis la machine locale (certaines machines ont plusieures
	 *				adresse IP sur des réseaux différents).
	 */
	static IN_STD string getInetAddress (const IN_STD string& name);

	/**
	 * @return		Le home utilisateur
	 * @param		Machine considérée
	 */
	static IN_STD string getUserHome (const IN_STD string& computerName);

	/**
	 * Méthode pouvant être utilisée pour prévenir le lancement d'applications
	 * X/GL sur des machines autres que celle de login (risque de plantage de
	 * station en cas d'incompatibilité).
	 * @return		<I>true</I> si le processus tourne sur une machine autre
	 *				que celle utilisée par l'utilisateur, <I>false</I> dans le
	 *				cas contraire.
	 * @warning		Méthodes de détection assez rudimentaires (SSH_CLIENT,
	 * 				SSH_CONNECTION, DISPLAY).
	 */
	static bool isRemoteHost ( );

	/**
	 * @return		La commande utilisée pour obtenir le "home utilisateur".
	 * @see			setHomeCommand
	 */
	static std::string getHomeCommand ( );

	/**
	 * @param		La commande utilisée pour obtenir le "home utilisateur". Vaut "home" par défaut.
	 * @see			getHomeCommand
	 */
	static void setHomeCommand (const std::string& command);	// v 6.1.0

	private :

	/** La commande "home". */
	static std::string					_homeCommand;			// v 6.1.0
	
	/** Les noms de calculateur. */
	static std::vector<HPComputer>		_computers;				// v 5.9.0
	
	/**
	 * Constructeurs, destructeur : interdits.
	 */
	LocalNetwork ( );
	LocalNetwork (const LocalNetwork&);
	LocalNetwork& operator = (const LocalNetwork&);
	~LocalNetwork ( );
};	// class LocalNetwork



END_NAMESPACE_UTIL


#endif	// LOCAL_NETWORK_H
