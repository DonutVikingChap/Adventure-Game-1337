#include "RNG.h"

#include <chrono>

RNG::RNG()
	:
m_Twister( static_cast<unsigned>( std::chrono::high_resolution_clock::now().time_since_epoch().count() ) )
{}
int RNG::getRandomInt( const int min, const int max )
{
	std::uniform_int_distribution<int> intDist( min, max );
	return intDist( m_Twister );
}