#include "TkUtil/ScriptingLog.h"
#include "TkUtil/Exception.h"
#include "TkUtil/MachineData.h"
#include "TkUtil/NumericConversions.h"


USING_STD
USING_UTIL


BEGIN_NAMESPACE_UTIL

static const Charset	charset ("àéèùô");


ScriptingLog::ScriptingLog (const string& name, const string& method, const string& result,	const UTF8String& comment)
	: DefaultLog (ScriptingLog::SCRIPTING, comment), _formated (false),
	  _name (name), _methodName (method), _result (result), _tags ( ), _comment (comment), _arguments ( ), _text ( ), _namedObject (0)
{
}	// ScriptingLog::ScriptingLog ( )


ScriptingLog::ScriptingLog (const ReferencedNamedObject& object, const string& method, const string& result, const UTF8String& comment)
	: DefaultLog (ScriptingLog::SCRIPTING, comment), _formated (false),
	  _name (object.getUniqueName ( )), _methodName (method), _result (result), _tags ( ), _comment (comment), _arguments ( ), _text ( ), _namedObject (&object)
{
}	// ScriptingLog::ScriptingLog ( )


ScriptingLog::ScriptingLog (const string& instruction, const UTF8String& comment)
	: DefaultLog (ScriptingLog::SCRIPTING, UTF8String (charset)),
	  _formated (true), _name ( ), _methodName ( ), _result ( ), _tags ( ), _comment (comment), _arguments ( ), _text (instruction, charset), _namedObject (0)
{
}	// ScriptingLog::ScriptingLog


ScriptingLog::ScriptingLog (const ScriptingLog& log)
	: DefaultLog (log), _formated (log._formated),
	  _name (log._name), _methodName (log._methodName),
	  _result (log._result), _tags (log._tags), _comment (log._comment),
	  _arguments (log._arguments), _text (log._text),
	  _namedObject (log._namedObject)
{
}	// ScriptingLog::ScriptingLog (const ScriptingLog&)


ScriptingLog& ScriptingLog::operator = (const ScriptingLog& log)
{
	if (&log != this)
	{
		DefaultLog::operator = (log);
		_formated				= log._formated;
		_name					= log._name;
		_methodName				= log._methodName;
		_result					= log._result;
		_tags					= log._tags;
		_comment				= log._comment;
		_arguments				= log._arguments;
		_text					= log._text;
		_namedObject			= log._namedObject;
	}	// if (&log != this)

	return *this;
}	// ScriptingLog::operator =


ScriptingLog::~ScriptingLog ( )
{
}	// ScriptingLog::~ScriptingLog


Log* ScriptingLog::clone ( ) const
{
	return new ScriptingLog (*this);
}	// ScriptingLog::clone


const UTF8String& ScriptingLog::getText ( ) const
{
	if (true == _formated)
		return _text;

	_text.clear ( );
	if (true == hasResult ( ))
		_text << getResult ( ) << " = ";
	_text << getName ( ) << "." << getMethodName ( ) << "(";

	for (size_t i = 0; i < getMethodArgumentsCount ( ); i++)
	{
		pair < ARG_TYPE, string >	arg		= getMethodArgument (i);
		string						argType ("unknown type");
		switch (arg.first)
		{
			case STRING		: argType	= "string";	break;
			case BOOL		: argType	= "bool";	break;
			case LONG		: argType	= "long";	break;
			case DOUBLE		: argType	= "double";	break;
			case INSTANCE	: argType	= "instance";	break;
		}	// switch (arg.first)

		_text << argType << ' ' << arg.second;
		if (i != getMethodArgumentsCount ( ) - 1)
			_text << ", ";
	}	// for (size_t i = 0; i < getMethodArgumentsCount ( ); i++)

	_text << "). Commentaires : " << getComment ( );

	return _text;
}	// ScriptingLog::getText


void ScriptingLog::setName (const string& name)
{
	_name	= name;
}	// ScriptingLog::setName


void ScriptingLog::setMethodName (const string& method)
{
	_methodName	= method;
}	// ScriptingLog::setMethodName


bool ScriptingLog::hasResult ( ) const
{
	return 0 == getResult ( ).length ( ) ? false : true;
}	// ScriptingLog::hasResult


void ScriptingLog::setResult (const string& result)
{
	_result	= result;
}	// ScriptingLog::setResult


