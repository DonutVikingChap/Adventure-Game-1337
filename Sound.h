#ifndef SOUND_H
#define SOUND_H
#pragma once

#include <windows.h>

class Sound
{
public:
	Sound();
	Sound( const LPCWSTR soundFile, const LPCWSTR audioDevice );

	bool loadSound( const LPCWSTR soundFile, const LPCWSTR audioDevice );
	bool playSound();
	void stopSound();
	void loopSound( const LPCWSTR soundFile );
	void stopLoop();

private:
	UINT m_DeviceID; // A device ID so we can keep track of it once we open it.
};

#endif