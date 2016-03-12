#ifndef RNG_H
#define RNG_H
#pragma once

#include <random>

class RNG
{
public:
	RNG();

	int getRandomInt( const int min, const int max );

private:
	std::mt19937 m_Twister;
};

#endif