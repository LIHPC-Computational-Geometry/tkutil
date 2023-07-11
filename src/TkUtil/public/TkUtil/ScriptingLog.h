#ifndef SCRIPTING_LOG_H
#define SCRIPTING_LOG_H

#include <TkUtil/util_config.h>
#include <TkUtil/DefaultLog.h>
#include <TkUtil/ReferencedObject.h>
#include <TkUtil/TaggedValue.h>

#include <string>
#include <vector>


BEGIN_NAMESPACE_UTIL


/**
 * <P>Classe représentant des logs pour le scripting. Chaque instance de cette
 * classe véhicule une instruction à exécuter dans un script.
 * </P>
 * <P>Des variables du script peuvent être balisées en vue, par exemple, d'être
 * remplacées par d'autres valeurs.
 * </P>
 * @author	Charles PIGNEROL, CEA/DAM/DSSI
 * @warning		Les instances utilisées dans les scripts sont supposées avoir
 *				des noms uniques.
 */
class ScriptingLog : public DefaultLog
{
	public :

	/**
	 * Constructeur.
	 * @param		Chaîne de caractère permettant d'identifier l'instance à invoquer (nom d'instance unique).
	 * @param		Nom de la méthode à invoquer.
	 * @param		Nom de la variable recevant le résultat, s'il y en a une, ou une chaine vide en l'absence d'affectation.
	 * @param		Eventuel commentaire de l'instruction.
	 */
	ScriptingLog (const IN_STD string& name, const IN_STD string& method, const IN_STD string& result, const UTF8String& comment = UTF8String ( ));

	/**
	 * Constructeur.
	 * @param		Objet référencé à invoquer. L'instance sera récupérée depuis un script en utilisant son nom unique (<I>getUniqueName</I>)
	 *				via la classe <I>ReferencedObjectManager</I>.
	 * @param		Nom de la méthode à invoquer.
	 * @param		Nom de la variable recevant le résultat, s'il y en a une, ou une chaine vide en l'absence d'affectation.
	 * @param		Eventuel commentaire de l'instruction.
	 */
	ScriptingLog (const ReferencedNamedObject& object, const IN_STD string& method, const IN_STD string& result = "", const UTF8String& comment = UTF8String ( ));

	/**
	 * Constructeur "instruction formattée".
	 * @param		L'instruction, déjà formattée dans le langage script cible. Cette instruction est écrite telle que.
	 * @param		Eventuel commentaire de l'instruction.
	 */
	ScriptingLog (const IN_STD string& instruction, const UTF8String& comment);

	/**
	 * Constructeur de copie et opérateur = : RAS.
	 */
	ScriptingLog (const ScriptingLog&);
	ScriptingLog& operator = (const ScriptingLog&);

	/**
	 * Destructeur : RAS.
	 */
	virtual ~ScriptingLog ( );

	/**
	 * @return		Une copie de l'instance. Cette copie est à prendre en charge par l'appelant, y compris sa destruction.
	 */
	virtual Log* clone ( ) const;

	/**
	 * @return		<I>true</I> si le log est formatté, c'est à dire qu'à sa création le texte a été transmis tel qu'il doit être écrit,
	 *				<I>false</I> dans le cas contraire.
	 */
	virtual bool isFormated ( ) const
	{ return _formated; }

	/**
	 * Le log sous forme textuelle.
	 */
	virtual const UTF8String& getText ( ) const;

	/**
	 * @return		Nom unique de l'instance à invoquer.
	 * @see			setName
	 */
	virtual const IN_STD string& getName ( ) const
	{ return _name; }

	/**
	 * @param		Nom unique de l'instance à invoquer.
	 * @see			getName
	 */
	virtual void setName (const IN_STD string& name);

	/**
	 * @return		Le nom de la méthode à invoquer.
	 * @see			setMethodName
	 */
	virtual const IN_STD string& getMethodName ( ) const
	{ return _methodName; }

	/**
	 * @param		Le nom de la méthode à invoquer.
	 * @see			getMethodName
	 */
	virtual void setMethodName (const IN_STD string& method);

	/**
	 * @return		Le nom de la variable recevant le résultat, s'il y en a une, ou une chaine vide.
	 * @see			hasResult
	 * @see			setResult
	 */
	virtual const IN_STD string& getResult ( ) const
	{ return _result; }

	/**
	 * @return		true en cas de résultat à affecter à une variable, false dans le cas contraire.
	 * @see			getResult
	 * @see			setResult
	 */
	virtual bool hasResult ( ) const;

	/**
	 * @param		Le nom de la variable recevant le résultat, ou une chaine vide en l'absence d'affectation à faire.
	 * @see			getResult
	 */
	virtual void setResult (const IN_STD string& result);

	/**
	 * @return		true si des couples (balise, valeur) sont associés à l'instruction, false dans le cas contraire.
	 * @see			getTags
	 * @see			addTag
	 */
	virtual bool hasTags ( ) const
	{ return 0 == _tags.size ( ) ? false : true; }

	/**
	 * @return		Les couples (balise, valeur) associés à l'instruction.
	 * @see			hasTags
	 * @see			addTag
	 */
	virtual const IN_STD vector <TaggedValue>& getTags ( ) const
	{ return _tags; }

	/**
	 * Ajoute le couple (balise, valeur) sans attributs aux valeurs balisées associés à l'instruction.	
	 * @see			getTags
	 */
	virtual void addTag (const IN_STD string& name, const IN_STD string& value);

