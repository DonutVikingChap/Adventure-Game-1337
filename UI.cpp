#include "UI.h"
#include "Utility.h"
#include "Console.h"
#include "Player.h"
#include "LocationData.h"
#include "Sound.h"
#include "GameVersion.h"

#include <conio.h>
#include <iostream>
#include <fstream>

int getInputInt( const bool eraseWhenDone, const short line, Console& console )
{
	console.setCursorPosition( 0, line );

	std::string inputBuffer;
	for( auto ch = _getch(); ch != '\r'; ch = _getch() )
	{
		if( inputBuffer.size() <= 8 )
		{
			_putch( ch );
			if( ch == 0 || ch == 0xE0 )
				_getch();
			else if( isdigit( ch ) )
				inputBuffer.push_back( ch );
			else if( ch == '\b' )
			{
				_putch( ' ' );
				_putch( '\b' );
				if( !inputBuffer.empty() )
					inputBuffer.pop_back();
			}
			else
			{
				_putch( '\b' );
				_putch( ' ' );
				_putch( '\b' );
			}
		}
		else if( ch == '\b' )
		{
			_putch( '\b' );
			_putch( ' ' );
			_putch( '\b' );
			if( !inputBuffer.empty() )
				inputBuffer.pop_back();
		}
	}

	if( eraseWhenDone )
	{
		console.setCursorPosition( 0, line );
		for( auto i = decltype( inputBuffer.size() ){ 0 }; i < inputBuffer.size(); ++i )
			_putch( ' ' );
		console.setCursorPosition( 0, line );
	}

	return std::stoi( inputBuffer );
}
bool getInputUntilIntRange( int& result, const int min, const int max, const bool retry, const bool eraseWhenDone, const short line, Console& console )
{
	console.setCursorPosition( 0, line );

	auto erase = [line]( const int inputIntBuffer, Console& console )
	{
		console.setCursorPosition( 0, line );
		const auto size = std::to_string( inputIntBuffer ).size();
		for( auto i = decltype( size ){ 0 }; i < size; ++i )
			_putch( ' ' );
		console.setCursorPosition( 0, line );
	};

	int inputIntBuffer = 0;
	auto found = false;
	for( auto done = false; !done; )
	{
		inputIntBuffer = getInputInt( false, line, console );
		found = ( inputIntBuffer >= min && inputIntBuffer <= max );

		if( found || retry == false )
			done = true;
		else
			erase( inputIntBuffer, console );
	}

	if( eraseWhenDone )
		erase( inputIntBuffer, console );

	if( found )
	{
		result = inputIntBuffer;
		return true;
	}
	else
		return false;
}
std::string getInputString( const int maxLen, const bool makeLower, const bool allowNumbers, const bool allowSpecialChars, const bool eraseWhenDone, const short line, Console& console )
{
	console.setCursorPosition( 0, line );

	std::string inputBuffer;
	for( auto ch = _getch(); ch != '\r'; ch = _getch() )
	{
		if( maxLen <= 0 || static_cast<int>( inputBuffer.size() ) < maxLen )
		{
			_putch( ch );
			if( ch == 0 || ch == 0xE0 )
				_getch();
			else if( ch == '\b' )
			{
				_putch( ' ' );
				_putch( '\b' );
				if( !inputBuffer.empty() )
					inputBuffer.pop_back();
			}
			else if( ch == ' ' || isalpha( ch ) )
				inputBuffer.push_back( ch );
			else if( isdigit( ch ) && allowNumbers )
				inputBuffer.push_back( ch );
			else if( allowSpecialChars )
				inputBuffer.push_back( ch );
			else
			{
				_putch( '\b' );
				_putch( ' ' );
				_putch( '\b' );
			}
		}
		else if( ch == '\b' )
		{
			_putch( '\b' );
			_putch( ' ' );
			_putch( '\b' );
			if( !inputBuffer.empty() )
				inputBuffer.pop_back();
		}
	}

	if( eraseWhenDone )
	{
		console.setCursorPosition( 0, line );
		for( const auto& ch : inputBuffer )
			_putch( ' ' );
		console.setCursorPosition( 0, line );
	}
	if( makeLower )
		inputBuffer = toLowerCase( inputBuffer );

	return inputBuffer;
}
bool getInputUntilStrings( std::string& result, const std::unordered_set<std::string>& allowedStrings, const int maxLen, const bool makeLower, const bool allowNumbers, const bool allowSpecialChars, const bool retry, const bool eraseWhenDone, const short line, Console& console )
{
	console.setCursorPosition( 0, line );

	auto erase = [line]( const std::string& inputBuffer, Console& console )
	{
		console.setCursorPosition( 0, line );
		for( const auto& ch : inputBuffer )
			_putch( ' ' );
		console.setCursorPosition( 0, line );
	};

	std::string inputBuffer;
	auto found = false;
	for( auto done = false; !done; )
	{
		inputBuffer = getInputString( maxLen, makeLower, allowNumbers, allowSpecialChars, false, line, console );
		found = allowedStrings.count( inputBuffer ) != 0;
		if( found || !retry )
			done = true;
		else
			erase( inputBuffer, console );
	}

	if( eraseWhenDone )
		erase( inputBuffer, console );

	if( found )
	{
		result = inputBuffer;
		return true;
	}
	else
		return false;
}

