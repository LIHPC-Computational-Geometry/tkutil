#include "TkUtil/UnicodeString.h"
#include "TkUtil/MemoryError.h"
#include "TkUtil/NumericConversions.h"
#include "TkUtil/Exception.h"

#include <string.h>		// memcpy, ...
#include <assert.h>
#include <sstream>


USING_STD


BEGIN_NAMESPACE_UTIL

static const Charset	charset ("àéèùô");

//
// Mémo :
//
// ASCII :
// -------
//
// De 0 à 127
//
// ISO 8859 :
// ----------
//
// - Codé sur des unsigned char, de 0 à 255
// - Les 127 premiers caractères sont ceux du jeu ASCII
// - Les suivants varient selon 16 jeux :
//		- Latin 1 : Europe de l'Ouest, dont la France, le Royaume-Uni,
//		l'Allemagne, l'Italie, l'Espagne, ...
//		- Latin 2 : Europe centrale
//		- Latin 3 : Europe du sud
//		- Latin 4 : Europe du Nord
//		- Cyrillic : Pays slaves
//		...
//
// Unicode : (UTF-16)
// ---------
//
// - Codé de 1 un 4 octets
// - Les 256 premiers caractères unicode sont les mêmes que ceux du jeu 
// ISO 8859-1.
//

// Source : Xerces, XMLUniDefs.hpp
const unsigned char	UnicodeString::maxASCIIChar			= 0x7F;		// 127
const unsigned char	UnicodeString::maxISOChar			= 255;
const WChar_t		UnicodeString::unicodeWhiteSpace	= ' ';
const WChar_t		UnicodeString::unicodeLineFeed		= 0x0A;


static const int	defaultDecimalNumber	= ostringstream ( ).precision ( );


UnicodeString::UnicodeString ( )
	: _string (0), _length (0), 
	  _numericFieldWidth (0), _decimalNumber (defaultDecimalNumber), 
	  _scientific (true)
{
}	// UnicodeString::UnicodeString


UnicodeString::UnicodeString (const UnicodeString& str)
	: _string (0), _length (0),
	  _numericFieldWidth (0), _decimalNumber (defaultDecimalNumber), 
	  _scientific (true)
{
	copy (str);
}	// UnicodeString::UnicodeString


UnicodeString::UnicodeString (const UTF8String& ustr)
	: _string (0), _length (0),
	  _numericFieldWidth (0), _decimalNumber (defaultDecimalNumber), 
	  _scientific (true)
{
	copy (ustr.utf16 ( ).utf16 ( ));
}	// UnicodeString::UnicodeString


UnicodeString::UnicodeString (const WChar_t* str)
	: _string (0), _length (0),
	  _numericFieldWidth (0), _decimalNumber (defaultDecimalNumber), 
	  _scientific (true)
{
	copy (str);
}	// UnicodeString::UnicodeString


UnicodeString::UnicodeString (const string& str)
	: _string (0), _length (0),
	  _numericFieldWidth (0), _decimalNumber (defaultDecimalNumber), 
	  _scientific (true)
{
	copy (str);
}	// UnicodeString::UnicodeString


UnicodeString::UnicodeString (const char* str)
	: _string (0), _length (0),
	  _numericFieldWidth (0), _decimalNumber (defaultDecimalNumber), 
	  _scientific (true)
{
	copy (string (str));
}	// UnicodeString::UnicodeString


UnicodeString::UnicodeString (WChar_t wc)
	: _string (0), _length (0),
	  _numericFieldWidth (0), _decimalNumber (defaultDecimalNumber), 
	  _scientific (true)
{
	WChar_t	array [2]	= {wc, 0};
	copy (array);
}	// UnicodeString::UnicodeString


UnicodeString::UnicodeString (char c)
	: _string (0), _length (0),
	  _numericFieldWidth (0), _decimalNumber (defaultDecimalNumber), 
	  _scientific (true)
{
	copy (string (&c, 1));
}	// UnicodeString::UnicodeString (char)


UnicodeString::UnicodeString (short s)
	: _string (0), _length (0),
	  _numericFieldWidth (0), _decimalNumber (defaultDecimalNumber), 
	  _scientific (true)
{
	copy (NumericConversions::toStr (s));
}	// UnicodeString::UnicodeString (short)


