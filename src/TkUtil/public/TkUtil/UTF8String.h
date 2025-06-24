#ifndef UTF8_STRING_H
#define UTF8_STRING_H


#include <TkUtil/util_config.h>

#include <iconv.h>
#include <map>
#include <mutex>
#include <string>
#include <iostream>
#include <iomanip>


//BEGIN_NAMESPACE_UTIL
namespace TkUtil {

#ifndef SWIG

#ifndef TKUTIL_WCHAR_T
#define TKUTIL_WCHAR_T
typedef unsigned short WChar_t;
#endif	// TKUTIL_WCHAR_T

class UnicodeString;

#endif	// SWIG


/**
 * <P>Représente un jeu de caractères utilisé pour encoder une chaîne de
 * caractères.</P>
 * <P>Le constructeur prenant en argument une "chaîne de référence" ("àéèùô")
 * permet de déterminer dans quel jeu de caractères est encodé le fichier.
 * </P>
 */
class Charset
{
	public :

	enum CHARSET { UNKNOWN, ASCII, ISO_8859, UTF_8, UTF_16 };

	/**
	 * @param	Jeu de caractères représenté.
	 */
	Charset (CHARSET cs)
		: _charset (cs)
	{ }

	/**
	 * Constructeur de copie, opérateur =. RAS.
	 */
	Charset (const Charset& cs)
		: _charset (cs._charset)
	{ }
	Charset& operator = (const Charset& cs)
	{
		if (&cs != this)
			_charset	= cs._charset;
		return *this;
	}
	Charset& operator = (Charset::CHARSET cs)
	{
		_charset	= cs;
		return *this;
	}

	/**
	 * Destructeur. RAS.
	 */
	~Charset ( )
	{ }

	/**
	 * Constructeur avec auto-détection du jeu de caractères utilisé pour
	 * encoder le fichier courant (où est déclarée l'instance). La
	 * chaîne de caractères "référence" doit être "àéèùô" afin de
	 * reconnaître le bon jeu de caractères.
	 */
	Charset (const char* refString);

	/**
	 * @return	Le jeu de caractères représenté sous forme d'énuméré ou de
	 * 			chaîne de caractère ASCII.
	 */
	CHARSET charset ( ) const
	{ return _charset; }
	std::string name ( ) const;

	/**
	 * @return	Le jeu de caractères correspondant à la chaine de caractères
	 * 			transmise en argument. Ex : <I>Charset::UTF_8</I> pour les
	 * 			chaînes "utf8", "utf-8", "UTF8", "UTF-8".
	 * @see		charset2str
	 */
	static Charset::CHARSET str2charset (const char* str);	// v 4.2.0

	/**
	 * @return	La chaîne de caractères correspondant au jeu de caractères
	 * 			transmis en argument.
	 * @see		str2charset
	 */
	static const char* charset2str (Charset::CHARSET cs);	// v 4.2.0


	private :

	CHARSET			_charset;
};	// class Charset



class UTF16String;

#ifndef SWIG

/**
 * Manipulateur permettant de spécifier le nombre de caractères minimum
 * lors du formatage de nombres entiers en chaines de caractères.
 * @author	Charles PIGNEROL, CEA/DAM/DSSI
 */
class setw
{
	public :

	/**
	 * Constructeurs et opérateur = : RAS.
	 * @param		width est le nombre de caractères minimum.
	 * @param		toCopy est l'instance à recopier.
	 */
	setw (size_t width)
		: _width (width)
	{ }
	setw (const setw& toCopy)
		: _width (toCopy.width ( ))
	{ }
	setw& operator = (const setw& toCopy)
	{ _width	= toCopy.width ( ); return *this; }

	/**
	 * @return		Le nombre de caractères minimum.
	 */
	size_t width ( ) const
	{ return _width; }


	private :

	int	_width;
};	// class setw


/**
 * Manipulateur permettant de spécifier le nombre  de décimales
 * lors du formatage de nombres réels en chaines de caractères.
 * @author	Charles PIGNEROL, CEA/DAM/DSSI
 */
class setprecision
{
	public :

