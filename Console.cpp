#include "Console.h"
#include "Utility.h"

#include <iostream>
#include <conio.h>

std::string getLastErrorAsString()
{
	// Get the error message, if any.
	auto errorMessageID = ::GetLastError();
	if( errorMessageID == 0 )
		return std::string(); // No error message has been recorded.

	auto messageBuffer = LPSTR( nullptr );
	auto size = static_cast<size_t>( FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, errorMessageID, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPSTR)&messageBuffer, 0, nullptr ) );

	auto message = std::string( messageBuffer, size );

	//Free the buffer.
	LocalFree( messageBuffer );

	return message;
}
void logError( const std::string& msg, const std::string& error, const bool fatal )
{
	const auto errorStr = "ERROR: " + msg + ( error.empty() ? "" : " | Error: " + error );
	std::cerr << errorStr << '\n';

	auto errorStr2 = "ERROR: " + msg;
	if( !error.empty() )
		errorStr2.append( "\nError: " + error );
	if( fatal )
		errorStr2.append( "\n(FATAL ERROR! Game will exit...)" );
	MessageBox( nullptr, stringToWideString( errorStr2 ).c_str(), L"Error!", MB_ICONERROR | MB_OK | MB_DEFBUTTON1 );
	if( fatal )
		exit( 1 );
}

Console::Console( const std::string& title, const short width, const short height, const WORD attributes )
	:
m_Handle( GetStdHandle( STD_OUTPUT_HANDLE ) )
{
	if( !SetConsoleTitle( stringToWideString( title ).c_str() ) )
	{
		auto err = getLastErrorAsString();
		logError( "SetConsoleTitle failed!\n", err );
	}

	if( !SetConsoleScreenBufferSize( m_Handle, COORD{ width, height } ) )
	{
		auto err = getLastErrorAsString();
		logError( "SetConsoleScreenBufferSize failed!\n", err );
	}

	auto windowRect = SMALL_RECT{ 0, 0, width - 1, height - 1 };
	if( !SetConsoleWindowInfo( m_Handle, TRUE, &windowRect ) )
	{
		auto err = getLastErrorAsString();
		logError( "SetConsoleWindowInfo failed!\n", err );
	}

	if( !SetConsoleTextAttribute( m_Handle, attributes ) )
	{
		auto err = getLastErrorAsString();
		logError( "SetConsoleTextAttribute failed!\n", err );
	}
}
void Console::setCursorPosition( const short x, const short y )
{
	if( !SetConsoleCursorPosition( m_Handle, COORD{ x, y } ) )
	{
		auto err = getLastErrorAsString();
		logError( "SetConsoleCursorPosition failed!\n", err );
	}
}
void Console::clearScreen()
{
	// Get the number of cells in the current buffer.
	CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
	if( !GetConsoleScreenBufferInfo( m_Handle, &bufferInfo ) )
	{
		auto err = getLastErrorAsString();
		logError( "GetConsoleScreenBufferInfo failed!\n", err );
		return;
	}
	auto cellCount = static_cast<DWORD>( bufferInfo.dwSize.X * bufferInfo.dwSize.Y );

	// Fill the entire buffer with spaces.
	auto homeCoords = COORD{ 0, 0 };
	DWORD count;
	if( !FillConsoleOutputCharacter( m_Handle, (TCHAR)' ', cellCount, homeCoords, &count ) )
	{
		auto err = getLastErrorAsString();
		logError( "FillConsoleOutputCharacter failed!\n", err );
		return;
	}

	// Fill the entire buffer with the current colors and attributes.
	if( !FillConsoleOutputAttribute( m_Handle, bufferInfo.wAttributes, cellCount, homeCoords, &count ) )
	{
		auto err = getLastErrorAsString();
		logError( "FillConsoleOutputAttribute failed!\n", err );
		return;
	}

	// Move the cursor home.
	if( !SetConsoleCursorPosition( m_Handle, homeCoords ) )
	{
		auto err = getLastErrorAsString();
		logError( "SetConsoleCursorPosition failed!\n", err );
	}
}
void Console::print( const char ch )
{
	std::cout << ch;
}
void Console::print( const std::string& str )
{
	std::cout << str;
}
void Console::printSlow( const std::string& str, const int delay, const bool allowSkip )
{
	auto restText = str;
	auto done = false;
	for( auto i = decltype( str.size() ){ 0 }; i < str.size() && done == false; ++i )
	{
		print( str[ i ] );
		restText.erase( restText.begin() );

		Sleep( static_cast<DWORD>( delay ) );

		if( _kbhit() && allowSkip == true && _getch() == ' ' )
		{
			print( restText );
			done = true;
		}
	}
}
void Console::getEnter()
{
	print( "Press ENTER to continue...\n\n" );
	while( _getch() != '\r' );
}