UnicodeString::UnicodeString (long l)
	: _string (0), _length (0),
	  _numericFieldWidth (0), _decimalNumber (defaultDecimalNumber), 
	  _scientific (true)
{
	copy (NumericConversions::toStr (l));
}	// UnicodeString::UnicodeString (long)


UnicodeString::UnicodeString (unsigned long ul)
	: _string (0), _length (0),
	  _numericFieldWidth (0), _decimalNumber (defaultDecimalNumber), 
	  _scientific (true)
{
	copy (NumericConversions::toStr (ul));
}	// UnicodeString::UnicodeString (unsigned long)


UnicodeString::UnicodeString (long long l)
	: _string (0), _length (0),
	  _numericFieldWidth (0), _decimalNumber (defaultDecimalNumber), 
	  _scientific (true)
{
	copy (NumericConversions::toStr (l));
}	// UnicodeString::UnicodeString (long long)


UnicodeString::UnicodeString (unsigned long long ul)
	: _string (0), _length (0),
	  _numericFieldWidth (0), _decimalNumber (defaultDecimalNumber), 
	  _scientific (true)
{
	copy (NumericConversions::toStr (ul));
}	// UnicodeString::UnicodeString (unsigned long long)


UnicodeString::UnicodeString (float f)
	: _string (0), _length (0),
	  _numericFieldWidth (0), _decimalNumber (defaultDecimalNumber), 
	  _scientific (true)
{
	copy (NumericConversions::toStr (f));
}	// UnicodeString::UnicodeString (float)


UnicodeString::UnicodeString (double d)
	: _string (0), _length (0),
	  _numericFieldWidth (0), _decimalNumber (defaultDecimalNumber), 
	  _scientific (true)
{
	copy (NumericConversions::toStr (d));
}	// UnicodeString::UnicodeString (double)


UnicodeString::UnicodeString (long double ld)
	: _string (0), _length (0),
	  _numericFieldWidth (0), _decimalNumber (defaultDecimalNumber), 
	  _scientific (true)
{
	copy (NumericConversions::toStr (ld));
}	// UnicodeString::UnicodeString (long double)


UnicodeString& UnicodeString::operator = (const UnicodeString& str)
{
	if (&str != this)
		copy (str);

	return *this;
}	// UnicodeString::operator =


UnicodeString& UnicodeString::operator = (const WChar_t* str)
{
	if (str != _string)
		copy (UnicodeString (str));

	return *this;
}	// UnicodeString::operator =


UnicodeString& UnicodeString::operator = (const string& str)
{
	copy (str);

	return *this;
}	// UnicodeString::operator =


UnicodeString& UnicodeString::operator = (const char* str)
{
	copy (string (str));

	return *this;
}	// UnicodeString::operator =


UnicodeString& UnicodeString::operator = (WChar_t wc)
{
	WChar_t	array [2]	= {wc, 0};
	copy (array);

	return *this;
}	// UnicodeString::operator =


UnicodeString& UnicodeString::operator = (char c)
{
	copy (string (&c, 1));

	return *this;
}	// UnicodeString::operator =


UnicodeString& UnicodeString::operator = (short s)
{
	copy (UnicodeString (s));

	return *this;
}	// UnicodeString::operator =


UnicodeString& UnicodeString::operator = (long l)
{
	copy (UnicodeString (l));

	return *this;
}	// UnicodeString::operator =


UnicodeString& UnicodeString::operator = (unsigned long ul)
{
	copy (UnicodeString (ul));

	return *this;
}	// UnicodeString::operator =


UnicodeString& UnicodeString::operator = (long long ll)
{
	copy (UnicodeString (ll));

	return *this;
}	// UnicodeString::operator =


UnicodeString& UnicodeString::operator = (unsigned long long ull)
{
	copy (UnicodeString (ull));

	return *this;
}	// UnicodeString::operator =


UnicodeString& UnicodeString::operator = (float f)
{
	copy (UnicodeString (f));

	return *this;
}	// UnicodeString::operator =


UnicodeString& UnicodeString::operator = (double d)
{
	copy (UnicodeString (d));

	return *this;
}	// UnicodeString::operator =


UnicodeString& UnicodeString::operator = (long double d)
{
	copy (UnicodeString (d));

	return *this;
}	// UnicodeString::operator =