	/**
	 * Constructeurs et opérateur = : RAS.
	 * @param		precision est le nombre de décimales.
	 * @param		toCopy est l'instance à recopier.
	 */
	setprecision (size_t precision)
		: _precision (precision)
	{ }
	setprecision (const setprecision& toCopy)
		: _precision (toCopy.precision ( ))
	{ }
	setprecision& operator = (const setprecision& toCopy)
	{ _precision	= toCopy.precision ( ); return *this; }

	/**
	 * @return		Le nombre de décimales.
	 */
	size_t precision ( ) const
	{ return _precision; }


	private :

	int	_precision;
};	// class setprecision

#endif	// SWIG


/**
 * <P>Cette classe a pour vocation de remplacer la classe <I>UnicodeString</I>
 * qui gére les chaînes de caractères au format unicode <I>UTF-16</I>.
 * <P>
 * Classe de chaîne de caractères unicode "8 bits". Les caractères sont
 * stockés dans des <I>char</I> selon le standard <I>UTF-8</I>.
 * Offre de nombreux ponts vers les types <I>std::string</I>, <I>char*</I>,
 * <I>char</I>, les autres jeux de caractères (<I>ISO-8859</I>, ...),
 * et des possibilités d'insertion de nombres.
 * </P>
 * <P>
 * Cette classe peut également être utilisée comme un <I>stream</I> c++ sortant
 * à l'aide des opérateurs <I><<</I> et de certains <I>manipulateurs</I>.
 * </P>
 * @warning		Cette classe propose de nombreux opérateurs avec les nombres
 *				afin de pouvoir les "streamer".
 * @author		Charles PIGNEROL, CEA/DAM/DSSI
 */
class UTF8String
{
	public :

#ifndef SWIG
	friend UTF8String& operator << (UTF8String& u8str, Charset::CHARSET);
	friend UTF8String& operator << (UTF8String& u8str, const Charset&);
	friend UTF8String& operator << (UTF8String& u8str, setw width);
	friend UTF8String& operator << (UTF8String& u8str, setprecision prec);
	friend UTF8String& operator << (UTF8String& u8str, IN_STD ios_base::fmtflags);
	friend UTF8String operator + (const UTF8String& us, const IN_STD string& str);
	friend UTF8String operator + (const UTF8String& us, const char* str);
	friend UTF8String operator + (const UTF8String& us1, const UTF8String& us2);
#endif	// SWIG

	/**
	 * <P>
	 * Constructeurs, opérateur =. Pour ceux où un Charset est transmis en
	 * argument ce charset est considéré comme jeu de caractères pour les
	 * opérations de streaming << ou opérateurs += à venir.
	 * </P>
	 * <P>Pour les autres constructeurs, exception faite de ceux prenant en
	 * argument <I>const std::string&</I> ou <I>const char*</I>, c'est
	 * UTF8String::defaultCharset qui défini le jeu de caractères utilisé dans
	 * les opérations de streaming.
	 * </P>
	 * <P>Enfin, pour les constructeurs/opérateur = prenant en argument
	 * <I>const std::string&</I> ou <I>const char*</I> <B>il n'y a pas
	 * d'opération de conversion</B>, la chaine transmise étant supposée
	 * en <I>Charset::ASCII</I> et seuls les caractères réllement ASCII étant
	 * recopiés (les éventuels caractères accentués ou exotiques ne sont pas
	 * conservés car ils peuvent poser ultérieurement des problèmes de
	 * conversion).
	 * </P>
	 */
	UTF8String ( );
	UTF8String (const Charset& cs);
	UTF8String (Charset::CHARSET cs);
	UTF8String (const UTF8String& str);
	UTF8String (const IN_UTIL UnicodeString& us);
	UTF8String (const IN_STD string& str);	// ASCII, no conversion
	UTF8String (const IN_STD string& str, const Charset&);
	UTF8String (const IN_STD string& str, Charset::CHARSET);
	UTF8String (const char* str);	// ASCII, no conversion
	UTF8String (const char* str, const Charset&);
	UTF8String (const char* str, Charset::CHARSET);
	UTF8String (char c);
	UTF8String (short s);
	UTF8String (long l);
	UTF8String (unsigned long ul);
	UTF8String (long long l);
	UTF8String (unsigned long long ul);
	UTF8String (float f);
	UTF8String (double d);
	UTF8String (long double d);
	UTF8String& operator = (const UTF8String& str);
	UTF8String& operator = (const IN_STD string& str);	// ASCII, no conversion
	UTF8String& operator = (const char* str);	// ASCII, no conversion
	UTF8String& operator = (char c);
	UTF8String& operator = (short s);
	UTF8String& operator = (long l);
	UTF8String& operator = (unsigned long ul);
	UTF8String& operator = (long long l);
	UTF8String& operator = (unsigned long long ul);
	UTF8String& operator = (float f);
	UTF8String& operator = (double d);
	UTF8String& operator = (long double d);

