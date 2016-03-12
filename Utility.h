#ifndef UTILITY_H
#define UTILITY_H
#pragma once

#include <windows.h>
#include <string>
#include <algorithm>
#include <vector>

std::string inline toLowerCase( const std::string& str )
{
	auto temp = str;
	std::transform( temp.begin(), temp.end(), temp.begin(), ::tolower );
	return temp;
}
char inline toLowerCase( const char ch )
{
	return tolower( ch );
}
int inline toLowerCase( const int i )
{
	return tolower( i );
}
std::wstring inline stringToWideString( const std::string& str )
{
	return std::wstring( str.begin(), str.end() );
}
std::wstring inline getExePath()
{
	std::vector<wchar_t> pathBuf;
	DWORD copied = 0;
	do
	{
		pathBuf.resize( pathBuf.size() + MAX_PATH );
		copied = GetModuleFileName( 0, &pathBuf.at( 0 ), pathBuf.size() );
	}
	while( copied >= pathBuf.size() );
	pathBuf.resize( copied );
	std::wstring path( pathBuf.begin(), pathBuf.end() );
	path.erase( path.rfind( '\\' ) + 1, std::wstring::npos );
	return path;
}

#endif