#ifndef CONSOLE_H
#define CONSOLE_H
#pragma once

#include <windows.h>
#include <string>

std::string getLastErrorAsString();
void logError( const std::string& msg, const std::string& error = "", const bool fatal = false );

class Console final
{
public:
	Console( const std::string& title, const short width, const short height, const WORD attributes );

	void setCursorPosition( const short x, const short y );
	void clearScreen();
	void print( const char ch );
	void print( const std::string& str );
	void printSlow( const std::string& str, const int delay, const bool allowSkip );
	void getEnter();

private:
	HANDLE m_Handle;
};

#endif