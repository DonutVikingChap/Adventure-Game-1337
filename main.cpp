#include "Player.h"
#include "UI.h"
#include "LocationData.h"
#include "RNG.h"
#include "Console.h"
#include <iostream>

int main( int argc, char *argv[] )
{
	Console console( "Adventure Game 1337", 128, 50, FOREGROUND_GREEN | FOREGROUND_INTENSITY );

	RNG rng;

	Player player1;
	player1.setHealth( 15 );
	player1.setStrength( 15 );
	player1.setIntelligence( 15 );
	player1.setMoney( 10 );
	player1.setXp( 0 );
	player1.setGameOver( false );
	player1.setCurrentLocation( "castleStart" );
	player1.setCurFile( "castle.txt" );
	player1.initializeConds( "castle.txt" );

	for( bool mainMenuLoop = true; mainMenuLoop; )
	{
		if( mainMenu( console, player1, rng ) )
		{
			for( auto next = player1.getCurrentLocation(); !player1.isGameOver() && next != "_FAIL"; )
			{
				if( next.substr( 0, 3 ) == "use" )
				{
					auto oldLoc = player1.getCurFile();
					next = execLocation( next, "gameitemdata.txt", console, player1, rng, true );
					player1.setCurFile( oldLoc );
				}
				else
					next = execLocation( next, player1.getCurFile(), console, player1, rng );
			}
			gameOver( console, player1 );
		}
		else
			mainMenuLoop = false;
	}

	return 0;
}