UnicodeString::~UnicodeString ( )
{
	clear ( );
}	// UnicodeString::~UnicodeString


void UnicodeString::clear ( )
{
	delete [] _string;
	_string	= 0;
	_length	= 0;
}	// UnicodeString::clear


size_t UnicodeString::length ( ) const
{
	return _length;
}	// UnicodeString::length


bool UnicodeString::empty ( ) const
{
	return 0 == length ( ) ? true : false;
}	// UnicodeString::empty


bool UnicodeString::isAscii ( ) const
{
	for (size_t i = 0; i < _length; i++)
		if (_string [i] > maxASCIIChar)
			return false;

	return true;
}	// UnicodeString::isAscii


bool UnicodeString::isIso ( ) const
{
	for (size_t i = 0; i < _length; i++)
		if (_string [i] > maxISOChar)
			return false;

	return true;
}	// UnicodeString::isIso


const WChar_t* UnicodeString::unicode ( ) const
{
	return _string;
}	// UnicodeString::unicode


string UnicodeString::ascii ( ) const
{
	string			str;
	const WChar_t*	uStr	= unicode ( );
	const size_t	l		= length ( );
	str.reserve (l + 1);

	for (size_t i = 0; i < l; i++)
	{
		if (uStr [i] <= maxASCIIChar)
			str	+= ((char)uStr [i]);
	}	// for (size_t i = 0; i < l + 1; l++)

	return str;
}	// UnicodeString::ascii


string UnicodeString::iso ( ) const
{
	string			str;
	const WChar_t*	uStr	= unicode ( );
	const size_t	l		= length ( );
	str.reserve (l + 1);

	for (size_t i = 0; i < l; i++)
	{
		if (uStr [i] <= maxISOChar)
			str	+= ((char)uStr [i]);
	}	// for (size_t i = 0; i < l + 1; l++)

	return str;
}	// UnicodeString::iso


bool UnicodeString::hasWhiteSpace ( ) const
{
	const size_t	l	= length ( );
	const WChar_t*	str	= unicode ( );
	
	for (size_t i = 0; i < l; i++)
		if (unicodeWhiteSpace & str [i])	// From Xerces
//		if (' ' == (char)str [i])
			return true;

	return false;
}	// UnicodeString::hasWhiteSpace


bool UnicodeString::operator == (const UnicodeString& toCompare) const
{
	if (length ( ) != toCompare.length ( ))
		return false;

	return 0 == memcmp (unicode ( ), toCompare.unicode ( ), length ( ) * sizeof (WChar_t));
}	// UnicodeString::operator ==


bool UnicodeString::operator != (const UnicodeString& toCompare) const
{
	return !(*this == toCompare);
}	// UnicodeString::operator ==


UnicodeString::operator string ( ) const
{
	return iso ( );
}	// UnicodeString::operator


WChar_t UnicodeString::operator [] (size_t pos) const
{
	if (pos >= length ( ))
		throw Exception (UTF8String ("UnicodeString::operator [] : position invalide.", charset));

	return unicode ( ) [pos];
}	// UnicodeString::operator []


size_t UnicodeString::find (const UnicodeString& str, size_t pos) const
{
	if (pos >= length ( ))
		throw Exception (UTF8String ("UnicodeString::find : position initiale invalide.", charset));
	if ((true == str.empty ( )) || (length ( ) < str.length ( )))
		return (size_t)-1;

	const WChar_t*	toFind	= str.unicode ( );
	const size_t	size	= str.length ( ) * sizeof (WChar_t);
	const size_t	stop	= length ( ) - str.length ( );
	for ( ; pos <= stop; pos++)
		if (0 == memcmp (unicode ( ) + pos, toFind, size))
			return pos;

	return (size_t)-1;
}	// UnicodeString::find (const UnicodeString& str, size_t pos)


size_t UnicodeString::find (const WChar_t* str, size_t pos) const
{
	return find (UnicodeString (str), pos);
}	// UnicodeString::find (const WChar_t* str, size_t pos)


size_t UnicodeString::find (const string& str, size_t pos) const
{
	return find (UnicodeString (str), pos);
}	// UnicodeString::find (const string& str, size_t pos)


size_t UnicodeString::find (const char* str, size_t pos) const
{
	return find (UnicodeString (str), pos);
}	// UnicodeString::find (const char* str, size_t pos)