	/**
	 * Ajoute la valeur balisée transmise en arguments aux valeurs associées à l'instruction.	
	 * @see			getTags
	 */
	virtual void addTag (const TaggedValue& taggedValue);

	/**
	 * @return		Les éventuels commentaires associés à l'instruction.
	 * @see			setComment
	 */
	virtual const UTF8String& getComment ( ) const
	{ return _comment; }

	/**
	 * @param		Les éventuels commentaires associés à l'instruction.
	 * @see			getComment
	 */
	virtual void setComment (const UTF8String& comment);

	/**
	 * Les types d'arguments supportés pour les méthodes.
	 */	
	enum ARG_TYPE { STRING, BOOL, LONG, DOUBLE, INSTANCE };

	/**
	 * @return		Le nombre d'arguments de la méthode.
	 * @see			getMethodArgument
	 * @see			addMethodArgument
	 */
	virtual size_t getMethodArgumentsCount ( ) const
	{ return _arguments.size ( ); }

	/**
	 * @return		Le i-ème argument de la méthode sous forme de paire type-valeur (premier indice : 0).
	 * @exception	Une exception est levée en cas d'indice invalide.
	 * @see			getMethodArgumentsCount
	 */
	virtual IN_STD pair < ARG_TYPE, IN_STD string > getMethodArgument (size_t i) const;

	/**
	 * Ajoute à l'appel de la méthode l'argument de type chaîne de caractères dont la valeur est transmise en argument.
	 * @param		Valeur de l'argument
	 * @see			getMethodArgumentsCount
	 * @see			getMethodArgument
	 */
	virtual void addMethodArgument (const IN_STD string& arg);
	virtual ScriptingLog& operator << (const IN_STD string& arg);
	virtual void addMethodArgument (const char* arg);
	virtual ScriptingLog& operator << (const char* arg);

	/**
	 * Ajoute à l'appel de la méthode l'argument de type bouléen dont la valeur est transmise en argument.
	 * @param		Valeur de l'argument
	 * @see			getMethodArgumentsCount
	 * @see			getMethodArgument
	 */
	virtual void addMethodArgument (bool arg);
	virtual ScriptingLog& operator << (bool arg);

	/**
	 * Ajoute à l'appel de la méthode l'argument de type entier long dont la valeur est transmise en argument.
	 * @param		Valeur de l'argument
	 * @see			getMethodArgumentsCount
	 * @see			getMethodArgument
	 */
	virtual void addMethodArgument (long arg);
	virtual ScriptingLog& operator << (long arg);

	/**
	 * Ajoute à l'appel de la méthode l'argument de type réel double précision dont la valeur est transmise en argument.
	 * @param		Valeur de l'argument
	 * @see			getMethodArgumentsCount
	 * @see			getMethodArgument
	 */
	virtual void addMethodArgument (double arg);
	virtual ScriptingLog& operator << (double arg);

	/**
	 * Ajoute à l'appel de la méthode l'instance dont le nom dans le script est transmis en argument. Se distingue de la méthode de même nom
	 * prenant en argument une instance de la classe ReferencedNamedObject par le fait que la variable peut être de n'importe quel type et qu'elle ne
	 * sera donc pas appelée dans le script via l'API ReferencedObjectManager.
	 * @see			getMethodArgumentsCount
	 * @see			getMethodArgument
	 */
	virtual void addVariable (const IN_STD string& name);

	/**
	 * Ajoute à l'appel de la méthode l'instance dont le nom dans le script est transmis en argument.
	 * @see			getMethodArgumentsCount
	 * @see			getMethodArgument
	 * @see			ReferencedObjectManager
	 */
	virtual void addMethodArgument (const ReferencedNamedObject* object);
	virtual ScriptingLog& operator << (const ReferencedNamedObject* object);

	/**
	 * Ajoute à l'appel de la méthode l'argument de type transmis en argument dont la valeur est transmise en second argument.
	 * A utiliser avec parcimonie !
	 */
	 virtual void addMethodArgument (ARG_TYPE, const std::string&);		// v 6.4.0

	/**
	 * @return		<I>true</I> si l'instance doit être récupérée par le script en utilisant l'API de la classe
	 *				<I>ReferencedObjectManager</I>, <I>false</I> dans le cas contraire.
	 * @see			getNamedObject
	 */
	virtual bool useObjectManagerApi ( ) const
	{ return 0 == _namedObject ? false : true; }

	/**
	 * @return		Un pointeur sur l'éventuel objet référencé à invoquer.
	 * @see			useObjectManagerApi
	 */
	virtual const ReferencedNamedObject* getNamedObject ( ) const
	{ return _namedObject; }


	private :

	/** Le log est il déjà formatté ? */
	bool													_formated;

	/** Le nom de l'instance à invoquer. */
	IN_STD string											_name;

	/** Le nom de la méthode à invoquer. */
	IN_STD string											_methodName;

	/** Le nom de l'éventuelle variable du script recevant le résultat. */
	IN_STD string											_result;

	/** Les couples (balise, valeur) éventuellement associés à l'instruction. */
	IN_STD vector< TaggedValue >							_tags;

	/** L'éventuel commentaire associé à la méthode. */
	UTF8String												_comment;

	/** La liste des arguments. */	
	IN_STD vector < IN_STD pair <ARG_TYPE, IN_STD string> >	_arguments;

	/** Pas très beau, pour maintenir l'API. */
	mutable UTF8String										_text;

	/** L'éventuel objet référencé. */
	mutable const ReferencedNamedObject*					_namedObject;
};	// class ScriptingLog


END_NAMESPACE_UTIL


#endif	// SCRIPTING_LOG_H
