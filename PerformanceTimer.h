#ifndef PERFTIMER_H
#define PERFTIMER_H
#pragma once

class PerformanceTimer final
{
public:
	PerformanceTimer();

	void startCounter();
	double getCounter();

private:
	double m_dPCFreq;
	__int64 m_iCounterStart;
};

#endif