size_t UnicodeString::find (WChar_t wc, size_t pos) const
{
	return find (UnicodeString (wc), pos);
}	// UnicodeString::find (WChar_t wc, size_t pos)


size_t UnicodeString::find (char c, size_t pos) const
{
	return find (UnicodeString (c), pos);
}	// UnicodeString::find (char c, size_t pos)


size_t UnicodeString::rfind (const UnicodeString& str, size_t pos) const
{
	const size_t	strLength	= str.length ( );
	if ((size_t)-1 == pos)
		pos	= length ( ) - 1;

	if (pos >= length ( ))
		throw Exception (UTF8String ("UnicodeString::rfind : position initiale invalide.", charset));
	if ((0 == strLength) || (length ( ) < strLength))
		return (size_t)-1;

	// Eviter les comparaisons au delà de la chaine :
	if (pos + strLength >= length ( ))
		pos	= length ( ) - 1 - strLength;
	const WChar_t*	toFind	= str.unicode ( );
	size_t			size	= strLength * sizeof (WChar_t);
	// coverity : pos >= 0 toujours vrai
//	for ( ; pos >= 0; pos--)
	for ( ; (size_t)-1 != pos; pos--)
		if (0 == memcmp (unicode ( ) + pos, toFind, size))
			return pos;

	return (size_t)-1;
}	// rfind (const UnicodeString&, size_t)


size_t UnicodeString::rfind (const WChar_t* str, size_t pos) const
{
	return rfind (UnicodeString (str), pos);
}	// rfind (const WChar_t*, size_t)


size_t UnicodeString::rfind (const string& str, size_t pos) const
{
	return rfind (UnicodeString (str), pos);
}	// rfind (const string&, size_t)


size_t UnicodeString::rfind (const char* str, size_t pos) const
{
	return rfind (UnicodeString (str), pos);
}	// rfind (const char*, size_t)


size_t UnicodeString::rfind (WChar_t wc, size_t pos) const
{
	const size_t	initialPos	= (size_t)-1 == pos ? length ( ) - 1 : pos;
	if (initialPos >= length ( ))
		throw Exception (UTF8String ("UnicodeString::rfind : position initiale invalide.", charset));
	const WChar_t*	ptr	= unicode ( ) + initialPos;

	size_t	current	= initialPos;
	while (unicode ( ) != ptr)
	{
		if (*ptr == wc)
			return current;

		ptr--;	current--;
	}	// while (unicode ( ) != ptr)

	return (size_t)-1;
}	// UnicodeString::rfind (WChar_t, size_t)


size_t UnicodeString::rfind (char c, size_t pos) const
{
	return rfind ((WChar_t)c, pos);
}	// UnicodeString::rfind (char, size_t)


UnicodeString UnicodeString::substring (size_t first, size_t last) const
{
	if (length ( ) < first)
		throw Exception (UTF8String ("UnicodeString::substring. first hors domaine.", charset));
	if ((size_t)-1 == last)
		last	= length ( );
	if (length ( ) < last)
		throw Exception (UTF8String ("UnicodeString::substring. last hors domaine.", charset));
	if (last < first)
		throw Exception (UTF8String ("UnicodeString::substring. Domaine invalide.", charset));
	const size_t	length	= last - first + 1;
	assert (0 != length);
	WChar_t*	str	= new WChar_t [length + 1];
	CHECK_NULL_PTR_ERROR (str)
	memcpy (str, unicode ( ) + first, length * sizeof (WChar_t));
	str [length]	= 0;

	UnicodeString	substr (str);
	delete [] str;

	return substr;
}	// UnicodeString::substring


UnicodeString& UnicodeString::operator += (const UnicodeString& str)
{
	const UnicodeString	dest	= *this + str;
	copy (dest);

	return *this;
}	// UnicodeString::operator += (const UnicodeString&)


UnicodeString& UnicodeString::operator += (const WChar_t* str)
{
	return operator += (UnicodeString (str));
}	// UnicodeString::operator += (const WChar_t*


UnicodeString& UnicodeString::operator += (const string& str)
{
	return operator += (UnicodeString (str));
}	// UnicodeString::operator += (const string&)


UnicodeString& UnicodeString::operator += (const char* str)
{
	return operator += (UnicodeString (str));
}	// UnicodeString::operator += (const char*)