std::unordered_set<std::string> getStandardCommands()
{
	auto result = std::unordered_set<std::string>{ "help", "info", "location", "inventory", "stats", "money", "save", "time" };
	if( _GAMEVERSION == "debug" )
	{
		result.insert( "debugbattle" );
		result.insert( "debugshop" );
	}
	return result;
}
std::unordered_set<std::string> getSynonyms( const std::string& word )
{
	if( word == "help" )
		return { "commands" };
	else if( word == "info" )
		return { "description", "more info" };
	else if( word == "money" )
		return { "balance", "cash", "dollars" };
	else if( word == "stats" )
		return { "character" };
	else if( word == "inventory" )
		return { "items", "backpack", "inv", "open inventory", "open backpack" };
	else if( word == "location" )
		return { "loc" };
	else
	{
		std::unordered_set<std::string> result;
		std::ifstream file( "game/data/synonyms.txt" );
		if( file.is_open() )
		{
			std::string buffer;
			while( std::getline( file, buffer ) )
			{
				if( buffer.find( "#" + word ) != std::string::npos )
				{
					for( auto loop = true; loop && std::getline( file, buffer ); )
					{
						if( !buffer.empty() )
						{
							if( buffer.front() == '*' )
								loop = false;
							else
								result.insert( buffer );
						}
					}
				}
			}
			file.close();
		}
		else
			logError( "Unable to open synonym data file!" );

		return result;
	}
}

