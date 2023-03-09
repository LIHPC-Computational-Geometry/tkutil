#include "TkUtil/Date.h"
#include "TkUtil/Exception.h"

#include <iostream>


USING_STD
USING_UTIL


static void display (const Date& date);


int main (int argc, char* argv [])
{
	try
	{
		Date	current;
		display (current);
	}
	catch (const Exception& exc)
	{
		cout << "Exception caught : " << exc.getFullMessage ( ) << endl;
		return -1;
	}
	catch (const exception& e)
	{
		cout << "Standard exception caught : " << e.what ( ) << endl;
		return -1;
	}
	catch (...)
	{
		cout << "Undocumented exception caught." << endl;
		return -1;
	}		

	return 0;
}	// main


static void display (const Date& date)
{
	cout << "Nous sommes le " << date.getDate ( ) << "." << endl
	     << "Il est " << date.getTime ( ) << endl;
}	// display