	/**
	 * Destructeur. RAS.
	 */
	virtual ~UTF8String ( );

	/**
	 * @return		Le jeu de caractères utilisé lors des opérations de
	 * 				streaming (<<).
	 */
	virtual const Charset& localCharset ( ) const
	{ return _localCharset; }

	/**
	 * @param		Nouveau jeu de caractères à utiliser lors des opérations de
	 * 				streaming (<<) ultérieures.
	 */
	virtual void setLocalCharset (const Charset& localCharset)
	{ _localCharset	= localCharset; }

	/**
	 * Réinitialise la chaîne à rien.
	 */
	virtual void clear ( );

	/**
	 * @return		Le nombre d'octets de la chaîne, caractère de terminaison 
	 *				non compris.
	 * @see			charNum
	 */
	IN_STD size_t length ( ) const;

	/**
	 * @return		Le nombre de caractères textuels de la chaîne, caractère de
	 * 				terminaison non compris. Un caractère textuel peut être codé
	 * 				sur plusieurs octets (cas des caractèrs non ASCII).
	 * @see			charAt
	 * @see			length
	 */
	IN_STD size_t charNum ( ) const;

	/**
	 * @return		Un pointeur sur le i-ème caractère UTF-8 de la chaîne, ou 0
	 * 				en cas de dépassement de chaîne.
	 * @see			utf8
	 * @see			charNum
	 */
	const unsigned char* charAt (size_t i) const;

	/**
	 * @return		true si la chaîne est vide, sinon false.
	 */
	virtual bool empty ( ) const;

	/** 
	 * @return		false si la chaîne contient des caractères non ASCII,
	 *				true dans le cas contraire.
	 * @see			ascii
	 * @see			convertedToAscii
	 */
	virtual bool isAscii ( ) const;

	/** 
	 * @return		false si la chaîne contient des caractères non ISO 8859,
	 *				true dans le cas contraire.
	 */
	virtual bool isIso ( ) const;

	/**
	 * @return		La chaîne de caractères sous forme UTF-8. Le tableau est à
	 *				détruire par l'appellant.
	 */
	virtual const IN_STD string& utf8 ( ) const;

	/**
	 * @return		La chaîne de caractères sous forme UTF-16.
	 */
	virtual UTF16String utf16 ( ) const;

	/**
	 * @return		La chaîne de caractères sous forme ASCII. Les caractères
	 *				non ASCII ne sont pas insérés dans la chaîne retournée.
	 * @see			convertedToAscii
	 */
	virtual IN_STD string ascii ( ) const;

	/**
	 * @return		La chaîne de caractères dont les caractères spéciaux
	 * 				(accents, cédille, ...) sont remplacés par le caractère
	 * 				"simple" (ex : 'é', 'è', 'ë' remplacés par 'e', 'ç' par 'c',
	 * 				...
	 * @warning		Il manque certainement des cas => à actualiser.
	 */
	virtual IN_STD string convertedToAscii ( ) const;

	/**
	 * @return		La chaîne de caractères sous forme ISO 8859. Les caractères
	 *				non ISO 8859 ne sont pas insérés dans la chaîne retournée.
	 */
	virtual IN_STD string iso ( ) const;

	/**
	 * @return		La chaîne de caractères formatée selon <I>charset ( )</I>,
	 * 				ou <I>ascii ( )</I> en cas de jeu de caractère indéterminé
	 * 				ou de type <I>UTF_16</I>.
	 * @see			charset
	 * @see			utf8
	 * @see			iso
	 * @see			ascii
	 */
	virtual IN_STD string asCharset ( ) const;