bool mainMenu( Console& console, Player& player, RNG& rng )
{
	Sound introMusic( L"introtheme.wav", L"waveaudio" );
	introMusic.playSound();

	console.clearScreen();
	std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << '\n'; Sleep( 100 );
	std::cout << "                  _____       .___                    __                           ________"                                     << '\n'; Sleep( 100 );
	std::cout << "                 /  _  \\    __| _/__  __ ____   _____/  |_ __ _________   ____    /  _____/_____    _____   ____"               << '\n'; Sleep( 100 );
	std::cout << "                /  /_\\  \\  / __ |\\  \\/ // __ \\ /    \\   __\\  |  \\_  __ \\_/ __ \\  /   \\  ___\\__  \\  /     \\_/ __ \\"<< '\n'; Sleep( 100 );
	std::cout << "               /    |    \\/ /_/ | \\   /\\  ___/|   |  \\  | |  |  /|  | \\/\\  ___/  \\    \\_\\  \\/ __ \\|  Y Y  \\  ___/"   << '\n'; Sleep( 100 );
	std::cout << "               \\____|__  /\\____ |  \\_/  \\___  >___|  /__| |____/ |__|    \\___  >  \\______  (____  /__|_|  /\\___  >"       << '\n'; Sleep( 100 );
	std::cout << "                       \\/      \\/           \\/     \\/                        \\/          \\/     \\/      \\/     \\/"      << '\n'; Sleep( 100 );
	std::cout << "                                                ____________ _________________"                                                  << '\n'; Sleep( 100 );
	std::cout << "                                               /_   \\_____  \\\\_____  \\______  \\"                                            << '\n'; Sleep( 100 );
	std::cout << "                                                |   | _(__  <  _(__  <   /    /"                                                 << '\n'; Sleep( 100 );
	std::cout << "                                                |   |/       \\/       \\ /    /"                                                << '\n'; Sleep( 100 );
	std::cout << "                                                |___/______  /______  //____/"                                                   << '\n'; Sleep( 100 );
	std::cout << "                                                           \\/       \\/"                                                        << '\n'; Sleep( 100 );
	std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << '\n'; Sleep( 250 );
	std::cout                                                                                                                                      << '\n';
	std::cout << "                                                   by Donut the Vikingchap"                                                      << '\n';
	std::cout << "                                                          (c) 2016"                                                              << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "                                                   Version: " << _GAMEVERSION                                                    << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n'; Sleep( 500 );
	std::cout << "                                               Welcome to ADVENTURE GAME 1337!"                                                  << '\n';
	std::cout                                                                                                                                      << '\n'; Sleep( 500 );
	std::cout << "                    HOW TO PLAY:"                                                                                                << '\n';
	std::cout << "                    Type what you wish to do and press ENTER to confirm after the text has been presented."                      << '\n';
	std::cout << "                    Your options are sometimes presented to you, but most of them are not. Use the"                              << '\n';
	std::cout << "                    description of your environment for hints on what commands are possible."                                    << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "                    Type \"help\" in-game for a list of standard commands."                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n'; Sleep( 500 );
	std::cout << "Type \"new game\" to continue to character creation."                                                                            << '\n';
	std::cout << "Type \"load game\" to load a previously saved game."                                                                             << '\n';
	std::cout << "Type \"exit\" to quit the game."                                                                                                 << '\n';
	std::cout                                                                                                                                      << '\n';

	std::unordered_set<std::string> options{ "new game", "load game", "exit" };
	if( _GAMEVERSION == "debug" )
		options = { "new game", "load game", "exit", "debugprintalllocationdata", "debugprintallitemdata", "debugprintallenemydata", "debuggotolocation", "debugbattle" };

	std::string menuChoise;
	getInputUntilStrings( menuChoise, options, 0, true, false, false, true, true, 49, console );

	introMusic.stopSound();
	if( menuChoise == "new game" )
		charCreation( console, player, rng );
	else if( menuChoise == "load game" )
		loadMenu( console, player, rng );
	else if( menuChoise == "exit" )
		return false;
	else if( menuChoise == "debugprintalllocationdata" )
	{
		console.clearScreen();
		debugPrintAllLocationData();
		console.getEnter();
	}
	else if( menuChoise == "debugprintallitemdata" )
	{
		console.clearScreen();
		debugPrintAllItemData();
		console.getEnter();
	}
	else if( menuChoise == "debugprintallenemydata" )
	{
		console.clearScreen();
		debugPrintAllEnemyData();
		console.getEnter();
	}
	else if( menuChoise == "debuggotolocation" )
	{
		console.clearScreen();
		std::cout << "Location: ";
		auto locChoise = getInputString( 0, false, true, true, true, 0, console );
		execLocation( locChoise, "castle.txt", console, player, rng );
	}
	else if( menuChoise == "debugbattle" )
		execBattle( "Ogre", 4, console, player, rng );

	return true;
}
void charCreation( Console& console, Player& player, RNG& rng )
{
	auto reload = true;
	auto skipStat = false;
	while( reload )
	{
		console.clearScreen();
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout << "                                                    CREATE YOUR CHARACTER"                                                       << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout << "What is your name?"                                                                                                              << '\n';
		auto name = getInputString( 16, false, false, false, false, 19, console ); std::cout                                                           << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout << "What is your gender?"                                                                                                            << '\n';
		std::string sex; getInputUntilStrings( sex, { "male", "female" }, 6, true, false, false, true, false, 22, console ); std::cout                 << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout << "What is your age?"                                                                                                               << '\n';
		int age; getInputUntilIntRange( age, 1, 100, true, false, 25, console ); std::cout                                                             << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout << "Confirm these choises? (type \"yes\" to proceed, \"no\" to start over or \"back\" to go back to main menu)"                      << '\n';
		std::string input; getInputUntilStrings( input, { "y", "yes", "n", "no", "back" }, 0, true, false, false, true, true, 49, console ); std::cout << '\n';

		if( input == "y" || input == "yes" )
		{
			reload = false;
			player.setCharName( name );
			player.setCharSex( sex );
			player.setCharAge( age );
		}
		else if( input == "back" )
		{
			reload = false;
			mainMenu( console, player, rng );
			skipStat = true;
		}
	}

	if( !skipStat )
		statMenu( console, player, rng );
}
void statMenu( Console& console, Player& player, RNG& rng )
{
	auto reload = true;
	while( reload )
	{
		auto ptsLeft = 15;
		console.clearScreen();
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout << "                                                 CHOOSE YOUR STARTING SKILLS"                                                    << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout << "                           You have: " << ptsLeft << " skillpoints left to spend."                                               << '\n'; // 13
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n'; // 15
		std::cout                                                                                                                                      << '\n';
		std::cout << "                           0         5         10        15        20"                                                           << '\n'; // 17
		std::cout << "1. Health                > [ | | | | | | | | | |                     ]"                                                          << '\n'; // 18
		std::cout                                                                                                                                      << '\n';
		std::cout << "                           0         5         10        15        20"                                                           << '\n';
        std::cout << "2. Strength                [ | | | | | | | | | |                     ]"                                                          << '\n'; // 21
		std::cout                                                                                                                                      << '\n';
		std::cout << "                           0         5         10        15        20"                                                           << '\n';
		std::cout << "3. Intelligence            [ | | | | | | | | | |                     ]"                                                          << '\n'; // 24
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout << "Use the up & down keys to navigate between skills and"                                                                           << '\n';
		std::cout << "the left & right keys to increase/decrease their respective values."                                                             << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout << "Press ENTER when you're done."                                                                                                   << '\n';
	
		auto health = 10;
		auto strength = 10;
		auto intel = 10;
		auto xp = 0;
		auto curStat = 1;
		auto hax = 0;

		console.setCursorPosition( 0, 49 );
		for( auto ch = _getch(); ch != '\r' && hax < 7; ch = _getch() )
		{
			auto ch1 = 0;
			if( ch == 0 || ch == 0xE0 )
			{
				ch1 = _getch();
				if( ch1 == 72 && curStat > 1 ) // UP
					--curStat;
				else if( ch1 == 80 && curStat < 3 ) // DOWN
					++curStat;
				else if( ch1 == 75 ) // LEFT
				{
					switch( curStat )
					{
					case 1:
						if( health > 10 )
						{
							--health;
							++ptsLeft;
							console.setCursorPosition( 29 + ( health * 2 ), 15 + ( curStat * 3 ) );
							std::cout << "  ";
						}
						break;
					case 2:
						if( strength > 10 )
						{
							--strength;
							++ptsLeft;
							console.setCursorPosition( 29 + ( strength * 2 ), 15 + ( curStat * 3 ) );
							std::cout << "  ";
						}
						break;
					case 3:
						if( intel > 10 )
						{
							--intel;
							++ptsLeft;
							console.setCursorPosition( 29 + ( intel * 2 ), 15 + ( curStat * 3 ) );
							std::cout << "  ";
						}
						break;
					}
				}
				else if( ch1 == 77 ) // RIGHT
				{
					switch( curStat )
					{
					case 1:
						if( health < 20 && ptsLeft > 0 )
						{
							++health;
							--ptsLeft;
							console.setCursorPosition( 27 + ( health * 2 ), 15 + ( curStat * 3 ) );
							std::cout << "| ";
						}
						break;
					case 2:
						if( strength < 20 && ptsLeft > 0 )
						{
							++strength;
							--ptsLeft;
							console.setCursorPosition( 27 + ( strength * 2 ), 15 + ( curStat * 3 ) );
							std::cout << "| ";
						}
						break;
					case 3:
						if( intel < 20 && ptsLeft > 0 )
						{
							++intel;
							--ptsLeft;
							console.setCursorPosition( 27 + ( intel * 2 ), 15 + ( curStat * 3 ) );
							std::cout << "| ";
						}
						break;
					}
				}
			}

			// Print ptsLeft at top.
			console.setCursorPosition( 37, 13 );
			std::cout << "  ";
			console.setCursorPosition( 37, 13 );
			std::cout << ptsLeft;

			if( ch1 == 72 || ch1 == 80  ) // UP OR DOWN
			{
				console.setCursorPosition( 25, 18 );
				_putch( ' ' );
				console.setCursorPosition( 25, 21 );
				_putch( ' ' );
				console.setCursorPosition( 25, 24 );
				_putch( ' ' );
				console.setCursorPosition( 25, 15 + ( curStat * 3 ) );
				_putch( '>' );
			}
			
			if( ch == '1' && hax == 0 )
				++hax;
			else if( ch == '3' && hax == 1 )
				++hax;
			else if( ch == '3' && hax == 2 )
				++hax;
			else if( ch == '7' && hax == 3 )
				++hax;
			else if( ch == 'h' && hax == 4 )
				++hax;
			else if( ch == 'a' && hax == 5 )
				++hax;
			else if( ch == 'x' && hax == 6 )
			{
				++hax;
				health = 1337;
				strength = 1337;
				intel = 1337;
				xp = 133700;
			}
			else
				hax = 0;

			console.setCursorPosition( 0, 49 );
		}
		console.setCursorPosition( 0, 48 );
		std::cout << "Confirm these choises? (type \"yes\" to proceed, \"no\" to start over or \"back\" to go back to character creation)\n";
		std::string input; getInputUntilStrings( input, { "y", "yes", "n", "no", "back" }, 0, true, false, false, true, true, 49, console ); std::cout << '\n';

		if( input == "y" || input == "yes" )
		{
			reload = false;
			player.setHealth( health );
			player.setStrength( strength );
			player.setIntelligence( intel );
			player.setXp( xp );
			player.setMoney( 10 );
		}
		else if( input == "back" )
		{
			reload = false;
			charCreation( console, player, rng );
		}
	}

	confirmMenu( console, player, rng );
}
void confirmMenu( Console& console, Player& player, RNG& rng )
{
	console.clearScreen();
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "                                                      CONFIRM CHARACTER"                                                         << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "Name: " << player.getCharName()                                                                                                  << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "Gender: " << player.getCharSex()                                                                                                 << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "Age: " << player.getCharAge()                                                                                                    << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "STATS:"                                                                                                                          << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "Health: " << player.getHealth()                                                                                                  << '\n';
	std::cout << "Strength: " << player.getStrength()                                                                                              << '\n';
	std::cout << "Intelligence: " << player.getIntelligence()                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "Confirm these choises? (type \"yes\" to proceed or \"no\" to go back to character creation)"                                     << '\n';
	std::string input; getInputUntilStrings( input, { "y", "yes", "n", "no" }, 0, true, false, false, true, true, 49, console );

	if( input == "n" || input == "no" )
		charCreation( console, player, rng );
}
void helpMenu( Console& console )
{
	console.clearScreen();
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "                                                      STANDARD COMMANDS"                                                         << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "\"HELP\" - show this menu"                                                                                                       << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "\"INFO\" - show full information about your current location"                                                                    << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "\"INVENTORY\" - access your inventory"                                                                                           << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "\"STATS\" - show information about yout character"                                                                               << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "\"MONEY\" - show your current balance"                                                                                           << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "\"LOCATION\" - show your current location"                                                                                       << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "\"SAVE\" - open the save game menu"                                                                                              << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	console.getEnter();
}
void saveMenu( Console& console, Player& player )
{
	console.clearScreen();
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "                                                          SAVE GAME"                                                             << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "Type the filename you want and press ENTER to make a new save of your current game."                                             << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "Press ENTER without typing anything to cancel."                                                                                  << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	auto result = getInputString( 20, false, true, true, true, 49, console );

	if( !result.empty() )
	{
		result.append( ".dat" );
		deleteSaveGame( result );

		if( !saveGame( result, player ) )
			logError( "Failed to save game!" );
		else
		{
			console.clearScreen();
			std::cout << "Game saved as: " << result << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
			console.getEnter();
		}
	}
	else
		logError( "Failed to save game!", "Empty filename" );
}
void loadMenu( Console& console, Player& player, RNG& rng )
{
	bool reload = true;
	while( reload )
	{
		// Load all savegames.
		struct SaveGameInfo final
		{
			std::string name;
			std::string date;
			std::string charName;
		};

		std::vector<SaveGameInfo> saveInfo;
		std::ifstream saveGamesFile( "game/saves/gamesaves.txt" );
		if( saveGamesFile.is_open() )
		{
			for( std::string buffer; getline( saveGamesFile, buffer ); )
			{
				if( buffer.find( ".dat" ) != std::string::npos )
				{
					std::string date; getline( saveGamesFile, date );
					std::string charName; getline( saveGamesFile, charName );
					saveInfo.push_back( SaveGameInfo{ buffer, date, charName } );
				}
			}
			saveGamesFile.close();
		}
		else
			logError( "Unable to open savegame data file!" );

		// Print menu.
		console.clearScreen();
		std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << '\n';
		std::cout << '\n';
		std::cout << "                                                          LOAD GAME" << '\n';
		std::cout << '\n';
		std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << '\n';
		std::cout << "                                              |                                                                                " << '\n';
		std::cout << "     01.                                      |                                                                                " << '\n';
		std::cout << "     02.                                      |                +----------------------------------------------+                " << '\n';
		std::cout << "     03.                                      |                |X                                            X|                " << '\n';
		std::cout << "     04.                                      |                +----------------------------------------------+                " << '\n';
		std::cout << "     05.                                      |                                                                                " << '\n';
		std::cout << "     06.                                      |                 Character name:                                                " << '\n';
		std::cout << "     07.                                      |                                                                                " << '\n';
		std::cout << "     08.                                      |                 Save date:                                                     " << '\n';
		std::cout << "     09.                                      |                                                                                " << '\n';
		std::cout << "     10.                                      |                                                                                " << '\n';
		std::cout << "     11.                                      |                                                                                " << '\n';
		std::cout << "     12.                                      |                                                                                " << '\n';
		std::cout << "     13.                                      |                                                                                " << '\n';
		std::cout << "     14.                                      |                                                                                " << '\n';
		std::cout << "     15.                                      |                                                                                " << '\n';
		std::cout << "     16.                                      |                                                                                " << '\n';
		std::cout << "     17.                                      |                                                                                " << '\n';
		std::cout << "     18.                                      |                                                                                " << '\n';
		std::cout << "     19.                                      |                                                                                " << '\n';
		std::cout << "     20.                                      |                                                                                " << '\n';
		std::cout << "     21.                                      |                                                                                " << '\n';
		std::cout << "     22.                                      |                                                                                " << '\n';
		std::cout << "     23.                                      |                                                                                " << '\n';
		std::cout << "     24.                                      |                                                                                " << '\n';
		std::cout << "     25.                                      |                                                                                " << '\n';
		std::cout << "     26.                                      |                                                                                " << '\n';
		std::cout << "     27.                                      |                                                                                " << '\n';
		std::cout << "     28.                                      |                                                                                " << '\n';
		std::cout << "     29.                                      |                                                                                " << '\n';
		std::cout << "     30.                                      |                                                                                " << '\n';
		std::cout << "                                              |                                                                                " << '\n';
		std::cout << "----------------------------------------------+                                                                                " << '\n';
		std::cout << "                                              |                                                                                " << '\n';
		std::cout << "                                              |                                                                                " << '\n';
		std::cout << "     Select a savegame and press ENTER        |                                                                                " << '\n';
		std::cout << "     to load it.                              |                                                                                " << '\n';
		std::cout << "                                              |                                                                                " << '\n';
		std::cout << "     Press BACKSPACE to cancel.               |                                                                                " << '\n';
		std::cout << "                                              |                                                                                " << '\n';
		std::cout << "     Press CTRL + BACKSPACE to delete         |                                                                                " << '\n';
		std::cout << "     a savegame.                              |                                                                                " << '\n';
		std::cout << "                                              |                                                                                " << '\n';
		std::cout << "                                              |                                                                                " << '\n';
		std::cout << "                                              |                                                                                ";

		// Print all savegame names.
		for( auto i = decltype( saveInfo.size() ){ 0 }; i < saveInfo.size(); ++i )
		{
			console.setCursorPosition( 9, 6 + static_cast<short>( i ) );
			std::cout << saveInfo[ i ].name;
		}

		// Print cursor.
		console.setCursorPosition( 9, 6 );
		if( !saveInfo.empty() )
			console.setCursorPosition( 9 + static_cast<short>( saveInfo[ 0 ].name.length() ) + 1, 6 );
		_putch( '<' );

		// Define printInfo lambda function.
		auto printInfo = [saveInfo]( const int curRow, Console& console )
		{
			console.setCursorPosition( 68, 8 );
			std::cout << "                                      ";
			console.setCursorPosition( 82, 11 );
			std::cout << "                                   ";
			console.setCursorPosition( 77, 13 );
			std::cout << "                                   ";
			if( curRow <= static_cast<int>( saveInfo.size() ) )
			{
				console.setCursorPosition( 87 - static_cast<short>( saveInfo[ curRow - 1 ].name.length() / 2 ), 8 );
				std::cout << saveInfo[ curRow - 1 ].name;
				console.setCursorPosition( 110 - static_cast<short>( saveInfo[ curRow - 1 ].charName.length() ), 11 );
				std::cout << saveInfo[ curRow - 1 ].charName;
				console.setCursorPosition( 110 - static_cast<short>( saveInfo[ curRow - 1 ].date.length() ), 13 );
				std::cout << saveInfo[ curRow - 1 ].date;
			}
			console.setCursorPosition( 0, 49 );
		};
		// Print info of savegame at cursor start.
		auto curRow = 1;
		printInfo( curRow, console );

		// Handle input.
		console.setCursorPosition( 0, 49 );
		auto ch = _getch();
		for( ; ch != '\b' && ch != 127 && ch != '\r'; ch = _getch() )
		{
			auto curNameLen = static_cast<short>( ( static_cast<int>( saveInfo.size() ) < curRow ) ? 0 : 1 + saveInfo[ curRow - 1 ].name.length() );
			console.setCursorPosition( 9 + curNameLen, 5 + curRow );
			_putch( ' ' );

			if( ch == 0 || ch == 0xE0 )
			{
				auto ch1 = _getch();
				if( ch1 == 72 && curRow > 1 ) // UP
					--curRow;
				else if( ch1 == 80 && curRow < 30 ) // DOWN
					++curRow;
			}

			curNameLen = static_cast<short>( ( static_cast<int>( saveInfo.size() ) < curRow ) ? 0 : 1 + saveInfo[ curRow - 1 ].name.length() );
			console.setCursorPosition( 9 + curNameLen, 5 + curRow );
			_putch( '<' );

			printInfo( curRow, console );
		}

		if( ch == '\r' && curRow <= static_cast<int>( saveInfo.size() ) )
		{
			if( loadGame( saveInfo[ curRow - 1 ].name, player ) )
				reload = false;
		}
		else if( ch == '\b' )
		{
			reload = false;
			mainMenu( console, player, rng );
		}
		else if( ch == 127 && curRow <= static_cast<int>( saveInfo.size() ) )
			deleteSaveGame( saveInfo[ curRow - 1 ].name );
	}
}
void showStatMenu( Console& console, Player& player )
{
	const auto gender = (player.getCharSex() == "male") ? "man" : "woman";
	const auto level = player.getLevel();
	auto xpNeeded = 100 - ( player.getXp() - level * 100 );
	if( xpNeeded == 0 )
		xpNeeded = 100;

	console.clearScreen();
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "                                                        YOUR CHARACTER"                                                          << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "Your name is " << player.getCharName() << '.'                                                                                    << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "You are a " << player.getCharAge() << "-year-old " << gender << '.'                                                              << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "STATS:"                                                                                                                          << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "Health: " << player.getHealth()                                                                                                  << '\n';
	std::cout << "Strength: " << player.getStrength()                                                                                              << '\n';
	std::cout << "Intelligence: " << player.getIntelligence()                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "Level: " << level                                                                                                                << '\n';
	std::cout << "XP to next level: " << xpNeeded << " (" << (100 - xpNeeded) << "/100)"                                                           << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "Money: " << player.getMoney()                                                                                                    << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout                                                                                                                                      << '\n';
	console.getEnter();
}
void showStatChange( const std::string& statType, const int value, Console& console )
{
	console.clearScreen();
	if( value >= 0 )
		std::cout << "You gained " << value << ' ' << statType << "!\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
	else
		std::cout << "You lost " << abs(value) << ' ' << statType << "!\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
	console.getEnter();
}
bool execInventory( std::string& resultLocation, const bool allowBattleItems, Console& console, Player& player )
{
	console.clearScreen();
	std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "                                                           INVENTORY"                                                            << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "-------------------------------+-------------------------------+---------------------------------------------------------------" << '\n';
	std::cout << "             ITEMS             |            WEAPONS            |                             INFO                              " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << " 01.                           | 01.                           |        +---------------------------------------------+        " << '\n'; //7       (5, 7) (37, 7)
	std::cout << " 02.                           | 02.                           |        |                                             |        " << '\n'; //8       (96, 8)
	std::cout << " 03.                           | 03.                           |        +---------------------------------------------+        " << '\n';
	std::cout << " 04.                           | 04.                           |                                                               " << '\n';
	std::cout << " 05.                           | 05.                           |                                                               " << '\n';
	std::cout << " 06.                           | 06.                           |                                                               " << '\n'; //12      (73, 12)
	std::cout << " 07.                           | 07.                           |                                                               " << '\n';
	std::cout << " 08.                           | 08.                           |                                                               " << '\n';
	std::cout << " 09.                           | 09.                           |                                                               " << '\n';
	std::cout << " 10.                           | 10.                           |                                                               " << '\n';
	std::cout << " 11.                           | 11.                           |                                                               " << '\n';
	std::cout << " 12.                           | 12.                           |                                                               " << '\n';
	std::cout << " 13.                           | 13.                           |                                                               " << '\n';
	std::cout << " 14.                           | 14.                           |                                                               " << '\n';
	std::cout << " 15.                           | 15.                           |                                                               " << '\n';
	std::cout << " 16.                           | 16.                           |                                                               " << '\n';
	std::cout << " 17.                           | 17.                           |                                                               " << '\n';
	std::cout << " 18.                           | 18.                           |                                                               " << '\n';
	std::cout << " 19.                           | 19.                           |                                                               " << '\n';
	std::cout << " 20.                           | 20.                           |                                                               " << '\n';
	std::cout << " 21.                           | 21.                           |                                                               " << '\n';
	std::cout << " 22.                           | 22.                           |                                                               " << '\n';
	std::cout << " 23.                           | 23.                           |                                                               " << '\n';
	std::cout << " 24.                           | 24.                           |                                                               " << '\n';
	std::cout << " 25.                           | 25.                           |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "-------------------------------+-------------------------------+                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "    Select an item using the arrow keys                        |                                                               " << '\n';
	std::cout << "    and press ENTER to use it.                                 |                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "    Press BACKSPACE to cancel.                                 |                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "                                                               |                                                               ";

	console.setCursorPosition( 73, 8 );
	std::cout << "X                                           X";
	console.setCursorPosition( 38, 43 );
	std::cout << "Your Strength: " << player.getStrength();
	console.setCursorPosition( 38, 44 );
	std::cout << "Your Intelligence: " << player.getIntelligence();
	console.setCursorPosition( 38, 45 );
	std::cout << "Your Health: " << player.getHealth();

	// Get all items.
	std::vector<ItemConsumable> allItems;
	for( const auto& item : player.getInventory().items )
		allItems.push_back( item.second );

	// Get all weapons.
	std::vector<Weapon> allWeapons;
	for( const auto& wep : player.getInventory().weapons )
		allWeapons.push_back( wep.second );

	// Print all items and their number of uses.
	for( auto i = decltype( allItems.size() ){ 0 }; i < allItems.size(); ++i )
	{
		console.setCursorPosition( 5, 7 + static_cast<short>( i ) );
		std::cout << allItems[ i ].codeName;

		auto usesLength = std::to_string( allItems[ i ].uses ).length();
		auto nSpaces = 25 - usesLength - allItems[ i ].codeName.length();
		for( auto i = decltype( nSpaces ){ 0 }; i < nSpaces; ++i )
			_putch( ' ' );

		std::cout << allItems[ i ].uses;
	}
	// Print all weapons.
	for( auto i = decltype( allWeapons.size() ){ 0 }; i < allWeapons.size(); ++i )
	{
		console.setCursorPosition( 37, 7 + static_cast<short>( i ) );
		std::cout << allWeapons[ i ].codeName;
	}

	auto getEquippedWeaponIndex = [allWeapons, player]()
	{
		// Get equipped weapon.
		for( auto i = decltype( allWeapons.size() ){ 0 }; i < allWeapons.size(); ++i )
		{
			if( allWeapons[ i ].codeName == player.getEquippedWeapon() )
				return static_cast<int>( i );
		}
		return -1;
	};
	auto putBracketsAroundEquippedWeapon = [allWeapons, player, getEquippedWeaponIndex]( Console& console )
	{
		auto index = getEquippedWeaponIndex();
		// Put brackets around it.
		if( index >= 0 )
		{
			console.setCursorPosition( 36, 6 + index + 1 );
			std::cout << '[';
			console.setCursorPosition( 37 + static_cast<short>( player.getEquippedWeapon().length() ), 6 + index + 1 );
			std::cout << ']';
			console.setCursorPosition( 0, 49 );
		}
	};
	auto equipWeaponAtRow = [allWeapons, getEquippedWeaponIndex, putBracketsAroundEquippedWeapon ]( const int row, Console& console, Player& player )
	{
		// Get equipped weapon.
		auto index = getEquippedWeaponIndex();
		if( index >= 0 )
		{
			// Clear brackets from previous weapon.
			console.setCursorPosition( 36, 7 + index );
			_putch( ' ' );
			console.setCursorPosition( 37 + static_cast<short>( player.getEquippedWeapon().length() ), 7 + index );
			_putch( ' ' );
		}
		// Equip new weapon.
		player.setEquippedWeapon( allWeapons[ row - 1 ].codeName );
		// Put brackets around new weapon.
		putBracketsAroundEquippedWeapon( console );
	};
	auto getCurNameLength = [allItems, allWeapons]( const int curRow, const int curColumn )
	{
		if( curColumn == 1 && curRow <= static_cast<int>( allItems.size() ) )
			return allItems[ curRow - 1 ].codeName.length() + 1;
		else if( curColumn == 2 && curRow <= static_cast<int>( allWeapons.size() ) )
			return allWeapons[ curRow - 1 ].codeName.length() + 1;
		return 0u;
	};
	auto clearCursor = [getCurNameLength]( const int curRow, const int curColumn, Console& console )
	{
		console.setCursorPosition( -27 + ( curColumn * 32 ) + static_cast<short>( getCurNameLength( curRow, curColumn ) ), 6 + curRow );
		_putch( ' ' );
	};
	auto printCursor = [getCurNameLength]( const int curRow, const int curColumn, Console& console )
	{
		console.setCursorPosition( -27 + ( curColumn * 32 ) + static_cast<short>( getCurNameLength( curRow, curColumn ) ), 6 + curRow );
		_putch( '<' );
	};
	auto showInventoryInfo = [allItems, allWeapons, player]( const int curRow, const int curColumn, Console& console )
	{
		// Clear previous info.
		console.setCursorPosition( 74, 8 );
		std::cout << "                                           ";
		for( auto i = 12; i <= 20; i += 2 ) {
			console.setCursorPosition( 73, i );
			std::cout << "                                                  ";
		}
		for( auto i = 21; i <= 23; ++i ) {
			console.setCursorPosition( 73, i );
			std::cout << "                                                  ";
		}
		for( auto i = 37; i <= 39; ++i ) {
			console.setCursorPosition( 73, i );
			std::cout << "                                                  ";
		}

		// Print new info.
		if( curColumn == 1 && curRow <= static_cast<int>( allItems.size() ) ) // Currently selected item is an Item.
		{
			console.setCursorPosition( 95 - static_cast<short>( allItems[ curRow - 1 ].name.length() / 2 ), 8 );
			std::cout << allItems[ curRow - 1 ].name;
			console.setCursorPosition( 73, 12 );
			std::cout << "Uses: " << allItems[ curRow - 1 ].uses;
			console.setCursorPosition( 73, 14 );
			std::cout << "Weight: " << allItems[ curRow - 1 ].weight;
			console.setCursorPosition( 73, 16 );
			std::cout << "Worth: $" << allItems[ curRow - 1 ].worth;

			if( allItems[ curRow - 1 ].damage > 0 )
			{
				console.setCursorPosition( 100, 14 );
				std::cout << "Damage: " << allItems[ curRow - 1 ].damage;
			}
			if( !allItems[ curRow - 1 ].damageType.empty() )
			{
				console.setCursorPosition( 100, 12 );
				std::cout << "Damage type: " << allItems[ curRow - 1 ].damageType;
			}

			std::vector<std::string> conds;
			if( allItems[ curRow - 1 ].strengthCond > 0 )
				conds.push_back( "Strength Requirement: " + std::to_string( allItems[ curRow - 1 ].strengthCond ) );
			if( allItems[ curRow - 1 ].intelligenceCond > 0 )
				conds.push_back( "Intel. Requirement: " + std::to_string( allItems[ curRow - 1 ].intelligenceCond ) );
			if( allItems[ curRow - 1 ].healthCond > 0 )
				conds.push_back( "Health Requirement: " + std::to_string( allItems[ curRow - 1 ].healthCond ) );
			for( auto i = decltype( conds.size() ){ 0 }; i < conds.size(); ++i )
			{
				console.setCursorPosition( 73, 37 + static_cast<short>( i ) );
				std::cout << conds[ i ];
			}

			auto desc = allItems[ curRow - 1 ].desc;
			for( auto y = 20; !desc.empty(); ++y )
			{
				console.setCursorPosition( 73, y );
				std::cout << desc.substr( 0, desc.find( '\n' ) );
				if( desc.find( '\n' ) == std::string::npos )
					desc.clear();
				else
					desc.erase( 0, desc.find( '\n' ) + 1 );
			}
		}
		else if( curColumn == 2 && curRow <= static_cast<int>( allWeapons.size() ) ) // Currently selected item is a Weapon.
		{
			console.setCursorPosition( 95 - static_cast<short>( allWeapons[ curRow - 1 ].name.length() / 2 ), 8 );
			std::cout << allWeapons[ curRow - 1 ].name;
			console.setCursorPosition( 73, 12 );
			std::cout << "Damage: " << allWeapons[ curRow - 1 ].damage;
			console.setCursorPosition( 73, 14 );
			std::cout << "Weight: " << allWeapons[ curRow - 1 ].weight;
			console.setCursorPosition( 73, 16 );
			std::cout << "Worth: $" << allWeapons[ curRow - 1 ].worth;

			std::vector<std::string> conds;
			if( allWeapons[ curRow - 1 ].strengthCond > 0 )
				conds.push_back( "Strength Requirement: " + std::to_string( allWeapons[ curRow - 1 ].strengthCond ) );
			if( allWeapons[ curRow - 1 ].intelligenceCond > 0 )
				conds.push_back( "Intel. Requirement: " + std::to_string( allWeapons[ curRow - 1 ].intelligenceCond ) );
			if( allWeapons[ curRow - 1 ].healthCond > 0 )
				conds.push_back( "Health Requirement: " + std::to_string( allWeapons[ curRow - 1 ].healthCond ) );
			for( auto i = decltype( conds.size() ){ 0 }; i < conds.size(); ++i )
			{
				console.setCursorPosition( 73, 37 + static_cast<short>( i ) );
				std::cout << conds[ i ];
			}

			auto desc = allWeapons[ curRow - 1 ].desc;
			for( auto y = 20; !desc.empty(); ++y )
			{
				console.setCursorPosition( 73, y );
				std::cout << desc.substr( 0, desc.find( '\n' ) );
				desc.erase( 0, desc.find( '\n' ) + 1 );
			}
		}
	};

	auto curRow = 1;
	auto curColumn = 1;
	// Put brackets around starting equipped weapon.
	putBracketsAroundEquippedWeapon( console );
	// Print cursor at start position.
	printCursor( curRow, curColumn, console );
	// Print inventory info of item at cursor start.
	showInventoryInfo( curRow, curColumn, console );

	// Handle input.
	console.setCursorPosition( 0, 49 );
	std::string result;
	for( auto ch = -1; ch != '\b' && result.empty(); )
	{
		ch = _getch();
		// Clear previous error string.
		console.setCursorPosition( 4, 45 );
		std::cout << "                                 ";
		// Clear previous cursor.
		clearCursor( curRow, curColumn, console );

		// Get new cursor position.
		if( ch == 0 || ch == 0xE0 )
		{
			const auto ch1 = _getch();
			if( ch1 == 72 && curRow > 1 ) // UP
				--curRow;
			else if( ch1 == 80 && curRow < 25 ) // DOWN
				++curRow;
			else if( ch1 == 75 && curColumn > 1 ) // LEFT
				--curColumn;
			else if( ch1 == 77 && curColumn < 2 ) // RIGHT
				++curColumn;
		}

		// Print new info.
		showInventoryInfo( curRow, curColumn, console );
		// Print new cursor.
		printCursor( curRow, curColumn, console );

		if( ch == '\r' )
		{
			if( curColumn == 1 && curRow <= static_cast<int>( allItems.size() ) ) // "Use item" request
			{
				if( player.getStrength() >= allItems[ curRow - 1 ].strengthCond &&
					player.getIntelligence() >= allItems[ curRow - 1 ].intelligenceCond &&
					player.getHealth() >= allItems[ curRow - 1 ].healthCond &&
					allItems[ curRow - 1 ].isBattleItem == false )
					result = allItems[ curRow - 1 ].codeName;
				else if( allItems[ curRow - 1 ].isBattleItem && allowBattleItems )
					result = allItems[ curRow - 1 ].codeName;
				else if( allItems[ curRow - 1 ].isBattleItem && !allowBattleItems )
				{
					console.setCursorPosition( 4, 45 );
					std::cout << "You are not in battle!";
					console.setCursorPosition( 0, 49 );
				}
				else
				{
					console.setCursorPosition( 4, 45 );
					std::cout << "You can't use that item!";
					console.setCursorPosition( 0, 49 );
				}
			}
			else if( curColumn == 2 && curRow <= static_cast<int>( allWeapons.size() ) ) // "Equip weapon" reqeust
			{
				if( player.getStrength() >= allWeapons[ curRow - 1 ].strengthCond &&
					player.getIntelligence() >= allWeapons[ curRow - 1 ].intelligenceCond &&
					player.getHealth() >= allWeapons[ curRow - 1 ].healthCond )
				{
					equipWeaponAtRow( curRow, console, player );
				}
				else
				{
					console.setCursorPosition( 4, 45 );
					std::cout << "You can't equip that weapon!";
					console.setCursorPosition( 0, 49 );
				}
			}
		}
		console.setCursorPosition( 0, 49 );
	}

	if( !result.empty() )
	{
		resultLocation = result;
		return true;
	}
	else
		return false;
}

void debugPrintAllLocationData()
{
	debugPrintLocationData( "castleStart", "castle.txt" );
	debugPrintLocationData( "castleStartLookArch", "castle.txt" );
	debugPrintLocationData( "castleStartLookWall", "castle.txt" );
	debugPrintLocationData( "castleStartWait", "castle.txt" );
	debugPrintLocationData( "castleStartLookBottle", "castle.txt" );
	debugPrintLocationData( "castleStartTakeBottle", "castle.txt" );
	debugPrintLocationData( "castleStartNoTakeBottle", "castle.txt" );
	debugPrintLocationData( "useBottle1337", "castle.txt" );
	debugPrintLocationData( "castleHall", "castle.txt" );
	debugPrintLocationData( "castleDark", "castle.txt" );
}
void debugPrintAllItemData()
{
	debugPrintItemData( "Bottle1337" );
	debugPrintItemData( "RotBanana" );
	debugPrintItemData( "Orng" );
}
void debugPrintAllEnemyData()
{
	debugPrintEnemyData( "Ogre" );
}