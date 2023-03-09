#include "TkUtil/Timer.h"
#include "TkUtil/Exception.h"

#include <iostream>
#include <math.h>


USING_STD
USING_UTIL


int main (int argc, char* argv [])
{
	try
	{
		Timer	timer;

		timer.start ( );
		cout << "Timer started ..." << endl;

		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 1000000; j++)
				sin ((double)j);

			timer.stop ( );
			cout << "Elapsed time : " << timer.duration ( ) << " sec "
				<< "micro-time : " << timer.microduration( ) << " microsec" << endl
				<< "CPU time : " << timer.cpuDuration ( ) << endl;
		}	// for (int i = 0; i < 10; i++)
	}
	catch (const Exception& exc)
	{
		cout << "Exception levée : " << exc.getFullMessage ( ) << endl;
		return -1;
	}
	catch (...)
	{
		cout << "Exception innattendue." << endl;
		return -1;
	}

	return 0;
}	// main