	/**
	 * @return		true si la chaîne contient au moins un espace.
	 */
	virtual bool hasWhiteSpace ( ) const;

	/**
	 * @return		true si les deux chaînes sont égales, sinon false.
	 */
	bool operator == (const UTF8String& toCompare) const;

	/**
	 * @return		true si les deux chaînes sont différentes, sinon false.
	 */
	bool operator != (const UTF8String& toCompare) const;

	/**
	 * @return		utf8 ( )
	 */
	virtual operator IN_STD string ( ) const;

	/**
	 * @return		Le caractère situé à la position donnée en argument.
	 * @throw		Une exception est levée en cas de position invalide.
	 */
	const char* operator [] (size_t pos) const;

	/**
	 * Fonctions de recherche à partir du début.
	 * @param		Elément ou occurence recherchée
	 * @param		Point de départ "informatique" de la recherche (pos-ième
	 * 				char, pas caractère typographique)
	 * @return		la position de l'occurence recherchée, ou (size_t)-1 si
	 *				elle ne figure pas dans la chaîne.
	 * @exception	En cas de position invalide.
	 */
	size_t find (const UTF8String& str, size_t pos = 0) const;
	size_t find (const IN_STD string& str, size_t pos = 0) const;
	size_t find (const char* str, size_t pos = 0) const;
	size_t find (char c, size_t pos = 0) const;

	/**
	 * Fonctions de recherche à partir de la fin.
	 * @param		Elément ou occurence recherchée
	 * @param		Point de départ de la recherche, ou -1 si la recherche doit
	 *				être effectuée `partir de la fin.
	 * @return		la position de l'occurence recherchée, ou (size_t)-1 si
	 *				elle ne figure pas dans la chaîne.
	 * @exception	En cas de position invalide.
	 */
	size_t rfind (const UTF8String& str, size_t pos = (size_t)-1) const;
	size_t rfind (const IN_STD string& str, size_t pos = (size_t)-1) const;
	size_t rfind (const char* str, size_t pos = (size_t)-1) const;
	size_t rfind (char c, size_t pos = (size_t)-1) const;

	/**
	 * Remplace les caractères <I>pattern1</I> par <I>pattern2</I>. Si<I>all</I>
	 * vaut <I>true</I> remplace toutes les occurences, sinon uniquement las
	 * première. Pour la version <I>string</I> les motifs sont considérés comme
	 * étant en UTF-8.
	 * @return		Le nombre de remplacements effectués.
	 */
	virtual size_t replace (
			const IN_STD string& pattern1, const IN_STD string& pattern2,
			bool all);
	virtual size_t replace (
			const UTF8String& pattern1, const UTF8String& pattern2, bool all);

	/**
	 * @param		Premier caractère de la chaîne à extraire.
	 * @param		Dernier caractère de la chaîne à extraire. Transmettre
	 *				(size_t)-1 pour que ce soit le dernier caractère de la
	 *				chaîne.
	 * @return		La chaîne de caractères comprise entre les positions
	 *				first et last.
	 * @exception	Si une des deux positions est invalide.
	 */
	UTF8String substring (size_t first, size_t last = (size_t)-1) const;

	/**
	 * Opérateurs de concaténation. Ajoutent l'argument reçu en fin de chaîne.
	 * Pour les opérateurs prenant en argument std::string et char*, le jeu de
	 * caractères utilisé est celui de l'instance en cours (_localCharset).
	 * @param		Elément à ajouter
	 * @return		Une référence sur cette chaîne.
	 */
	UTF8String& operator += (const UTF8String& str);
	UTF8String& operator += (const IN_STD string& str);
	UTF8String& operator += (const char* str);
	UTF8String& operator += (char c);
	UTF8String& operator += (short s);
	UTF8String& operator += (long l);
	UTF8String& operator += (unsigned long ul);
	UTF8String& operator += (long long l);
	UTF8String& operator += (unsigned long long ul);
	UTF8String& operator += (float f);
	UTF8String& operator += (double d);
	UTF8String& operator += (long double d);
	UTF8String& operator << (const UTF8String& str);
	UTF8String& operator << (const IN_STD string& str);
	UTF8String& operator << (const char* str);
	UTF8String& operator << (char c);
	UTF8String& operator << (short s);
	UTF8String& operator << (long l);
	UTF8String& operator << (unsigned long ul);
	UTF8String& operator << (long long l);
	UTF8String& operator << (unsigned long long ul);
	UTF8String& operator << (float f);
	UTF8String& operator << (double d);
	UTF8String& operator << (long double d);