UnicodeString& UnicodeString::operator += (WChar_t wc)
{
	return operator += (UnicodeString (wc));
}	// UnicodeString::operator += (WChar_t)


UnicodeString& UnicodeString::operator += (char c)
{
	return operator += (UnicodeString (c));
}	// UnicodeString::operator += (char)


UnicodeString& UnicodeString::operator += (short s)
{
	return operator += (UnicodeString (s));
}	// UnicodeString::operator += (short)


UnicodeString& UnicodeString::operator += (long l)
{
	return operator += (UnicodeString (l));
}	// UnicodeString::operator += (long)


UnicodeString& UnicodeString::operator += (unsigned long ul)
{
	return operator += (UnicodeString (ul));
}	// UnicodeString::operator += (unsigned long)


UnicodeString& UnicodeString::operator += (long long l)
{
	return operator += (UnicodeString (l));
}	// UnicodeString::operator += (long long)


UnicodeString& UnicodeString::operator += (unsigned long long ul)
{
	return operator += (UnicodeString (ul));
}	// UnicodeString::operator += (unsigned long long)


UnicodeString& UnicodeString::operator += (float f)
{
	return operator += (UnicodeString (f));
}	// UnicodeString::operator += (float f)


UnicodeString& UnicodeString::operator += (double d)
{
	return operator += (UnicodeString (d));
}	// UnicodeString::operator += (double d)


UnicodeString& UnicodeString::operator += (long double d)
{
	return operator += (UnicodeString (d));
}	// UnicodeString::operator += (long double d)


UnicodeString& UnicodeString::operator << (const UnicodeString& str)
{
	return *this += str;
}	// UnicodeString::operator << (const UnicodeString& str)


UnicodeString& UnicodeString::operator << (const WChar_t* str)
{
	return *this += str;
}	// UnicodeString::operator << (const WChar_t* str)


UnicodeString& UnicodeString::operator << (const string& str)
{
	return *this += str;
}	// UnicodeString::operator << (const string& str)


UnicodeString& UnicodeString::operator << (const char* str)
{
	return *this += str;
}	// UnicodeString::operator << (const char*)


UnicodeString& UnicodeString::operator << (WChar_t wc)
{
	return *this += wc;
}	// UnicodeString::operator << (WChar_t)


UnicodeString& UnicodeString::operator << (char c)
{
	return *this += c;
}	// UnicodeString::operator << (char)


UnicodeString& UnicodeString::operator << (short s)
{
	return *this += NumericConversions::toStr (s, _numericFieldWidth);
}	// UnicodeString::operator << (short)


UnicodeString& UnicodeString::operator << (long l)
{
	return *this += NumericConversions::toStr (l, _numericFieldWidth);
}	// UnicodeString::operator << (long)


UnicodeString& UnicodeString::operator << (unsigned long ul)
{
	return *this += NumericConversions::toStr (ul, _numericFieldWidth);
}	// UnicodeString::operator << (unsigned long)


UnicodeString& UnicodeString::operator << (long long l)
{
	return *this += NumericConversions::toStr (l, _numericFieldWidth);
}	// UnicodeString::operator << (long)


UnicodeString& UnicodeString::operator << (unsigned long long ul)
{
	return *this += NumericConversions::toStr (ul, _numericFieldWidth);
}	// UnicodeString::operator << (unsigned long)


UnicodeString& UnicodeString::operator << (float f)
{
	return *this += NumericConversions::toStr (
						f, _decimalNumber, _numericFieldWidth, _scientific);
}	// UnicodeString::operator << (float)


UnicodeString& UnicodeString::operator << (double d)
{
	return *this += NumericConversions::toStr (
							d, _decimalNumber, _numericFieldWidth, _scientific);
}	// UnicodeString::operator << (double)


UnicodeString& UnicodeString::operator << (long double d)
{
	return *this += NumericConversions::toStr (
							d, _decimalNumber, _numericFieldWidth, _scientific);
}	// UnicodeString::operator << (long double)


