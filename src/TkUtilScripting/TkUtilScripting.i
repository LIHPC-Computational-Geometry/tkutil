//
// Binding Python/Swig pour accéder aux services de ce module
//


//%module TkUtil
%{
#include "TkUtil/ReferencedObject.h"
#include "TkUtil/UTF8String.h"
#include "TkUtil/Version.h"
using namespace TkUtil;
%}  // module TkUtil

// v 6.14.2 : possible usage de :
// s = UTF8String ("Une chaîne de caractères", Charset.UTF_8)
// print (s.utf8 ( ))
%include "std_string.i"	// v 6.14.2

%include TkUtil/UTF8String.h
%include TkUtil/ReferencedObject.h
%include TkUtil/Version.h