	/**
	 * Formate la chaîne de caractères sur le nombre de colonne renseigné en
	 * argument. La césure se fait au niveau des espaces.
	 * @return		La chaîne de caractères formatée
	 * @exception	Si columnNum est nul.
	 * @see			trim
	 */
	virtual UTF8String format (size_t columnNum) const;

	/**
	 * @return		La chaîne de caractères dépourvue des espaces et sauts de
	 *				ligne en début et fin de chaîne. Si removeTabs vaut true
	 *				enlève également les tabulations.
	 * @see			format
	 */
	virtual UTF8String trim (bool removeTabs = true) const;

	/**
	 * @return	Le nombre d'octets utilisés par le "caractère" UTF-8 transmis
	 *			en argument.
	 */
	static unsigned char utf8CharSize (const unsigned char* c);

	/**
	 * @return	Une copie de la chaîne transmise en argument dont le caractère
	 * 			UTF-8 'r' est remplacé par le caractère ASCII 'c'.
	 */
	static IN_STD string replaceUtf8Char (
				const IN_STD string& utf8str, const unsigned char* r, char c);

	/**
	 * @return	Une copie de la chaîne de caractères transmise en argument dont
	 * 			les caractères spéciaux (accents, cédille, ...) sont remplacés 
	 * 			par des caractères simples (cf. <I>convertedToAscii</I>) et non
	 * 			pas supprimés comme avec <I>ascii</I>.
	 * @see		convertedToAscii
	 * @see		ascii
	 */
	static IN_STD string replaceSpecialCharacters (const IN_STD string& str);

	/**
	 * Le jeu de caractères par défaut des chaînes transmises par l'opeérateur
	 * de streaming <<. Sa valeur par défaut est déterminée par le construction
	 * d'auto-détection.
	 */
	static Charset					defaultCharset;


	protected :

	/**
	 * Copie l'instance reçue en argument.
	 */
	virtual void copy (const UTF8String& str);

	/**
	 * Copie la chaîne de caractères transmise en 1er argument et de jeu de
	 * caractères transmis en second argument.
	 */
	virtual void copy (const char* str, const Charset& charset);

	/**
	 * Copie les caractères ASCII de la chaîne de caractères transmise en
	 * argument. N'utilise pas <I>iconv</I>.
	 */
	virtual void asciiCopy (const char* str);

	/**
	 * Copie la chaîne UTF-16 reçue en argument.
	 */
	virtual void copy (const IN_UTIL WChar_t* utf16str);

	/**
	 * @return		La longueur de la chaîne unicode tranmise en argument,
	 *				caractère de terminaison non compris.
	 */
	static size_t utf16StringLength (const WChar_t* str);

	/**
	 * Le plus grand caractère ASCII supporté. Le jeu de caractères ASCII va de
	 * 0 à maxASCIIChar.
	 */
	static const unsigned char			maxASCIIChar;

	/**
	 * Le plus grand caractère ISO supporté. Le jeu de caractères ISO 8859 va de
	 * 0 à maxISOChar.
	 */
	static const unsigned char			maxISOChar;

	/**
	 * @return	Une chaîne de caractères encodée avec le jeu transmis en premier
	 * 			argument obtenue à partir de la chaîne de caractères du second
	 * 			argument et encodée dans le jeu de caractèrs en 3ème argument.
	 * 			Version optimisée pour de petites chaînes de caractères, appelle
	 * 			<I>dynTranslate</I> si nécessaire.
	 * @exception	Une exception peut être levée en cas d'erreur. Elle est alors en
	 * 			ASCII afin d'éviter des accès concurrents et dead-locks.
	 * @see		dynTranslate
	 */
	static IN_STD string translate (
				Charset::CHARSET to, const char* str, Charset::CHARSET from);