UnicodeString UnicodeString::format (size_t columnNum) const
{
	if (0 == columnNum)
		throw Exception (UTF8String ("UnicodeString::format : nombre de colonnes nul.", charset));
	if (columnNum >= length ( ))
		return UnicodeString (*this);

	UnicodeString	formated;
	const WChar_t*	str		= unicode ( );
	const WChar_t*	ptr		= str;
	size_t			lastWs	= 0, pos = 0, lastPos = 0, size = 0;
	bool			hasWs	= false;
	const size_t	l		= length ( );

	while (pos < l)
	{
		if (unicodeWhiteSpace == *ptr)
		{
			lastWs	= pos;
			hasWs	= true;
		}	// if (unicodeWhiteSpace == *ptr)
		if (unicodeLineFeed == *ptr)
		{
			formated	+= substring (lastPos, pos);
			lastPos		= pos + 1;
			size		= 1;
			hasWs		= false;
			pos++;	ptr++;
			continue;
		}	// if (unicodeLineFeed == *ptr)

		if (size + 1 >= columnNum)	// => necessité de couper
		{
			if (false == hasWs)
				lastWs	= lastPos + columnNum - 1;
			formated	+= substring (lastPos, lastWs);
			formated	+= unicodeLineFeed;
			lastPos		= lastWs + 1;
			size		= pos - lastWs;
			hasWs		= false;
		}	// if (size + 1 == columnNum)

		if (lastPos + columnNum >= l)
		{
			formated	+= substring (lastPos);
			break;
		}	// if (lastPos + columnNum == l)

		pos++;	ptr++; size++;
	}	// while (lastWs < l)

	return formated;
}	// UnicodeString::format


UnicodeString UnicodeString::trim (bool removeTabs) const
{
	const WChar_t*	str		= unicode ( );
	const WChar_t*	ptr		= str;
	bool			hasWs	= false;
	const size_t	l		= length ( );
	size_t			pos = 0, first = 0, last = l - 1;

	for (pos = 0, ptr = str; pos < l; pos++, ptr++)
	{
		if (true == removeTabs)
		{
			if ((unicodeWhiteSpace == *ptr) ||
			    (unicodeLineFeed   == *ptr) ||
			    (9 == *ptr))	// tab, cf. Qt
			{
				hasWs	= true;
				continue;
			}
		}
		else if ((unicodeWhiteSpace == *ptr) || (unicodeLineFeed   == *ptr))
		{
			hasWs	= true;
			continue;
		}

		first	= pos;
		break;
	}	// for (pos = 0, ptr = str; pos < l; pos++, ptr++)
	if (l == pos)		// Que des séparateurs
		return "";

	for (pos = l - 1, ptr = str + l - 1; pos != (size_t)-1; pos--, ptr--)
	{
		if (true == removeTabs)
		{
			if ((unicodeWhiteSpace == *ptr) ||
			    (unicodeLineFeed   == *ptr) ||
			    (9 == *ptr))	// tab, cf. Qt
			{
				hasWs	= true;
				continue;
			}
		}
		else if ((unicodeWhiteSpace == *ptr) || (unicodeLineFeed   == *ptr))
		{
			hasWs	= true;
			continue;
		}

		last	= pos;
		break;
	}	// for (pos = l - 1, ptr = str + l - 1; pos != (size_t)-1; pos--, ptr--)

	if (false == hasWs)
		return *this;

	if (first <= last)
		return substring (first, last);

	return "";
}	// UnicodeString::trim



void UnicodeString::copy (const UnicodeString& str)
{
	if (&str == this)
		return;
	clear ( );
	_length	= str.length ( );
	_string	= new WChar_t [_length + 1];
	CHECK_NULL_PTR_ERROR (_string)
	if (0 != unicode ( ))	// v 6.14.1
		memcpy (_string, str.unicode ( ), _length * sizeof (WChar_t));
	_string [_length]	= 0;
}	// UnicodeString::copy


void UnicodeString::copy (const WChar_t* str)
{
	if (_string == str)
		return;

	clear ( );
	_length	= stringLength (str);
	_string	= new WChar_t [_length + 1];
	CHECK_NULL_PTR_ERROR (_string)
	if (0 != unicode ( ))	// v 6.14.1
		memcpy (_string, str, _length * sizeof (WChar_t));
	_string [_length]	= 0;
}	// UnicodeString::copy


void UnicodeString::copy (const string& str)
{
	clear ( );
	_length	= str.length ( );
	_string	= new WChar_t [_length + 1];
	CHECK_NULL_PTR_ERROR (_string)
	for (size_t i = 0; i < _length; i++)
//		_string [i]	= (WChar_t)str [i];
		_string [i] = (unsigned char)str [i];
	_string [_length]	= 0;
}	// UnicodeString::copy


