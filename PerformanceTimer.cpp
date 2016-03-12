#include "PerformanceTimer.h"
#include "Console.h"

PerformanceTimer::PerformanceTimer()
	:
m_dPCFreq( 0.0 ),
m_iCounterStart( 0 )
{}
void PerformanceTimer::startCounter()
{
	LARGE_INTEGER li;
	if( !QueryPerformanceFrequency( &li ) )
	{
		logError( "QueryPerformanceFrequency failed!\n", getLastErrorAsString() );
		return;
	}

	m_dPCFreq = static_cast<double>( li.QuadPart ) / 1000.0;
	QueryPerformanceCounter( &li );
	m_iCounterStart = li.QuadPart;
}
double PerformanceTimer::getCounter()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter( &li );
	return static_cast<double>( li.QuadPart - m_iCounterStart ) / m_dPCFreq;
}