	/**
	 * @return	Une chaîne de caractères encodée avec le jeu transmis en premier
	 * 			argument obtenue à partir de la chaîne de caractères du second
	 * 			argument et encodée dans le jeu de caractèrs en 3ème argument.
	 * 			Version non optimisée qui passe par des tableaux alloués
	 * 			dynamiquement et en fonction des besoins.
	 * @except	Une exception peut être levée en cas d'erreur. Elle est alors en
	 * 			ASCII afin d'éviter des accès concurrents et dead-locks.
	 * @see		translate
	 */
	static IN_STD string dynTranslate (
				Charset::CHARSET to, const char* str, Charset::CHARSET from);


	private :

	/**
	 * La chaîne de caractères, au format UTF-8.
	 */
	std::string		_utf8String;

	/**
	 * Le jeu de caractères des chaînes de caractères reçues vial l'opérateur
	 * de streaming <<.
	 */
	Charset			_localCharset;

	/**
	 * La largeur des champs numériques.
	 * @warning		Ne concerne que les opérations de streaming (opérateur <<).
	 */
	int				_numericFieldWidth;

	/**
	 * Le nombre de décimale lors de la conversion de réels.
	 * @warning		Ne concerne que les opérations de streaming (opérateur <<).
	 */
	int				_decimalNumber;

	/**
	 * Choix de l'utilisation (true) ou non (false) de la notation scientifique
	 * (avec exposant) ou non lors du formattage de réels en chaînes.
	 * @warning		Ne concerne que les opérations de streaming (opérateur <<).
	 */
	bool			_scientific;

	static iconv_t getIconvDescriptor (Charset::CHARSET from, Charset::CHARSET to);
};	// UTF8String


#ifndef SWIG

/**
 * Le jeu de caractères des chaînes (char*, string) transmises via
 * l'opérateur <<.
 * @warning		Ne concerne que les opérations de streaming (opérateur <<).
 */
UTF8String& operator << (UTF8String& u8str, Charset::CHARSET charset);
UTF8String& operator << (UTF8String& u8str, const Charset& charset);

/**
 * Largeur minimum de formatage des flux entiers.
 * @warning		Ne concerne que les opérations de streaming (opérateur <<).
 */
UTF8String& operator << (UTF8String& u8str, setw width);

/**
 * Nombre de décimales de formatage des flux réels.
 * @warning		Ne concerne que les opérations de streaming (opérateur <<).
 */
UTF8String& operator << (UTF8String& u8str, setprecision prec);

/**
 * Flags de formattage des nombres. Valeurs prises en compte : fixed et
 * scientific.
 * @warning		Ne concerne que les opérations de streaming (opérateur <<).
 */
UTF8String& operator << (UTF8String& u8str, IN_STD ios_base::fmtflags);

/**
 * Injecte str.utf8 ( ) dans os puis retourne os.
 */
IN_STD ostream& operator << (IN_STD ostream& os, const UTF8String& str);

/**
 * @return		La concaténation des deux chaînes reçues en argument.
 */
UTF8String operator + (const UTF8String& us1, const UTF8String& us2);
UTF8String operator + (const UTF8String& us, const IN_STD string& str);
UTF8String operator + (const UTF8String& us, const char* str);
UTF8String operator + (const UTF8String& us, char c);
UTF8String operator + (const UTF8String& us, short s);
UTF8String operator + (const UTF8String& us, long l);
UTF8String operator + (const UTF8String& us, unsigned long ul);
UTF8String operator + (const UTF8String& us, long long l);
UTF8String operator + (const UTF8String& us, unsigned long long ul);
UTF8String operator + (const UTF8String& us, float f);
UTF8String operator + (const UTF8String& us, double d);
UTF8String operator + (const UTF8String& us, long double d);

#endif // SWIG


/**
 * <P>
 * Classe uniquement faite afin d'accéder à la représentation <I>UTF-16</I> de
 * chaînes <I>UTF-8</I>.
 * </P>
 */
class UTF16String
{
	public :

	/**
	 * Constructeur avec adoption du tableau d'octets. Ce tableau sera de ce
	 * fait détruit par cette instance.
	 * @param	Nombre d'octets de la chaîne UTF-16, 0 de terminaison non
	 * 			compris.
	 * @param	Octets de la chaîne UTF-16
	 */
	UTF16String (size_t length, WChar_t* bytes);