size_t UnicodeString::stringLength (const WChar_t* str)
{
	size_t			length	= 0;
	const WChar_t*	ptr		= str;

	while (0 != *ptr)
	{
		ptr++;
		length++;
	}

	return length;
}	// UnicodeString::stringLength



UnicodeString operator + (const UnicodeString& us1, const UnicodeString& us2)
{	
	const size_t	length	= us1.length ( ) + us2.length ( );
	WChar_t*		str		= new WChar_t [length + 1];
	CHECK_NULL_PTR_ERROR (str)
	if (0 != us1.unicode ( ))	// v 6.14.1
		memcpy (str, us1.unicode ( ), us1.length ( ) * sizeof (WChar_t));
	if (0 != us2.unicode ( ))	// v 6.14.1
		memcpy (str + us1.length ( ), us2.unicode ( ), us2.length ( ) * sizeof (WChar_t));
	str [length]	= 0;

	UnicodeString	us (str);
	delete [] str;

	return us;
}	// operator + (const UnicodeString& us1, const UnicodeString& us2)


UnicodeString operator + (const UnicodeString& us, const WChar_t* wc)
{
	return us + UnicodeString (wc);
}	// operator + (const UnicodeString&, const WChar_t*)


UnicodeString operator + (const UnicodeString& us, const string str)
{
	return us + UnicodeString (str);
}	// operator + (const UnicodeString&, const string&)


UnicodeString operator + (const UnicodeString& us, const char* str)
{
	return us + UnicodeString (str);
}	// operator + (const UnicodeString&, const char*)


UnicodeString operator + (const UnicodeString& us, WChar_t wc)
{
	return us + UnicodeString (wc);
}	// operator + (const UnicodeString&, WChar_t)


UnicodeString operator + (const UnicodeString& us, char c)
{
	return us + UnicodeString (c);
}	// operator + (const UnicodeString&, char)


UnicodeString operator + (const UnicodeString& us, short s)
{
	return us + UnicodeString (s);
}	// operator + (const UnicodeString&, short)


UnicodeString operator + (const UnicodeString& us, long l)
{
	return us + UnicodeString (l);
}	// operator + (const UnicodeString&, long)


UnicodeString operator + (const UnicodeString& us, unsigned long ul)
{
	return us + UnicodeString (ul);
}	// operator + (const UnicodeString&, unsigned long)


UnicodeString operator + (const UnicodeString& us, long long l)
{
	return us + UnicodeString (l);
}	// operator + (const UnicodeString&, long)


UnicodeString operator + (const UnicodeString& us, unsigned long long ul)
{
	return us + UnicodeString (ul);
}	// operator + (const UnicodeString&, unsigned long)


UnicodeString operator + (const UnicodeString& us, float f)
{
	return us + UnicodeString (f);
}	// operator + (const UnicodeString&, float)


UnicodeString operator + (const UnicodeString& us, double d)
{
	return us + UnicodeString (d);
}	// operator + (const UnicodeString&, double)


UnicodeString operator + (const UnicodeString& us, long double d)
{
	return us + UnicodeString (d);
}	// operator + (const UnicodeString&, long double)


UnicodeString& operator << (UnicodeString& ustr, setw width)
{
	ustr._numericFieldWidth	= width.width ( );
	return ustr;
}	// operator << (UnicodeString&, setw)


UnicodeString& operator << (UnicodeString& ustr, setprecision prec)
{
	ustr._decimalNumber	= prec.precision ( );
	return ustr;
}	// operator << (UnicodeString&, setprecision)


UnicodeString& operator << (UnicodeString& ustr, ios_base::fmtflags flags)
{
	if (flags == ios_base::scientific)
		ustr._scientific	= true;
	else if (flags == ios_base::fixed)
		ustr._scientific	= false;

	return ustr;
}	// operator << (UnicodeString&, ios_base::fmtflags)


ostream& operator << (ostream& os, const UnicodeString& str)
{
	if (0 != str.length ( ))
		os << str.iso ( );

	return os;
}	// operator << (ostream& os, const UnicodeString& str)


END_NAMESPACE_UTIL
