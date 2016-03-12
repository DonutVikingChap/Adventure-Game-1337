#include "Sound.h"
#include "Console.h"
#pragma comment (lib , "winmm.lib")

#include <vector>

Sound::Sound()
	:
m_DeviceID( 0 )
{}
Sound::Sound( const LPCWSTR soundFile, const LPCWSTR audioDevice )
	:
m_DeviceID( 0 )
{
	loadSound( soundFile, audioDevice );
}
bool Sound::loadSound( const LPCWSTR soundFile, const LPCWSTR audioDevice )
{
	MCI_OPEN_PARMS openParms;
	openParms.lpstrDeviceType = audioDevice;
	openParms.lpstrElementName = soundFile;

	auto result = mciSendCommand( NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)(LPVOID)&openParms );
	if( result != 0 )
		logError( "Failed to open audio device!", std::to_string( result ) );
	else
		m_DeviceID = openParms.wDeviceID;
	return result == 0;
}
bool Sound::playSound()
{
	auto result = mciSendCommand( m_DeviceID, MCI_PLAY, 0, NULL );
	if( result != 0 )
	{
		logError( "Failed to play sound!", std::to_string( result ) );
		mciSendCommand( m_DeviceID, MCI_CLOSE, 0, NULL );
	}
	return result == 0;
}
void Sound::stopSound()
{
	mciSendCommand( m_DeviceID, MCI_CLOSE, 0, NULL );
}
void Sound::loopSound( const LPCWSTR soundFile )
{
	PlaySound( soundFile, nullptr, SND_FILENAME | SND_LOOP | SND_ASYNC );
}
void Sound::stopLoop()
{
	PlaySound( NULL, 0, 0 );
}