	/**
	 * Constructeur avec recopie du tableau d'octets.
	 * @param	Octets de la chaîne UTF-16
	 * @param	Nombre d'octets de la chaîne UTF-16, 0 de terminaison non
	 * 			compris.
	 */
	UTF16String (const WChar_t* bytes, size_t length);

	/**
	 * Constructeur de copie, opérateur = : RAS, recopie du tableau.
	 */
	UTF16String (const UTF16String&);
	UTF16String& operator = (const UTF16String&);

	/**
	 * Destructeur. RAS, destruction du tableau.
	 */
	~UTF16String ( );

	/**
	 * @return	Le nombre d'octets du tableau, 0 de terminaison non compris.
	 */
	size_t length ( ) const
	{ return _length; }

	/**
	 * Accès aux octets de la chaîne.
	 */
	const WChar_t* utf16 ( ) const
	{ return _bytes; }


	private :

	/** Le nombre d'octets du tableau. */
	size_t			_length;

	/** Les octets de la chaîne de caractères UTF-16. */
	WChar_t*		_bytes;
};


/**
 * Classe permettant d'encapsuler l'utilisation de flux type <I>cout</I> et
 * <I>cerr</I> tout en gérant l'encodage qu'ils utilisent.
 */
class ConsoleOutput		// v 4.2.0
{
	public :

	/**
	 * Constructeur.
	 * @param		Flux encapsulé (<I>cout</I>, ...). Ne l'adopte pas.
	 */
	ConsoleOutput (std::ostream& stream)
		: _stream (stream), _charset (TkUtil::Charset::UTF_8)
	{ }

	/**
	 * Destructeur. Ne fait rien !
	 */
	~ConsoleOutput ( )
	{ }

	/**
	 * Accesseurs sur le jeu de caractères utilisé lors de l'injection de
	 * chaînes de caractères dans le flux encapsulé.
	 */
	void setCharset (const TkUtil::Charset& charset)
	{ _charset	= charset; }
	const TkUtil::Charset& getCharset ( ) const
	{ return _charset; }

#ifndef SWIG
	ConsoleOutput& operator << (const TkUtil::UTF8String&);
	ConsoleOutput& operator << (unsigned long);
	ConsoleOutput& operator << (double);
	ConsoleOutput& operator << (const std::string&);
	ConsoleOutput& operator << (const char*);
	ConsoleOutput& operator << (char);
	ConsoleOutput& operator << (std::ios_base::fmtflags);
	friend ConsoleOutput& operator << (ConsoleOutput&, std::_Setprecision);
	friend ConsoleOutput& operator << (ConsoleOutput&, std::_Setw);
	friend ConsoleOutput& co_endl (ConsoleOutput&);
	ConsoleOutput& operator << (ConsoleOutput& (*pf)(ConsoleOutput&))
	{ return pf (*this); }	// Pour endl et autres manipulateurs.
#endif	// SWIG

	/**
	 * @return	Une référence sur la sortie standard.
	 */
	static ConsoleOutput& cout ( );

	/**
	 * @return	Une référence sur la sortie erreur.
	 */
	static ConsoleOutput& cerr ( );


	private :

	/**
	 * Constructeur de copie et opérateur = : interdits.
	 */
	ConsoleOutput (const ConsoleOutput& co)
		: _stream (co._stream), _charset (co._charset)
	{ }
	ConsoleOutput& operator = (const ConsoleOutput& co)
	{ return *this; }

	static ConsoleOutput*	_cout;
	static ConsoleOutput*	_cerr;
	std::ostream&			_stream;
	TkUtil::Charset			_charset;
};	// class ConsoleOutput

#ifndef SWIG
ConsoleOutput& operator << (ConsoleOutput&, std::_Setprecision);
ConsoleOutput& operator << (ConsoleOutput&, std::_Setw);
ConsoleOutput& co_endl (ConsoleOutput&);
#endif	// SWIG


//END_NAMESPACE_UTIL
}	// namespace TkUtil

#endif	// UTF8_STRING_H