void ScriptingLog::addTag (const string& name, const string& value)
{
	_tags.push_back (TaggedValue (name, value));
}	// ScriptingLog::addTag


void ScriptingLog::addTag (const TaggedValue& taggedValue)
{
	_tags.push_back (taggedValue);
}	// ScriptingLog::addTag


void ScriptingLog::setComment (const UTF8String& comment)
{
	_comment	= comment;
}	// ScriptingLog::setComment


pair < ScriptingLog::ARG_TYPE, string > ScriptingLog::getMethodArgument (size_t i) const
{
	if (_arguments.size ( ) < i)
	{
		UTF8String	msg (charset);
		msg << "ScriptingLog::getMethodArgument. Argument invalide ("
		    << (unsigned long)i << "). La méthode " << getMethodName ( )
		    << " n'a que " << (unsigned long)_arguments.size ( ) << " arguments.";
		throw Exception (msg);
	}	// if (_arguments.size ( ) < i)

	return _arguments [i];
}	// ScriptingLog::getMethodArgument


void ScriptingLog::addMethodArgument (const string& arg)
{
	_arguments.push_back (pair < ARG_TYPE, string > (STRING, arg));
}	// ScriptingLog::addMethodArgument (const string&)


ScriptingLog& ScriptingLog::operator << (const string& arg)
{
	addMethodArgument (arg);
	return *this;
}	// ScriptingLog::operator << (const string&)


void ScriptingLog::addMethodArgument (const char* arg)
{
	_arguments.push_back (pair < ARG_TYPE, string > (STRING, arg));
}	// ScriptingLog::addMethodArgument (const char*)


ScriptingLog& ScriptingLog::operator << (const char* arg)
{
	addMethodArgument (arg);
	return *this;
}	// ScriptingLog::operator << (const char*)


void ScriptingLog::addMethodArgument (bool arg)
{
	_arguments.push_back (pair < ARG_TYPE, string > (BOOL, true == arg ? "true" : "false"));
}	// ScriptingLog::addMethodArgument (bool)


ScriptingLog& ScriptingLog::operator << (bool arg)
{
	addMethodArgument (arg);
	return *this;
}	// ScriptingLog::operator << (bool)


void ScriptingLog::addMethodArgument (long arg)
{
	_arguments.push_back (pair < ARG_TYPE, string > (LONG, NumericConversions::toStr (arg)));
}	// ScriptingLog::addMethodArgument (long)


ScriptingLog& ScriptingLog::operator << (long arg)
{
	addMethodArgument (arg);
	return *this;
}	// ScriptingLog::operator << (long)


ScriptingLog& ScriptingLog::operator << (const ReferencedNamedObject* object)
{
	addMethodArgument (object);
	return *this;
}	// operator << (const ReferencedNamedObject*)


void ScriptingLog::addMethodArgument (double arg)
{
	// - 1 car precision indique le nombre de décimales en mode scientifique,
	// et qu'il y aura un chiffre dans les unités.
	_arguments.push_back (pair < ARG_TYPE, string > (
//			DOUBLE, NumericConversions::toStr (
//					arg, MachineData::doubleSignificantNumeral ( ) - 1, true)));
			DOUBLE, NumericConversions::userRepresentation (	// v 2.33.0
					arg, MachineData::doubleSignificantNumeral ( ) - 1)));
}	// ScriptingLog::addMethodArgument (double)


ScriptingLog& ScriptingLog::operator << (double arg)
{
	addMethodArgument (arg);
	return *this;
}	// ScriptingLog::operator << (double)


void ScriptingLog::addVariable (const string& name)
{
	_arguments.push_back (pair < ARG_TYPE, string > (INSTANCE, name));
}	// ScriptingLog::addVariable


void ScriptingLog::addMethodArgument (const ReferencedNamedObject* object)
{
	const string	name	= 0 == object ? "" : object->getUniqueName ( );
	_arguments.push_back (pair < ARG_TYPE, string > (INSTANCE, name));
}	// ScriptingLog::addMethodArgument (const ReferencedNamedObject&)


void ScriptingLog::addMethodArgument (ScriptingLog::ARG_TYPE type, const string& value)		// v 6.4.0
{
	_arguments.push_back (pair < ARG_TYPE, string > (type, value));
}	// ScriptingLog::addMethodArgument

END_NAMESPACE_UTIL
