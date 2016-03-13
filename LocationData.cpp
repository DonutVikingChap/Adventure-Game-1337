#include "LocationData.h"
#include "Console.h"
#include "Player.h"
#include "PerformanceTimer.h"
#include "UI.h"
#include "GameVersion.h"
#include "RNG.h"
#include "Sound.h"

#include <chrono>
#include <map>
#include <fstream>
#include <iostream>
#include <conio.h>

std::unordered_map<std::string, Location> locationCache;
std::unordered_map<std::string, ItemConsumable> itemCache;
std::unordered_map<std::string, Enemy> enemyCache;
std::unordered_map<std::string, Weapon> weaponCache;
std::unordered_map<std::string, Shop> shopCache;
std::unordered_map<std::string, std::string> locationNewFileCache;

bool saveGame( const std::string& fileName, Player& player )
{
	std::ofstream saveFile( "game/saves/" + fileName );
	if( saveFile.is_open() )
	{
		// Prefix a '_' to the current location if the player has already been here before.
		auto curLoc = player.getCurrentLocation();
		if( player.hasBeenLocation( player.getCurrentLocation() ) && loadLocationData( player.getCurrentLocation(), player.getCurFile(), false ).optionsName.empty() == false )
			curLoc.insert( curLoc.begin(), '_' );

		saveFile << player.getCharName() << std::endl;						// name
		saveFile << player.getCharAge() << std::endl;						// age
		saveFile << player.getCharSex() << std::endl;						// sex
		saveFile << player.getXp() << std::endl;							// xp
		saveFile << player.getHealth() << std::endl;						// health
		saveFile << player.getStrength() << std::endl;						// strength
		saveFile << player.getIntelligence() << std::endl;					// intelligence
		saveFile << player.getMoney() << std::endl;							// money
		saveFile << curLoc << std::endl;									// current location
		saveFile << player.getCurFile() << std::endl;						// current file
		saveFile << player.getPrevLocationsString() << std::endl;			// previous locations
		saveFile << player.getEquippedWeapon() << std::endl;				// equipped weapon
		for( const auto& cond : player.getItemConds() )
			saveFile << cond << std::endl;									// item conditions
		saveFile << '*' << std::endl;
		for( const auto& cond : player.getLocConds() )
			saveFile << cond << std::endl;									// location conditions
		saveFile << '*' << std::endl;

		std::vector<ItemConsumable> allItems;
		for( const auto& item : player.getInventory().items )
			allItems.push_back( item.second );
		std::vector<Weapon> allWeapons;
		for( const auto& wep : player.getInventory().weapons )
			allWeapons.push_back( wep.second );

		for( const auto& item : allItems )
		{
			saveFile << item.codeName << std::endl;							// item names
			saveFile << '#' << item.uses << std::endl;						// item uses
		}
		saveFile << '*' << std::endl;
		for( const auto& wep : allWeapons )
			saveFile << wep.codeName << std::endl;							// weapon names
		saveFile << '*' << std::endl;
		for( const auto& file : ::locationNewFileCache )
			saveFile << file.first << ',' << file.second << '#';			// filename cache
		saveFile << std::endl << '*' << std::endl;

		saveFile.close();
	}
	else
	{
		logError( "Unable to open file for saving!" );
		return false;
	}

	std::ofstream savesListFile( "game/saves/gamesaves.txt", std::ios::app );
	if( savesListFile.is_open() )
	{
		savesListFile << fileName << std::endl;

		auto time = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );
		struct tm localTime;
		localtime_s( &localTime, &time );
		char str[ 256 ];
		auto error = asctime_s( str, 256, &localTime );
		if( error != 0 )
		{
			logError( "asctime_s failed!", std::to_string( error ) );
			savesListFile << std::endl;
		}
		else
			savesListFile << str;

		savesListFile << player.getCharName() << std::endl;
		savesListFile << "*" << std::endl;

		savesListFile.close();
	}
	else
	{
		logError( "Unable to open saves file for saving!" );
		return false;
	}
	return true;
}
bool loadGame( const std::string& fileName, Player& player )
{
	std::ifstream loadFile( "game/saves/" + fileName );
	if( loadFile.is_open() )
	{
		player.clearConds();

		std::string name; std::getline( loadFile, name );
		player.setCharName( name );
		std::string age; std::getline( loadFile, age );
		player.setCharAge( stoi( age ) );
		std::string sex; std::getline( loadFile, sex );
		player.setCharSex( sex );
		std::string xp; std::getline( loadFile, xp );
		player.setXp( stoi( xp ) );
		std::string health; std::getline( loadFile, health );
		player.setHealth( stoi( health ) );
		std::string strength; std::getline( loadFile, strength );
		player.setStrength( stoi( strength ) );
		std::string intelligence; std::getline( loadFile, intelligence );
		player.setIntelligence( stoi( intelligence ) );
		std::string money; std::getline( loadFile, money );
		player.setMoney( stoi( money ) );
		std::string curLoc; std::getline( loadFile, curLoc );
		player.setCurrentLocation( curLoc );
		std::string curFile; std::getline( loadFile, curFile );
		player.setCurFile( curFile );
		std::string prevLocs; std::getline( loadFile, prevLocs );
		while( !prevLocs.empty() )
		{
			player.addPrevLocation( prevLocs.substr( 0, prevLocs.find( ',' ) ) );
			prevLocs.erase( 0, prevLocs.find( ',' ) + 1 );
		}
		std::string equippedWep; std::getline( loadFile, equippedWep );
		player.setEquippedWeapon( equippedWep );

		std::string itemCond; std::getline( loadFile, itemCond );
		while( itemCond != "*" )
		{
			player.addItemCond( itemCond );
			std::getline( loadFile, itemCond );
		}
		std::string locCond; std::getline( loadFile, locCond );
		while( locCond != "*" )
		{
			player.addLocCond( locCond );
			std::getline( loadFile, locCond );
		}

		std::string itemName; std::getline( loadFile, itemName );
		while( itemName != "*" )
		{
			player.addItem( itemName );
			std::string itemUses; std::getline( loadFile, itemUses );
			itemUses.erase( itemUses.begin() );
			player.getInventory().items[ itemName ].uses = std::stoi( itemUses );
			std::getline( loadFile, itemName );
		}

		std::string weaponName; std::getline( loadFile, weaponName );
		while( weaponName != "*" )
		{
			player.addWeapon( weaponName );
			std::getline( loadFile, weaponName );
		}

		std::string fileNameCache; std::getline( loadFile, fileNameCache );
		while( fileNameCache.find( '#' ) != std::string::npos )
		{
			auto loc = fileNameCache.substr( 0, fileNameCache.find( ',' ) );
			auto file = fileNameCache.substr( fileNameCache.find( ',' ) + 1, fileNameCache.find( '#' ) - ( fileNameCache.find( ',' ) + 1 ) );
			::locationNewFileCache[ loc ] = file;
			fileNameCache.erase( 0, fileNameCache.find( '#' ) + 1 );
		}

		loadFile.close();
		return true;
	}
	logError( "Couldn't load file " + fileName + "!" );
	return false;
}
bool deleteSaveGame( const std::string& fileName )
{
	if( std::remove( ( "game/saves/" + fileName ).c_str() ) != 0 )
		return false;

	std::ifstream gamesaves( "game/saves/gamesaves.txt" );
	if( gamesaves.is_open() )
	{
		std::string fileContents;
		for( std::string temp; std::getline( gamesaves, temp ); )
		{
			if( !temp.empty() )
				fileContents.append( temp + "\n" );
		}
		std::ofstream tempFile( "game/saves/gamesaves_temp.txt" );
		if( tempFile.is_open() )
		{
			tempFile << fileContents;
			tempFile.close();
		}
		else
		{
			logError( "Couldn't open gamesaves_temp.txt!" );
			return false;
		}

		std::ifstream gamesaves_temp( "game/saves/gamesaves_temp.txt" );
		std::ofstream gamesaves_temp2( "game/saves/gamesaves_temp2.txt" );
		if( gamesaves_temp.is_open() && gamesaves_temp2.is_open() )
		{
			for( std::string temp; std::getline( gamesaves_temp, temp ); )
			{
				if( !temp.empty() && temp.find( fileName ) == std::string::npos )
					gamesaves_temp2 << temp << std::endl;
				else
				{
					std::getline( gamesaves_temp, temp );
					std::getline( gamesaves_temp, temp );
					std::getline( gamesaves_temp, temp );
				}
			}
			gamesaves_temp.close();
			gamesaves_temp2.close();
		}
		else
		{
			logError( "Couldn't open gamesaves_temp.txt & gamesaves_temp2.txt!" );
			return false;
		}

		gamesaves.close();
		if( std::remove( "game/saves/gamesaves.txt" ) != 0 )
		{
			logError( "Couldn't delete gamesaves.txt!" );
			return false;
		}

		if( std::remove( "game/saves/gamesaves_temp.txt" ) != 0 )
		{
			logError( "Couldn't delete gamesaves_temp.txt!" );
			return false;
		}

		if( std::rename( "game/saves/gamesaves_temp2.txt", "game/saves/gamesaves.txt" ) != 0 )
		{
			logError( "Couldn't rename gamesaves_temp2.txt to gamesaves.txt!" );
			return false;
		}
	}
	else
	{
		logError( "Couldn't open gamesaves.txt!" );
		return false;
	}

	return true;
}
void gameOver( Console& console, Player& player )
{
	// TODO: Implement maybe
	player.setGameOver( true );

	console.clearScreen();
	std::cout << "Game Over lol\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
	console.getEnter();
}

Location loadLocationData( const std::string& location, const std::string& filename, const bool overRideData, const bool dataFile )
{
	if( ::locationCache.count( location ) && overRideData == false ) // If we already parsed this location...
		return ::locationCache[ location ]; // Return cached data.
	else
	{
		auto filePath = std::string( "game/data/" );
		if( !dataFile )
			filePath.append( "locations/" );
		std::ifstream locFile( filePath + filename );
		if( locFile.is_open() )
		{
			bool foundLoc = false;
			for( std::string line; std::getline(locFile, line); )
			{
				Location parsedLocation;
				if( line.find( "[" + location + "]" ) != std::string::npos )
				{
					foundLoc = true;

					// Parse optionsname/next filename.
					std::string optsName; std::getline( locFile, optsName );
					if( optsName.find( ".txt" ) != std::string::npos )
						::locationNewFileCache[ location ] = optsName;
					else
						parsedLocation.optionsName = optsName;

					// Parse display name.
					std::string dispName; std::getline( locFile, dispName );
					parsedLocation.displayName = dispName;

					// Parse location text.
					std::string locText;
					for( std::string scanLine; ( (scanLine.empty()) ? true : scanLine.back() != '}' ) && std::getline( locFile, scanLine ); )
					{
						if( !scanLine.empty() && scanLine.front() == '{' )
							locText = scanLine;
						else
							locText.append( "\n" + scanLine );
					}
					if( locText.size() >= 2 )
					{
						locText.erase( locText.begin() ); // Remove { at beginning.
						locText.pop_back(); // Remove } at end.
					}
					parsedLocation.locationText = locText;

					// Parse rest.
					bool endReached = false;
					for( std::string scanLine; !endReached && std::getline( locFile, scanLine ); )
					{
						if( !scanLine.empty() )
						{
							switch( scanLine.front() )
							{
							case '*': endReached = true; break; // END
							case '#': // OPTION / DEFAULT OPTION
								if( scanLine.at( 1 ) == '"' ) // OPTION
								{
									scanLine.erase( 0, 2 );
									auto optionName = scanLine.substr( 0, scanLine.find( '\"' ) );
									parsedLocation.options.insert( optionName );
									scanLine.erase( 0, scanLine.find( '\"' ) + 2 );

									auto optionResultLoc = scanLine.substr( 0, scanLine.find( ';' ) );
									parsedLocation.resultLoc[ optionName ] = optionResultLoc;
									scanLine.erase( 0, scanLine.find( ';' ) + 1 );

									if( !scanLine.empty() ) // if the option has conditions...
									{
										auto condType = scanLine.substr( 0, scanLine.find( ':' ) );
										scanLine.erase( 0, scanLine.find( ':' ) + 2 );
										if( condType == "loccond" || condType == "itemcond" || condType == "statcond" )
											parsedLocation.conds[ optionName ] = scanLine;
										else
											logError( "Invalid location data syntax in " + filename + "!", "Invalid condition" );
									}
								}
								else if( scanLine.at( 1 ) == '(' ) // DEFAULT OPTION
								{
									scanLine.erase( 0, 2 );
									parsedLocation.defaultOption = scanLine.substr( 0, scanLine.find( ')' ) );
								}
								else
									logError( "Invalid location data syntax in " + filename + "!", "Neither option nor default option specified" );
								break;
							case '+': // ACTION (ITEM / STAT / CONDITION)
								scanLine.erase( scanLine.begin() );
								auto action = scanLine.substr( 0, scanLine.find( ':' ) );
								scanLine.erase( 0, scanLine.find( ':' ) + 2 );
								auto value = scanLine;

								if( action == "item" )
									parsedLocation.addItems.push_back( value );
								else if( action == "remove_item" )
									parsedLocation.removeItems.push_back( value );
								else if( action == "weapon" )
									parsedLocation.addWeapons.push_back( value );
								else if( action == "remove_weapon" )
									parsedLocation.removeWeapons.push_back( value );
								else if( action == "itemcond" || action == "loccond" )
									parsedLocation.addConds.push_back( value );
								else if( action == "battle" )
								{
									auto enemyName = value.substr( 0, value.find( ' ' ) );
									value.erase( 0, value.find( ' ' ) + 1 );
									auto nEnemies = std::stoi( value.substr( 0, value.find( ',' ) ) );
									value.erase( 0, value.find( ',' ) + 1 );
									auto level = std::stoi( value );
									for( auto i = 0; i < nEnemies; ++i )
									{
										parsedLocation.battleEnemy.push_back( enemyName );
										parsedLocation.battleLevel.push_back( level );
									}
								}
								else if( action == "battlechance" )
								{
									auto enemyName = value.substr( 0, value.find( ' ' ) );
									value.erase( 0, value.find( ' ' ) + 1 );
									auto chance = std::stoi( value.substr( 0, value.find( ',' ) ) );
									value.erase( 0, value.find( ',' ) + 1 );
									auto level = std::stoi( value );
								
									parsedLocation.battleChanceEnemy.push_back( enemyName );
									parsedLocation.battleChancePercentage.push_back( chance );
									parsedLocation.battleChanceLevel.push_back( level );
								}
								else if( action == "shop" )
									parsedLocation.shop = value;
								else
									parsedLocation.addStats.push_back( action + " " + value ); // Assume it's a stat.
								break;
							}
						}
					}
					::locationCache[ location ] = parsedLocation;
					return ::locationCache[ location ];
				}
			}
			locFile.close();
			//if( !foundLoc && filename != "gameitemdata.txt" )
			//	logError( "Location [" + location + "] not found in " + filename + "!" );
		}
		else
			logError( "Unable to open location data file " + filename + "!" );
	}
	return Location();
}
ItemConsumable loadItemData( const std::string& item, const std::string& filename )
{
	if( ::itemCache.count( item ) ) // If we already parsed this item...
		return ::itemCache[ item ]; // Return cached data.
	else
	{
		std::ifstream itemFile( "game/data/" + filename );
		if( itemFile.is_open() )
		{
			bool foundItem = false;
			for( std::string line; getline( itemFile, line ); )
			{
				if( line.find( "[" + item + "]" ) != std::string::npos )
				{
					foundItem = true;
					ItemConsumable parsedItem;
					parsedItem.codeName = item;
					parsedItem.isBattleItem = false;

					bool endReached = false;
					for( std::string scanLine; !endReached && std::getline( itemFile, scanLine ); )
					{
						if( !scanLine.empty() )
						{
							switch( scanLine.front() )
							{
							case '*': endReached = true; break; // END
							case '#':
								scanLine.erase( scanLine.begin() );
								auto type = scanLine.substr( 0, scanLine.find( ' ' ) );
								scanLine.erase( 0, scanLine.find( ' ' ) + 2 );
								auto value = scanLine.substr( 0, scanLine.find( '\"' ) );

								if( type == "prefix" )
									parsedItem.prefix = value;
								else if( type == "name" )
									parsedItem.name = value;
								else if( type == "uses" )
									parsedItem.uses = std::stoi( value );
								else if( type == "weight" )
									parsedItem.weight = std::stof( value );
								else if( type == "worth" )
									parsedItem.worth = std::stoi( value );
								else if( type == "healthCond" )
									parsedItem.healthCond = std::stoi( value );
								else if( type == "strengthCond" )
									parsedItem.strengthCond = std::stoi( value );
								else if( type == "intelCond" )
									parsedItem.intelligenceCond = std::stoi( value );
								else if( type == "battleItem" )
									parsedItem.isBattleItem = true;
								else if( type == "damageType" )
									parsedItem.damageType = value;
								else if( type == "damage" )
									parsedItem.damage = std::stoi( value );
								else if( type == "desc" )
								{
									auto descText = value;
									while( scanLine.back() != '}' && std::getline( itemFile, scanLine ) )
										descText.append( "\n" + scanLine );
									if( !descText.empty() )
										descText.pop_back(); // Remove } at end.
									parsedItem.desc = descText;
								}
								break;
							}
						}
					}
					::itemCache[ item ] = parsedItem;
					return ::itemCache[ item ];
				}
			}
			itemFile.close();
			if( !foundItem )
				logError( "Item data for [" + item + "] not found in " + filename + "!" );
		}
		else
			logError( "Unable to open item data file " + filename + "!" );
	}
	return ItemConsumable();
}
Weapon loadWeaponData( const std::string& weapon, const std::string& filename )
{
	if( ::weaponCache.count( weapon ) ) // If we already parsed this weapon...
		return ::weaponCache[ weapon ]; // Return cached data.
	else
	{
		std::ifstream wepFile( "game/data/" + filename );
		if( wepFile.is_open() )
		{
			bool foundWep = false;
			for( std::string line; getline( wepFile, line ); )
			{
				if( line.find( "[" + weapon + "]" ) != std::string::npos )
				{
					foundWep = true;
					Weapon parsedWeapon;
					parsedWeapon.codeName = weapon;

					bool endReached = false;
					for( std::string scanLine; !endReached && std::getline( wepFile, scanLine ); )
					{
						if( !scanLine.empty() )
						{
							switch( scanLine.front() )
							{
							case '*': endReached = true; break; // END
							case '#':
								scanLine.erase( scanLine.begin() );
								auto type = scanLine.substr( 0, scanLine.find( ' ' ) );
								scanLine.erase( 0, scanLine.find( ' ' ) + 2 );
								auto value = scanLine.substr( 0, scanLine.find( '\"' ) );
								
								if( type == "prefix" )
									parsedWeapon.prefix = value;
								else if( type == "name" )
									parsedWeapon.name = value;
								else if( type == "damage" )
									parsedWeapon.damage = std::stoi( value );
								else if( type == "weight" )
									parsedWeapon.weight = std::stof( value );
								else if( type == "worth" )
									parsedWeapon.worth = std::stoi( value );
								else if( type == "healthCond" )
									parsedWeapon.healthCond = std::stoi( value );
								else if( type == "strengthCond" )
									parsedWeapon.strengthCond = std::stoi( value );
								else if( type == "intelCond" )
									parsedWeapon.intelligenceCond = std::stoi( value );
								else if( type == "desc" )
								{
									auto descText = value;
									while( scanLine.back() != '}' && std::getline( wepFile, scanLine ) )
										descText.append( "\n" + scanLine );
									if( !descText.empty() )
										descText.pop_back(); // Remove } at end.
									parsedWeapon.desc = descText;
								}
								break;
							}
						}
					}
					::weaponCache[ weapon ] = parsedWeapon;
					return ::weaponCache[ weapon ];
				}
			}
			wepFile.close();
			//if( !foundWep )
			//	logError( "Weapon data for [" + weapon + "] not found in " + filename + "!" );
		}
		else
			logError( "Unable to open item data file " + filename + "!" );
	}
	return Weapon();
}
Enemy loadEnemyData( const std::string& enemy, const std::string& filename )
{
	if( ::enemyCache.count( enemy ) ) // If we already parsed this enemy...
		return ::enemyCache[ enemy ]; // Return cached data.
	else
	{
		std::ifstream enemyFile( "game/data/" + filename );
		if( enemyFile.is_open() )
		{
			bool foundEnemy = false;
			for( std::string line; getline( enemyFile, line ); )
			{
				if( line.find( "[" + enemy + "]" ) != std::string::npos )
				{
					foundEnemy = true;
					Enemy parsedEnemy;

					bool endReached = false;
					for( std::string scanLine; !endReached && std::getline( enemyFile, scanLine ); )
					{
						if( !scanLine.empty() )
						{
							switch( scanLine.front() )
							{
							case '*': endReached = true; break; // END
							case '#':
								scanLine.erase( scanLine.begin() );
								auto type = scanLine.substr( 0, scanLine.find( ' ' ) );
								scanLine.erase( 0, scanLine.find( ' ' ) + 2 );
								auto value = scanLine.substr( 0, scanLine.find( '\"' ) );

								if( type == "prefix" )
									parsedEnemy.prefix = value;
								else if( type == "name" )
									parsedEnemy.name = value;
								else if( type == "weakness" )
									parsedEnemy.damageTypeResistance = value;
								else if( type == "resistance" )
									parsedEnemy.damageTypeResistance = value;
								else if( type == "defhealth" )
									parsedEnemy.health = std::stoi( value );
								else if( type == "defdamage" )
									parsedEnemy.damage = std::stoi( value );
								else if( type == "runchance" )
									parsedEnemy.runChance = std::stoi( value );
								else if( type == "reward" )
									parsedEnemy.reward = std::stoi( value );
								else if( type == "desc" )
								{
									std::string descText = value;
									while( scanLine.back() != '}' && std::getline( enemyFile, scanLine ) )
										descText.append( "\n" + scanLine );
									if( !descText.empty() )
										descText.pop_back(); // Remove } at end.
									parsedEnemy.desc = descText;
								}
								else if( type == "img" )
								{
									std::string image;
									for( std::string imgLine; std::getline( enemyFile, imgLine ) && imgLine != "========================================"; )
										image.append( "\n" + imgLine );
									parsedEnemy.image = image;
								}
								else if( type == "hitimg" )
								{
									std::string image;
									for( std::string imgLine; std::getline( enemyFile, imgLine ) && imgLine != "========================================"; )
										image.append( "\n" + imgLine );
									parsedEnemy.hitImage = image;
								}
								break;
							}
						}
					}
					::enemyCache[ enemy ] = parsedEnemy;
					return ::enemyCache[ enemy ];
				}
			}
			enemyFile.close();
			if( !foundEnemy )
				logError( "Enemy data for [" + enemy + "] not found in " + filename + "!" );
		}
		else
			logError( "Unable to open enemy data file " + filename + "!" );
	}
	return Enemy();
}
Shop loadShopData( const std::string& shop, const std::string& filename )
{
	if( ::shopCache.count( shop ) ) // If we already parsed this shop...
		return ::shopCache[ shop ]; // Return cached data.
	else
	{
		std::ifstream shopFile( "game/data/" + filename );
		if( shopFile.is_open() )
		{
			bool foundShop = false;
			for( std::string line; getline( shopFile, line ); )
			{
				if( line.find( "[" + shop + "]" ) != std::string::npos )
				{
					foundShop = true;
					Shop parsedShop;

					bool endReached = false;
					for( std::string scanLine; !endReached && std::getline( shopFile, scanLine ); )
					{
						if( !scanLine.empty() )
						{
							switch( scanLine.front() )
							{
							case '*': endReached = true; break; // END
							case '#':
								scanLine.erase( scanLine.begin() );
								auto type = scanLine.substr( 0, scanLine.find( ' ' ) );
								scanLine.erase( 0, scanLine.find( ' ' ) + 2 );
								auto value = scanLine.substr( 0, scanLine.find( '\"' ) );

								if( type == "name" )
									parsedShop.name = value;
								else if( type == "item" )
								{
									auto itemName = value.substr( 0, value.find( ',' ) );
									value.erase( 0, value.find( ',' ) + 1 );
									auto itemAmount = std::stoi( value.substr( 0, value.find( ',' ) ) );
									value.erase( 0, value.find( ',' ) + 1 );
									auto itemPrice = value;

									auto item = loadItemData( itemName, "gameitemdata.txt" );
									if( itemPrice == "def" )
										itemPrice = std::to_string( item.worth );

									parsedShop.items.push_back( item );
									parsedShop.nItems.push_back( itemAmount );
									parsedShop.itemPrices.push_back( std::stoi( itemPrice ) );
								}
								else if( type == "weapon" )
								{
									auto weaponName = value.substr( 0, value.find( ',' ) );
									value.erase( 0, value.find( ',' ) + 1 );
									auto weaponAmount = std::stoi( value.substr( 0, value.find( ',' ) ) );
									value.erase( 0, value.find( ',' ) + 1 );
									auto weaponPrice = value;

									auto weapon = loadWeaponData( weaponName, "gameitemdata.txt" );
									if( weaponPrice == "def" )
										weaponPrice = std::to_string( weapon.worth );

									parsedShop.weapons.push_back( weapon );
									parsedShop.nWeapons.push_back( weaponAmount );
									parsedShop.weaponPrices.push_back( std::stoi( weaponPrice ) );
								}
								else if( type == "buymultiplier" )
									parsedShop.buyMultiplier = std::stod( value );
								break;
							}
						}
					}
					::shopCache[ shop ] = parsedShop;
					return ::shopCache[ shop ];
				}
			}
			shopFile.close();
			if( !foundShop )
				logError( "Shop data for [" + shop + "] not found in " + filename + "!" );
		}
		else
			logError( "Unable to open shop data file " + filename + "!" );
	}
	return Shop();
}

int displayLocationText( const std::string& location, const std::string& filename, const int delay, Console& console, Player& player, const bool dataFile )
{
	auto loc = loadLocationData( location, filename, false, dataFile );

	for( auto i = loc.locationText.find( "_CHARAGE_" ); i != std::string::npos; i = loc.locationText.find( "_CHARAGE_" ) )
		loc.locationText.replace( i, 9, std::to_string( player.getCharAge() ) );
	for( auto i = loc.locationText.find( "_CHARSEX_" ); i != std::string::npos; i = loc.locationText.find( "_CHARSEX_" ) )
		loc.locationText.replace( i, 9, player.getCharSex() == "male" ? "man" : "woman" );
	for( auto i = loc.locationText.find( "_CHARNAME_" ); i != std::string::npos; i = loc.locationText.find( "_CHARNAME_" ) )
		loc.locationText.replace( i, 10, player.getCharName() );
	for( auto i = loc.locationText.find( "_IF_" ); i != std::string::npos; i = loc.locationText.find( "_IF_" ) )
	{
		auto line = loc.locationText.substr( i, loc.locationText.find( '\n', i ) );
		auto fullCond = line.substr( 4, line.find( ':' ) - 4 );
		auto condName = fullCond.substr( 0, fullCond.find( ' ' ) );

		std::string foundCond;
		if( ( player.findLocCond( condName, &foundCond ) && foundCond == fullCond ) || ( player.findItemCond( condName, &foundCond ) && foundCond == fullCond ) )
			loc.locationText.erase( i, line.find( ':' ) + 2 );
		else
			loc.locationText.erase( i, line.length() + 1 );
	}

	auto nLines = 1;
	for( auto i = loc.locationText.find( '\n' ); i != std::string::npos; i = loc.locationText.find( '\n', i + 1 ) )
		++nLines;

	console.printSlow( loc.locationText, delay, true );
	return nLines;
}
std::string handleLocationInput( const std::string& location, const std::string& filename, const short line, Console& console, Player& player, RNG& rng ) // returns the location to go to next
{
	auto loc = loadLocationData( location, filename, false );

	// Get all valid options.
	auto goodOptions = loc.options;
	for( const auto& option : getStandardCommands() )
		goodOptions.insert( option );
	const auto tempGoodOptions = goodOptions;
	for( const auto& option : tempGoodOptions )
	{
		if( option.find( ' ' ) != std::string::npos )
		{
			auto command = option.substr( 0, option.find( ' ' ) );
			auto target = option.substr( option.find( ' ' ) + 1, std::string::npos );
			auto actionSynonyms = getSynonyms( command );
			if( !actionSynonyms.empty() )
			{
				for( const auto& synonym : actionSynonyms )
				{
					goodOptions.insert( synonym + " " + target );
					loc.resultLoc[ synonym + " " + target ] = loc.resultLoc[ option ];
					if( loc.conds.count( option ) )
						loc.conds[ synonym + " " + target ] = loc.conds[ option ];
				}
			}
		}
		else
		{
			auto optionSynonyms = getSynonyms( option );
			if( !optionSynonyms.empty() )
			{
				for( const auto& synonym : optionSynonyms )
				{
					goodOptions.insert( synonym );
					loc.resultLoc[ synonym ] = loc.resultLoc[ option ];
					if( loc.conds.count( option ) )
						loc.conds[ synonym ] = loc.conds[ option ];
				}
			}
		}
	}

	std::string nextLoc;
	while( nextLoc.empty() )
	{
		// Get input.
		std::string input;
		if( !getInputUntilStrings( input, goodOptions, 0, true, true, false, false, true, line, console ) )
		{
			console.setCursorPosition( 0, line - 2 );
			std::cout << "Invalid command.";
			console.setCursorPosition( 0, line );
			while( !_kbhit() );
			console.setCursorPosition( 0, line - 2 );
			std::cout << "                                                                                                                               ";
			console.setCursorPosition( 0, line );
		}
		else // If input matched one of the valid commands...
		{
			auto singleLineScreenMsg = []( const std::string& text, Console& console )
			{
				console.clearScreen();
				std::cout << text << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
				console.getEnter();
			};
			auto wasStandardCommand = [singleLineScreenMsg, input, loc, filename, location]( std::string& nextLocResult, Console& console, Player& player, RNG& rng )
			{
				const auto sameLoc = ( location.front() == '_' ) ? location.substr( 1, std::string::npos ) : location;

				if( input == "info" || input == "description" || input == "more info" )
				{
					auto result = location;
					if( result.front() == '_' )
						result.erase( 0, 1 );

					console.clearScreen();
					std::cout << "Location Description:\n\n\n";
					auto nLines = displayLocationText( result, filename, 1, console, player );
					for( auto i = 0; i < 45 - nLines; ++i )
						std::cout << '\n';
					console.getEnter();

					nextLocResult = result;
					return true;
				}
				else if( input == "money" || input == "balance" || input == "cash" || input == "dollars" )
				{
					singleLineScreenMsg( "You have $" + std::to_string( player.getMoney() ) + '.', console );
					nextLocResult = sameLoc;
					return true;
				}
				else if( input == "location" || input == "loc" )
				{
					singleLineScreenMsg( "You are in " + loc.displayName + '.', console );
					nextLocResult = sameLoc;
					return true;
				}
				else if( input == "time" )
				{
					singleLineScreenMsg( "The time is: 13:37.", console );
					nextLocResult = sameLoc;
					return true;
				}
				else if( input == "stats" || input == "character" )
				{
					showStatMenu( console, player );
					nextLocResult = sameLoc;
					return true;
				}
				else if( input == "inventory" || input == "items" || input == "backpack" || input == "inv" || input == "open inventory" || input == "open backpack" )
				{
					std::string resultInv;
					if( execInventory( resultInv, false, console, player ) )
					{
						std::string resultItem;
						if( player.useItem( resultInv, resultItem ) )
						{
							if( !player.hasBeenLocation( location ) )
								player.addPrevLocation( location );
							nextLocResult = resultItem;
							return true;
						}
						else
							logError( "Tried to use an item that wasn't found in inventory!" );
					}
					else
					{
						nextLocResult = sameLoc;
						return true;
					}
				}
				else if( input == "help" || input == "commands" )
				{
					helpMenu( console );
					nextLocResult = sameLoc;
					return true;
				}
				else if( input == "save" )
				{
					saveMenu( console, player );
					nextLocResult = sameLoc;
					return true;
				}
				else if( input == "debugbattle" && _GAMEVERSION == "debug" )
				{
					execBattle( "Ogre", 5, console, player, rng );
				}
				else if( input == "debugshop" && _GAMEVERSION == "debug" )
				{
					execShop( "travelSales", console, player );
				}
				return false;
			};

			if( !wasStandardCommand( nextLoc, console, player, rng ) ) // If this is not a standard command...
			{
				if( !loc.conds.count( input ) ) // If this option doesn't require any specific conditions to be met...
					nextLoc = loc.resultLoc.at( input ); // Accept it.
				else
				{
					// Find out whether or not the player meets the required condition(s).
					auto wasValidCond = [line, player]( const std::string& fullCond, Console& console )
					{
						auto condFailMsg = [line]( const std::string& text, Console& console )
						{
							console.setCursorPosition( 0, line - 2 );
							std::cout << text;
							console.setCursorPosition( 0, line );
							while( !_kbhit() );
							console.setCursorPosition( 0, line - 2 );
							std::cout << "                                                                                                                               ";
							console.setCursorPosition( 0, line );
						};
						auto loadMsgFromLocationFile = [player, condFailMsg]( const std::string& locationName, Console& console )
						{
							auto loc2 = loadLocationData( locationName, player.getCurFile(), false );
							for( auto i = loc2.locationText.find( "_IF_" ); i != std::string::npos; i = loc2.locationText.find( "_IF_" ) )
							{
								auto line = loc2.locationText.substr( i, loc2.locationText.find( '\n', i ) );
								auto fullCond = line.substr( 4, line.find( ':' ) - 4 );
								auto condName = fullCond.substr( 0, fullCond.find( ' ' ) );

								std::string foundCond;
								if( ( player.findLocCond( condName, &foundCond ) && foundCond == fullCond ) || ( player.findItemCond( condName, &foundCond ) && foundCond == fullCond ) )
									loc2.locationText.erase( i, line.find( ':' ) + 2 );
								else
									loc2.locationText.erase( i, line.length() + 1 );
							}
							condFailMsg( loc2.locationText, console );
						};
						auto statMsg = [condFailMsg]( const std::string& msg, const int value, Console& console )
						{
							condFailMsg( msg + std::to_string( value ) + " to do that!", console );
						};

 						if( fullCond == "isMale" )
						{
							if( player.getCharSex() == "male" )
								return true;
							else
								condFailMsg( "You need to be male to do that!", console );
						}
						else if( fullCond == "isFemale" )
						{
							if( player.getCharSex() == "female" )
								return true;
							else
								condFailMsg( "You need to be female to do that!", console );
						}
						else if( fullCond.find( ' ' ) != std::string::npos )
						{
							const auto condName = fullCond.substr( 0, fullCond.find( ' ' ) );
							const auto condSymbol = fullCond.at( fullCond.find( ' ' ) + 1 );
							const auto condValue = std::stoi( fullCond.substr( fullCond.find( ' ' ) + 1, std::string::npos ) );

							if( condName == "strength" )
							{
								if( condSymbol == '<' )
								{
									if( player.getStrength() < condValue )
										return true;
									else
										statMsg( "Your strength must be lower than ", condValue, console );
								}
								else if( condSymbol == '>' )
								{
									if( player.getStrength() > condValue )
										return true;
									else
										statMsg( "Your strength must be higher than ", condValue, console );
								}
							}
							else if( condName == "health" )
							{
								if( condSymbol == '<' )
								{
									if( player.getHealth() < condValue )
										return true;
									else
										statMsg( "Your health must be lower than ", condValue, console );
								}
								else if( condSymbol == '>' )
								{
									if( player.getHealth() > condValue )
										return true;
									else
										statMsg( "Your health must be higher than ", condValue, console );
								}
							}
							else if( condName == "intelligence" )
							{
								if( condSymbol == '<' )
								{
									if( player.getIntelligence() < condValue )
										return true;
									else
										statMsg( "Your intelligence must be lower than ", condValue, console );
								}
								else if( condSymbol == '>' )
								{
									if( player.getIntelligence() > condValue )
										return true;
									else
										statMsg( "Your intelligence must be higher than ", condValue, console );
								}
							}
							else if( condName == "level" )
							{
								if( condSymbol == '<' )
								{
									if( player.getLevel() < condValue )
										return true;
									else
										statMsg( "Your level must be lower than ", condValue, console );
								}
								else if( condSymbol == '>' )
								{
									if( player.getLevel() > condValue )
										return true;
									else
										statMsg( "Your level must be higher than ", condValue, console );
								}
							}
							else if( condName == "age" )
							{
								if( condSymbol == '<' )
								{
									if( player.getCharAge() < condValue )
										return true;
									else
										statMsg( "Your age must be lower than ", condValue, console );
								}
								else if( condSymbol == '>' )
								{
									if( player.getCharAge() > condValue )
										return true;
									else
										statMsg( "Your age must be higher than ", condValue, console );
								}
							}

							std::string foundCond;
							if( ( player.findLocCond( condName, &foundCond ) && foundCond == fullCond ) || ( player.findItemCond( condName, &foundCond ) && foundCond == fullCond ) )
								return true;
							else
								loadMsgFromLocationFile( condName, console );
						}
						return false;
					};

					if( wasValidCond( loc.conds.at( input ), console ) )
						nextLoc = loc.resultLoc.at( input );
				}
			}
		}
	}
	return nextLoc;
}
std::string execLocation( const std::string& location, const std::string& filename, Console& console, Player& player, RNG& rng, const bool dataFile )
{
	// Load location.
	auto loc = (dataFile) ? loadLocationData( location, filename, false, true ) : loadLocationData( location, filename, false );

	// New file?
	if( ::locationNewFileCache.count( location ) && !player.hasBeenLocation( location ) )
	{
		player.initializeConds( ::locationNewFileCache.at( location ) );
		loadLocationData( location, ::locationNewFileCache.at( location ), true ); // Override location cache data.
		player.setCurFile( ::locationNewFileCache.at( location ) );
		player.addPrevLocation( location );
		return location;
	}

	// If there is no optionsName, assume location failed to load.
	if( loc.optionsName.empty() )
	{
		// Try again, this time without the first character.
		if( location.find( '_' ) != std::string::npos )
			return location.substr( 1, std::string::npos );
		else
		{
			logError( "Location [" + location + "] in file " + filename + " didn't load correctly!", "No optionsName found!", true );
			console.getEnter();
			return "_FAIL";
		}
	}

	// If the location's optionsName does not match its actual name, replace its options, displayName and battleChance with the ones in the optionsName's.
	if( loc.optionsName == "curLocation" )
		loc.displayName = loadLocationData( player.getCurrentLocation(), filename, false ).displayName;
	else if( loc.optionsName != location )
	{
		auto tempLoc = loadLocationData( loc.optionsName, filename, false );
		loc.conds = tempLoc.conds;
		loc.defaultOption = tempLoc.defaultOption;
		loc.displayName = tempLoc.displayName;
		loc.options = tempLoc.options;
		loc.resultLoc = tempLoc.resultLoc;
		loc.battleChanceEnemy = tempLoc.battleChanceEnemy;
		loc.battleChanceLevel = tempLoc.battleChanceLevel;
		loc.battleChancePercentage = tempLoc.battleChancePercentage;
	}

	// If the optionsName matches the current location, set player's current location.
	if( loc.optionsName == location )
		player.setCurrentLocation( location );
	else
	{
		// Double-check in case the optionsName started with a '_'.
		const auto optionsNameWithoutUnderscore = ( loc.optionsName.front() == '_' ) ? loc.optionsName.substr( 1, std::string::npos ) : loc.optionsName;
		if( optionsNameWithoutUnderscore == location )
			player.setCurrentLocation( location );
	}

	// Add items?
	for( const auto& item : loc.addItems )
		player.addItem( item );
	// Remove items?
	for( const auto& item : loc.removeItems )
		player.removeItem( item );
	// Add weapons?
	for( const auto& weapon : loc.addWeapons )
		player.addWeapon( weapon );
	// Remove weapons?
	for( const auto& weapon : loc.removeWeapons )
		player.removeWeapon( weapon );
	// Add conditions?
	for( const auto& cond : loc.addConds )
	{
		const auto condName = cond.substr( 0, cond.find( ' ' ) );
		std::string result;
		if( player.findLocCond( condName, &result ) )
		{
			player.removeLocCond( result );
			player.addLocCond( cond );
		}
		else if( player.findItemCond( condName, &result ) )
		{
			player.removeItemCond( result );
			player.addItemCond( cond );
		}
		else
			player.addItemCond( cond );
	}
	// Add stats?
	std::map<std::string, int> addedStats;
	for( const auto& stat : loc.addStats )
	{
		auto statName = stat.substr( 0, stat.find( ' ' ) );
		auto value = std::stoi( stat.substr( stat.find( ' ' ) + 1, std::string::npos ) );

		if( statName == "health" )
			player.addHealth( value );
		else if( statName == "strength" )
			player.addStrength( value );
		else if( statName == "intelligence" )
			player.addIntelligence( value );
		else if( statName == "xp" )
			player.addXp( value );

		addedStats[ statName ] = value;
	}
	// Battle random enemy?
	bool doneBattle = false;
	for( auto i = decltype( loc.battleChanceEnemy.size() ){ 0 }; !doneBattle && i < loc.battleChanceEnemy.size(); ++i )
	{
		if( rng.getRandomInt( 0, 100 ) <= loc.battleChancePercentage.at( i ) )
		{
			console.clearScreen();
			std::cout << "A wild " << loc.battleChanceEnemy.at( i ) << " appears! (LEVEL " << loc.battleChanceLevel.at( i ) << ")\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
			console.getEnter();

			execBattle( loc.battleChanceEnemy.at( i ), loc.battleChanceLevel.at( i ), console, player, rng );
			doneBattle = true;
		}
	}

	// Print location text.
	console.clearScreen();
	auto nLines = displayLocationText( location, filename, 40, console, player );

	// Print newlines until we reach the bottom.
	constexpr int screenHeight = 50;
	for( auto i = screenHeight - 2 - nLines; i > 0; --i )
		_putch( '\n' );

	// Battle?
	for( auto i = decltype( loc.battleEnemy.size() ){ 0 }; i < loc.battleEnemy.size(); ++i )
	{
		console.getEnter();
		execBattle( loc.battleEnemy.at( i ), loc.battleLevel.at( i ), console, player, rng );
	}

	std::string nextLoc;
	if( loc.battleEnemy.empty() )
	{
		// If there was no battle, get player input.
		if( loc.defaultOption.empty() )
			nextLoc = handleLocationInput( loc.optionsName, filename, screenHeight - 1, console, player, rng );
		else if( loc.defaultOption == "curLocation" )
		{
			nextLoc = player.getCurrentLocation();
			console.getEnter();
		}
		else
		{
			nextLoc = loc.defaultOption;
			console.getEnter();
		}

		// If the player has already been to the new location before, insert a '_' in front of the new location name.
		if( nextLoc.front() != '_' && player.hasBeenLocation( nextLoc ) && nextLoc.substr( 0, 3 ) != "use" )
			nextLoc.insert( nextLoc.begin(), '_' );
	}
	else
		nextLoc = (location.front() == '_') ? location : "_" + location; // If there was a battle, go back to the current location.

	// Shop?
	if( !loc.shop.empty() )
	{
		execShop( loc.shop, console, player );
		nextLoc = (location.front() == '_') ? location : "_" + location; // Go back to the current location.
	}

	// If the player hasn't already been here, add the location to the player's list of previously visited locations.
	if( !player.hasBeenLocation( location ) )
		player.addPrevLocation( location );

	// Show any stat changes to the player.
	for( const auto& stat : addedStats )
		showStatChange( stat.first, stat.second, console );

	return nextLoc;
}

void execShop( const std::string& shop, Console& console, Player& player )
{
	console.clearScreen();
	std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "                                                              ==                                                               " << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "-------------------------------+-------------------------------+---------------------------------------------------------------" << '\n';
	std::cout << "      BUY (Shop's items)       |       SELL (Your items)       |                             INFO                              " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |        +---------------------------------------------+        " << '\n'; //7       (5, 7) (37, 7)
	std::cout << "                               |                               |        |X                                           X|        " << '\n'; //8       (96, 8)
	std::cout << "                               |                               |        +---------------------------------------------+        " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n'; //12      (73, 12)
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "                               |                               |                                                               " << '\n';
	std::cout << "-------------------------------+-------------------------------+                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "    Select an item using the arrow keys                        |                                                               " << '\n';
	std::cout << "    and press ENTER to confirm.                                |                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "    Press SPACEBAR to switch between Items / Weapons.          |                                                               " << '\n';
	std::cout << "    Press BACKSPACE to exit the shop.                          |                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "                                                               |                                                               " << '\n';
	std::cout << "                                                               |                                                               ";

	Shop shop1 = loadShopData( shop, "gameshopdata.txt" );
	Sound kaching( L"kaching.wav", L"waveaudio" );
	Sound music;
	//music.loopSound( L"shoptheme.wav" ); // TODO: Make music

	// Print shop name.
	Sleep( 600 );
	std::string nameToPrint( "=" );
	for( const auto& ch : shop1.name )
	{
		nameToPrint.push_back( ch );
		console.setCursorPosition( 63 - static_cast<short>( nameToPrint.length() / 2 ), 2 );
		std::cout << nameToPrint << '=';
		Sleep( 50 );
	}

	struct ShopThings
	{
		std::vector<ItemConsumable> playerItems;
		std::vector<Weapon> playerWeapons;
		std::vector<int> playerItemPrices;
		std::vector<int> playerWeaponPrices;
		std::vector<ItemConsumable> shopItems;
		std::vector<Weapon> shopWeapons;
	};

	auto updateShopThings = []( ShopThings& shopThings, const Shop& shop1, const Player& player )
	{
		// Clear all previous items/weapons/prices.
		shopThings.playerItems.clear();
		shopThings.playerItemPrices.clear();
		shopThings.playerWeapons.clear();
		shopThings.playerWeaponPrices.clear();
		shopThings.shopItems.clear();
		shopThings.shopWeapons.clear();

		// Get all player items/weapons/prices.
		for( const auto& item : player.getInventory().items )
		{
			shopThings.playerItems.push_back( item.second );
			shopThings.playerItemPrices.push_back( static_cast<int>( item.second.worth * shop1.buyMultiplier + 0.5 ) );
		}
		for( const auto& wep : player.getInventory().weapons )
		{
			shopThings.playerWeapons.push_back( wep.second );
			shopThings.playerWeaponPrices.push_back( static_cast<int>( wep.second.worth * shop1.buyMultiplier + 0.5 ) );
		}
		// Get all shop items/weapons.
		for( const auto& item : shop1.items )
			shopThings.shopItems.push_back( item );
		for( const auto& wep : shop1.weapons )
			shopThings.shopWeapons.push_back( wep );
	};
	auto updateShopInterface = []( const ShopThings& shopThings, const Shop& shop1, const Player& player, const int curRow, const int curColumn, const int curType, Console& console )
	{
		// Get equipped weapon.
		auto equippedWeaponNo = -1;
		for( auto i = decltype( shopThings.playerWeapons.size() ){ 0 }; i < shopThings.playerWeapons.size(); ++i )
		{
			if( player.getEquippedWeapon() == shopThings.playerWeapons[ i ].codeName )
				equippedWeaponNo = i;
		}

		// Clear previous item/weapon lists.
		for( auto i = 7; i < 25 + 7; ++i )
		{
			console.setCursorPosition( 1, i );
			std::cout << "                             ";
		}
		for( auto i = 7; i < 25 + 7; i++ )
		{
			console.setCursorPosition( 33, i );
			std::cout << "                             ";
		}

		if( curType == 0 ) // ITEMS
		{
			// Clear previous player weapons.
			for( auto i = 7; i < 25 + 7; ++i )
			{
				console.setCursorPosition( 37, i );
				std::cout << "                         ";
			}
			// Print all player items, their quantity, and their price.
			for( auto i = decltype( shopThings.playerItems.size() ){ 0 }; i < shopThings.playerItems.size(); ++i )
			{
				console.setCursorPosition( 33, 7 + static_cast<short>( i ) );
				std::cout << shopThings.playerItems[ i ].uses;

				console.setCursorPosition( 37, 7 + static_cast<short>( i ) );
				std::cout << shopThings.playerItems[ i ].codeName;
				auto priceLen = std::to_string( shopThings.playerItemPrices[ i ] ).length();
				auto codeNameLen = shopThings.playerItems[ i ].codeName.length();
				for( auto spc = 24 - priceLen - codeNameLen; spc > 0; --spc )
					_putch( ' ' );
				std::cout << '$' << shopThings.playerItemPrices[ i ];
			}
		}
		else // assume curType == 1 (WEAPONS)
		{
			// Clear previous player items.
			for( auto i = 7; i < 25 + 7; ++i )
			{
				console.setCursorPosition( 33, i );
				std::cout << "                             ";
			}
			// Print all player weapons and their price.
			for( auto i = decltype( shopThings.playerWeapons.size() ){ 0 }; i < shopThings.playerWeapons.size(); ++i )
			{
				console.setCursorPosition( 37, 7 + static_cast<short>( i ) );
				const auto name = (i == equippedWeaponNo) ? "[" + shopThings.playerWeapons[ i ].codeName + "]" : shopThings.playerWeapons[ i ].codeName;
				std::cout << name;
				auto priceLen = std::to_string( shopThings.playerWeaponPrices[ i ] ).length();
				auto codeNameLen = name.length();
				for( auto spc = 24 - priceLen - codeNameLen; spc > 0; --spc )
					_putch( ' ' );
				std::cout << '$' << shopThings.playerWeaponPrices[ i ];
			}
		}

		// Print all shop items, their quantity, and their price.
		for( auto i = decltype( shopThings.shopItems.size() ){ 0 }; i < shopThings.shopItems.size(); ++i )
		{
			console.setCursorPosition( 1, 7 + static_cast<short>( i ) );
			std::cout << shop1.nItems[ i ];

			console.setCursorPosition( 5, 7 + static_cast<short>( i ) );
			std::cout << shopThings.shopItems[ i ].codeName;
			auto priceLen = std::to_string( shop1.itemPrices[ i ] ).length();
			auto codeNameLen = shopThings.shopItems[ i ].codeName.length();
			for( auto spc = 24 - priceLen - codeNameLen; spc > 0; --spc )
				_putch( ' ' );
			std::cout << '$' << shop1.itemPrices[ i ];
		}
		// Print all shop weapons, their quantity, and their price (append after the list of items).
		for( auto i = decltype( shopThings.shopWeapons.size() ){ 0 }; i < shopThings.shopWeapons.size(); ++i )
		{
			console.setCursorPosition( 1, 7 + static_cast<short>( i + shopThings.shopItems.size() ) );
			std::cout << shop1.nWeapons[ i ];

			console.setCursorPosition( 5, 7 + static_cast<short>( i + shopThings.shopItems.size() ) );
			std::cout << shopThings.shopWeapons[ i ].codeName;
			auto priceLen = std::to_string( shop1.weaponPrices[ i ] ).length();
			auto codeNameLen = shopThings.shopWeapons[ i ].codeName.length();
			for( auto spc = 24 - priceLen - codeNameLen; spc > 0; --spc )
				_putch( ' ' );
			std::cout << '$' << shop1.weaponPrices[ i ];
		}

		// Print player's current balance
		console.setCursorPosition( 73, 44 );
		std::cout << "Your Money: $" << "        ";
		console.setCursorPosition( 86, 44 );
		std::cout << player.getMoney();
		console.setCursorPosition( 0, 49 );
	};
	auto putNewCursor = []( const ShopThings& shopThings, const int curRow, const int curColumn, const int curType, Console& console )
	{
		// Get length of the currently selected item codeName.
		auto curNameLen = 0;
		if( curColumn == 1 )
		{
			if( curRow <= static_cast<int>( shopThings.shopItems.size() ) )
				curNameLen = 1 + shopThings.shopItems[ curRow - 1 ].codeName.length();
			else if( curRow <= static_cast<int>( shopThings.shopItems.size() + shopThings.shopWeapons.size() ) )
				curNameLen = 1 + shopThings.shopWeapons[ curRow - 1 - shopThings.shopItems.size() ].codeName.length();
		}
		else if( curColumn == 2 )
		{
			if( curType == 0 && curRow <= static_cast<int>( shopThings.playerItems.size() ) )
				curNameLen = 1 + shopThings.playerItems[ curRow - 1 ].codeName.length();
			else if( curType == 1 && curRow <= static_cast<int>( shopThings.playerWeapons.size() ) )
				curNameLen = 1 + shopThings.playerWeapons[ curRow - 1 ].codeName.length();
		}

		// Print new cursor.
		console.setCursorPosition( -27 + ( curColumn * 32 ) + curNameLen, 6 + curRow );
		_putch( '<' );
	};
	auto eraseCursor = []( const ShopThings& shopThings, const int curRow, const int curColumn, const int curType, Console& console )
	{
		// Get length of the currently selected item codeName.
		auto curNameLen = 0;
		if( curColumn == 1 )
		{
			if( curRow <= static_cast<int>( shopThings.shopItems.size() ) )
				curNameLen = 1 + shopThings.shopItems[ curRow - 1 ].codeName.length();
			else if( curRow <= static_cast<int>( shopThings.shopItems.size() + shopThings.shopWeapons.size() ) )
				curNameLen = 1 + shopThings.shopWeapons[ curRow - 1 - shopThings.shopItems.size() ].codeName.length();
		}
		else if( curColumn == 2 )
		{
			if( curType == 0 && curRow <= static_cast<int>( shopThings.playerItems.size() ) )
				curNameLen = 1 + shopThings.playerItems[ curRow - 1 ].codeName.length();
			else if( curType == 1 && curRow <= static_cast<int>( shopThings.playerWeapons.size() ) )
				curNameLen = 1 + shopThings.playerWeapons[ curRow - 1 ].codeName.length();
		}

		// Erase cursor.
		console.setCursorPosition( -27 + ( curColumn * 32 ) + curNameLen, 6 + curRow );
		_putch( ' ' );
	};
	auto updateInfo = []( const ShopThings& shopThings, const Player& player, const int curRow, const int curColumn, const int curType, Console& console )
	{
		// Clear previous info.
		console.setCursorPosition( 74, 8 );
		std::cout << "                                           ";
		for( auto y = 12; y <= 20; y += 2 )
		{
			console.setCursorPosition( 73, y );
			std::cout << "                                                  ";
		}
		for( auto y = 21; y <= 23; ++y )
		{
			console.setCursorPosition( 73, y );
			std::cout << "                                                  ";
		}
		for( auto y = 37; y <= 39; ++y )
		{
			console.setCursorPosition( 73, y );
			std::cout << "                                                  ";
		}

		auto printItemInfo = []( const ItemConsumable& item, Console& console, const int usesOverride = 0 )
		{
			console.setCursorPosition( 95 - static_cast<short>( item.name.length() / 2 ), 8 );
			std::cout << item.name;
			console.setCursorPosition( 73, 12 );
			std::cout << "Uses: " << (usesOverride > 0) ? usesOverride : item.uses;
			console.setCursorPosition( 73, 14 );
			std::cout << "Weight: " << item.weight;
			console.setCursorPosition( 73, 16 );
			std::cout << "Worth: $" << item.worth;
			if( item.damage > 0 )
			{
				console.setCursorPosition( 100, 14 );
				std::cout << "Damage: " << item.damage;
				console.setCursorPosition( 100, 12 );
				std::cout << "Damage type: " << item.damageType;
			}

			std::vector<std::string> conds;
			if( item.strengthCond > 0 )
				conds.push_back( "Strength Requirement: " + std::to_string( item.strengthCond ) );
			if( item.healthCond > 0 )
				conds.push_back( "Health Requirement: " + std::to_string( item.healthCond ) );
			if( item.intelligenceCond > 0 )
				conds.push_back( "Intel. Requirement: " + std::to_string( item.intelligenceCond ) );
			for( auto i = decltype( conds.size() ){ 0 }; i < conds.size(); ++i )
			{
				console.setCursorPosition( 73, 37 + static_cast<short>( i ) );
				std::cout << conds[ i ];
			}

			auto desc = item.desc;
			for( auto y = 20; !desc.empty(); ++y )
			{
				console.setCursorPosition( 73, y );
				std::cout << desc.substr( 0, desc.find( '\n' ) );
				if( desc.find( '\n' ) == std::string::npos )
					desc.clear();
				else
					desc.erase( 0, desc.find( '\n' ) + 1 );
			}
		};
		auto printWeaponInfo = []( const Weapon& wep, Console& console )
		{
			console.setCursorPosition( 95 - static_cast<short>( wep.name.length() / 2 ), 8 );
			std::cout << wep.name;
			console.setCursorPosition( 73, 12 );
			std::cout << "Damage: " << wep.damage;
			console.setCursorPosition( 73, 14 );
			std::cout << "Weight: " << wep.weight;
			console.setCursorPosition( 73, 16 );
			std::cout << "Worth: $" << wep.worth;

			std::vector<std::string> conds;
			if( wep.strengthCond > 0 )
				conds.push_back( "Strength Requirement: " + std::to_string( wep.strengthCond ) );
			if( wep.healthCond > 0 )
				conds.push_back( "Health Requirement: " + std::to_string( wep.healthCond ) );
			if( wep.intelligenceCond > 0 )
				conds.push_back( "Intel. Requriement: " + std::to_string( wep.intelligenceCond ) );
			for( auto i = decltype( conds.size() ){ 0 }; i < conds.size(); ++i )
			{
				console.setCursorPosition( 73, 37 + static_cast<short>( i ) );
				std::cout << conds[ i ];
			}

			auto desc = wep.desc;
			for( auto y = 20; !desc.empty(); ++y )
			{
				console.setCursorPosition( 73, y );
				std::cout << desc.substr( 0, desc.find( '\n' ) );
				if( desc.find( '\n' ) == std::string::npos )
					desc.clear();
				else
					desc.erase( 0, desc.find( '\n' ) + 1 );
			}
		};

		// Print info about currently selected item.
		if( curColumn == 1 )
		{
			if( curRow <= static_cast<int>( shopThings.shopItems.size() ) )
				printItemInfo( shopThings.shopItems[ curRow - 1 ], console, 1 );
			else if( curRow <= static_cast<int>( shopThings.shopItems.size() + shopThings.shopWeapons.size() ) )
				printWeaponInfo( shopThings.shopWeapons[ curRow - 1 - shopThings.shopItems.size() ], console );
		}
		else if( curColumn == 2 && curType == 0 && curRow <= static_cast<int>( shopThings.playerItems.size() ) )
			printItemInfo( shopThings.playerItems[ curRow - 1 ], console );
		else if( curColumn == 2 && curType == 1 && curRow <= static_cast<int>( shopThings.playerWeapons.size() ) )
			printWeaponInfo( shopThings.playerWeapons[ curRow - 1 ], console );
	};

	ShopThings shopThings;
	auto curRow = 1;
	auto curColumn = 1;
	auto curType = 0; // 0 = items, 1 = weapons

	// Print shop interface.
	updateShopThings( shopThings, shop1, player );
	updateShopInterface( shopThings, shop1, player, curRow, curColumn, curType, console );
	putNewCursor( shopThings, curRow, curColumn, curType, console );
	updateInfo( shopThings, player, curRow, curColumn, curType, console );

	// Handle input.
	console.setCursorPosition( 0, 49 );
	for( auto ch = _getch(); ch != '\b'; ch = _getch() )
	{
		console.setCursorPosition( 4, 45 );
		std::cout << "                                 ";

		// Erase old cursor.
		eraseCursor( shopThings, curRow, curColumn, curType, console );

		// Check for UI inputs
		if( ch == ' ' )
		{
			if( curType == 0 )
			{
				curType = 1;
				updateShopInterface( shopThings, shop1, player, curRow, curColumn, curType, console );
				console.setCursorPosition( 38, 5 );
				std::cout << "SELL (Your weapons)";
			}
			else
			{
				curType = 0;
				updateShopInterface( shopThings, shop1, player, curRow, curColumn, curType, console );
				console.setCursorPosition( 38, 5 );
				std::cout << " SELL (Your items) ";
			}
		}
		else if( ch == 0 || ch == 0xE0 )
		{
			const auto ch1 = _getch();
			if( ch1 == 72 && curRow > 1 ) // UP
				curRow--;
			else if( ch1 == 80 && curRow < 25 ) // DOWN
				curRow++;
			else if( ch1 == 75 && curColumn > 1 ) // LEFT
				curColumn--;
			else if( ch1 == 77 && curColumn < 2 ) // RIGHT
				curColumn++;
		}

		// Print new cursor and info about the currently selected item.
		putNewCursor( shopThings, curRow, curColumn, curType, console );
		updateInfo( shopThings, player, curRow, curColumn, curType, console );

		console.setCursorPosition( 0, 49 );
		// If player presses ENTER, do stuff
		if( ch == '\r' )
		{
			if( curColumn == 1 && curRow <= static_cast<int>( shopThings.shopItems.size() ) ) // BUY ITEM
			{
				const auto index = curRow - 1;
				if( player.getMoney() >= shop1.itemPrices[ index ] )
				{
					// Print info text.
					console.setCursorPosition( 4, 45 );
					std::cout << "You bought 1 " << shopThings.shopItems[ index ].codeName << '!';

					// Buy item.
					player.addMoney( -shop1.itemPrices[ index ] );
					player.addItem( shopThings.shopItems[ index ].codeName, 1 );
					if( shop1.nItems[ index ] > 1 )
						--shop1.nItems[ index ];
					else
					{
						shop1.items.erase( shop1.items.begin() + index );
						shop1.itemPrices.erase( shop1.itemPrices.begin() + index );
						shop1.nItems.erase( shop1.nItems.begin() + index );
					}

					kaching.stopSound();
					kaching.loadSound( L"kaching.wav", L"waveaudio" );
					kaching.playSound();

					// Update shop things and interface.
					updateShopThings( shopThings, shop1, player );
					updateShopInterface( shopThings, shop1, player, curRow, curColumn, curType, console );
				}
				else
				{
					console.setCursorPosition( 4, 45 );
					std::cout << "You can't afford that item!";
				}
			}
			else if( curColumn == 1 && curRow <= static_cast<int>( shopThings.shopItems.size() + shopThings.shopWeapons.size() ) ) // BUY WEAPON
			{
				const auto index = curRow - 1 - shopThings.shopItems.size();
				if( player.getMoney() >= shop1.weaponPrices[ index ] )
				{
					if( !player.getInventory().weapons.count( shopThings.shopWeapons[ index ].codeName ) )
					{
						// Print info text.
						console.setCursorPosition( 4, 45 );
						std::cout << "You bought 1 " << shopThings.shopWeapons[ index ].codeName << '!';

						// Buy weapon.
						player.addMoney( -shop1.weaponPrices[ index ] );
						player.addWeapon( shopThings.shopWeapons[ index ].codeName );
						if( shop1.nWeapons[ index ] > 1 )
							--shop1.nWeapons[ index ];
						else
						{
							shop1.weapons.erase( shop1.weapons.begin() + index );
							shop1.weaponPrices.erase( shop1.weaponPrices.begin() + index );
							shop1.nWeapons.erase( shop1.nWeapons.begin() + index );
						}

						kaching.stopSound();
						kaching.loadSound( L"kaching.wav", L"waveaudio" );
						kaching.playSound();

						// Update shop things and interface.
						updateShopThings( shopThings, shop1, player );
						updateShopInterface( shopThings, shop1, player, curRow, curColumn, curType, console );
					}
					else
					{
						console.setCursorPosition( 4, 45 );
						std::cout << "You already have that weapon!";
					}
				}
				else
				{
					console.setCursorPosition( 4, 45 );
					std::cout << "You can't afford that weapon!";
				}
			}
			else if( curColumn == 2 && curType == 0 && curRow <= static_cast<int>( shopThings.playerItems.size() ) ) // SELL ITEM
			{
				const auto index = curRow - 1;

				// Print info text.
				console.setCursorPosition( 4, 45 );
				std::cout << "You sold 1 " << shopThings.playerItems[ index ].codeName << '!';

				// Sell item.
				if( player.getInventory().items[ shopThings.playerItems[ index ].codeName ].uses > 1 )
					player.addItemUses( shopThings.playerItems[ index ].codeName, -1 );
				else
				{
					player.removeItem( shopThings.playerItems[ index ].codeName );
					player.addMoney( shopThings.playerItemPrices[ index ] );
				}

				kaching.stopSound();
				kaching.loadSound( L"kaching.wav", L"waveaudio" );
				kaching.playSound();

				// Update shop things and interface.
				updateShopThings( shopThings, shop1, player );
				updateShopInterface( shopThings, shop1, player, curRow, curColumn, curType, console );
			}
			else if( curColumn == 2 && curType == 1 && curRow <= static_cast<int>( shopThings.playerWeapons.size() ) ) // SELL WEAPON
			{
				const auto index = curRow - 1;

				// Print info text.
				console.setCursorPosition( 4, 45 );
				std::cout << "You sold 1 " << shopThings.playerWeapons[ index ].codeName << '!';

				// Sell weapon.
				player.removeWeapon( shopThings.playerWeapons[ index ].codeName );
				player.addMoney( shopThings.playerWeaponPrices[ index ] );

				kaching.stopSound();
				kaching.loadSound( L"kaching.wav", L"waveaudio" );
				kaching.playSound();

				// Update shop things and interface.
				updateShopThings( shopThings, shop1, player );
				updateShopInterface( shopThings, shop1, player, curRow, curColumn, curType, console );
			}
		}
	}

	kaching.stopSound();
	music.stopLoop();
	::shopCache[ shop ] = shop1; // TODO: Make saveGame() save the shopCache. (and loadGame() load it)
}
void execBattle( const std::string& enemy, const int level, Console& console, Player& player, RNG& rng )
{
	Sound battleMusicIntro( L"battlethemeintro.wav", L"waveaudio" );
	Sound battleMusicLoop;

	PerformanceTimer timer;
	timer.startCounter();
	battleMusicIntro.playSound();

	console.clearScreen();
	std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << '\n';
	std::cout                                                                                                                                      << '\n';
	std::cout << "                                                             BATTLE                                                            " << '\n';
	std::cout                                                                                                                                      << '\n'; Sleep( 450 );
	std::cout << "+------------------------------------------------+----------------------------------------------------------------------------+" << '\n';
	std::cout << "|                   LEVEL:                       |                                                                            |" << '\n'; // ( 27, 5 )
	std::cout << "|                                                |                                                                            |" << '\n'; // ( 55, 6 )
	std::cout << "|                                                |                                                                            |" << '\n';
	std::cout << "|                                                |                                                                            |" << '\n'; // ( 55, 8 )
	std::cout << "|                                                |                                                                            |" << '\n';
	std::cout << "|                                                |                                                                            |" << '\n';
	std::cout << "|                                                |                                                                            |" << '\n';
	std::cout << "|                                                |                                                                            |" << '\n';
	std::cout << "|                                                |                                                                            |" << '\n';
	std::cout << "|                                                |                                                                            |" << '\n'; Sleep( 450 );
	std::cout << "|                                                |                                                                            |" << '\n';
	std::cout << "|                                                |                                                                            |" << '\n';
	std::cout << "|                                                |                                                                            |" << '\n';
	std::cout << "|                                                |                                                                            |" << '\n';
	std::cout << "|                                                |                                                                            |" << '\n';
	std::cout << "|                                                |                                                                            |" << '\n';
	std::cout << "|                                                |                                                                            |" << '\n';
	std::cout << "|                                                |                              Your weapon:                                  |" << '\n';
	std::cout << "|                                                |                              Your base damage:                             |" << '\n';
	std::cout << "|                                                |                                                                            |" << '\n';
	std::cout << "|  Enemy health:                                 |                              Your health:                                  |" << '\n'; // ( 17, 25 ) ( 93, 25 )
	std::cout << "|  [ |||||||||||||||||||||||||||||||||||||||| ]  |                              [ |||||||||||||||||||||||||||||||||||||||| ]  |" << '\n'; // ( 5, 26 ) ( 82, 26 )
	std::cout << "+------------------------------------------------+----------------------------------------------------------------------------+" << '\n'; Sleep( 450 );
	std::cout << "|                                                                                                                             |" << '\n';
	std::cout << "|                                                      What will you do?                                                      |" << '\n';
	std::cout << "|                                                                                                                             |" << '\n';
	std::cout << "|                [  ATTACK  ]               [  DEFEND  ]               [ USE ITEM ]               [ RUN AWAY ]                |" << '\n'; // ( 23, 31 ) ( 50, 31 ) ( 77, 31 ) ( 104, 31 )
	std::cout << "|                                                                                                                             |" << '\n';
	std::cout << "+-----------------------------------------------------------------------------------------------------------------------------+" << '\n'; // ( 0, 33 )
	std::cout                                                                                                                                      << '\n'; // ( 0, 34 )
	std::cout                                                                                                                                      << '\n'; // ( 0, 35 )
	std::cout                                                                                                                                      << '\n'; // ( 0, 36 )
	std::cout                                                                                                                                      << '\n'; // ( 0, 37 )
	std::cout                                                                                                                                      << '\n'; // ( 0, 38 )
	std::cout                                                                                                                                      << '\n'; // ( 0, 39 )
	std::cout                                                                                                                                      << '\n'; // ( 0, 40 )
	std::cout                                                                                                                                      << '\n'; // ( 0, 41 )
	std::cout                                                                                                                                      << '\n'; // ( 0, 42 )
	std::cout                                                                                                                                      << '\n'; // ( 0, 43 )
	std::cout                                                                                                                                      << '\n'; // ( 0, 44 )
	std::cout                                                                                                                                      << '\n'; // ( 0, 45 )
	std::cout                                                                                                                                      << '\n'; // ( 0, 46 )
	std::cout                                                                                                                                      << '\n'; // ( 0, 47 )
	std::cout                                                                                                                                      << '\n'; // ( 0, 48 )
	                                                                                                                                                        // ( 0, 49 )
	const auto printTime = static_cast<int>( round( timer.getCounter() - 1350 ) );
	if( 650 - printTime > 0 )
		Sleep( 650 - printTime );
	battleMusicLoop.loopSound( L"battlethemeloop.wav" );
	battleMusicIntro.stopSound();

	auto enemy1 = loadEnemyData( enemy, "gameenemydata.txt" );
	enemy1.level = level;
	enemy1.health = static_cast<int>( round( enemy1.health * ( 1 + enemy1.level / 6.0f ) ) );
	enemy1.damage = static_cast<int>( round( enemy1.damage * ( 1 + enemy1.level / 6.0f ) ) );

	const auto playerOriginalHealth = player.getHealth();
	const auto enemyOriginalHealth = enemy1.health;

	auto clearEnemyImage = []( Console& console )
	{
		for( auto y = 6; y < 24; ++y )
		{
			console.setCursorPosition( 2, y );
			std::cout << "                                               ";
		}
	};
	auto printEnemyImage = []( const std::string& image, Console& console )
	{
		auto img = image;
		for( auto y = 6; !img.empty(); ++y )
		{
			console.setCursorPosition( 2, y );
			std::cout << img.substr( 0, img.find( '\n' ) );
			if( img.find( '\n' ) == std::string::npos )
				img.clear();
			else
				img.erase( 0, img.find( '\n' ) + 1 );
		}
	};
	auto printEnemyHealthBar = [enemyOriginalHealth]( const Enemy& enemy1, Console& console )
	{
		console.setCursorPosition( 5, 26 );
		auto enemyHealthBar = static_cast<int>( round( ( enemy1.health / static_cast<float>( enemyOriginalHealth ) ) * 40 ) );
		for( auto i = 0; i < enemyHealthBar; ++i )
			_putch( '|' );
		console.setCursorPosition( 127, 49 );
	};
	auto printPlayerHealthBar = [playerOriginalHealth]( const Player& player, Console& console )
	{
		console.setCursorPosition( 82, 26 );
		auto playerHealthBar = static_cast<int>( round( ( player.getHealth() / static_cast<float>( playerOriginalHealth ) ) * 40 ) );
		for( auto i = 0; i < playerHealthBar; ++i )
			_putch( '|' );
		console.setCursorPosition( 127, 49 );
	};
	auto getPlayerDamage = []( const Player& player )
	{
		const auto damage = loadWeaponData( player.getEquippedWeapon(), "gameitemdata.txt" ).damage;
		if( damage <= 0 )
			return static_cast<int>( round( 1.0f + player.getStrength() / 6.0f ) );
		else
			return static_cast<int>( round( damage * ( 1.0f + player.getStrength() / 6.0f ) ) );
	};
	auto getPlayerWeapon = []( const Player& player )
	{
		return (player.getEquippedWeapon().empty()) ? "Fists" : player.getEquippedWeapon();
	};
	auto printBattleScreen = [playerOriginalHealth, enemyOriginalHealth, getPlayerDamage, printEnemyHealthBar, printPlayerHealthBar, printEnemyImage]( const Enemy& enemy1, const Player& player, Console& console )
	{
		console.clearScreen();
		std::cout << "-------------------------------------------------------------------------------------------------------------------------------" << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout << "                                                             BATTLE"                                                             << '\n';
		std::cout                                                                                                                                      << '\n';
		std::cout << "+------------------------------------------------+----------------------------------------------------------------------------+" << '\n';
		std::cout << "|                   LEVEL:                       |                                                                            |" << '\n'; // ( 27, 5 )
		std::cout << "|                                                |                                                                            |" << '\n'; // ( 55, 6 )
		std::cout << "|                                                |                                                                            |" << '\n';
		std::cout << "|                                                |                                                                            |" << '\n'; // ( 55, 8 )
		std::cout << "|                                                |                                                                            |" << '\n';
		std::cout << "|                                                |                                                                            |" << '\n';
		std::cout << "|                                                |                                                                            |" << '\n';
		std::cout << "|                                                |                                                                            |" << '\n';
		std::cout << "|                                                |                                                                            |" << '\n';
		std::cout << "|                                                |                                                                            |" << '\n';
		std::cout << "|                                                |                                                                            |" << '\n';
		std::cout << "|                                                |                                                                            |" << '\n';
		std::cout << "|                                                |                                                                            |" << '\n';
		std::cout << "|                                                |                                                                            |" << '\n';
		std::cout << "|                                                |                                                                            |" << '\n';
		std::cout << "|                                                |                                                                            |" << '\n';
		std::cout << "|                                                |                                                                            |" << '\n';
		std::cout << "|                                                |                              Your weapon:                                  |" << '\n';
		std::cout << "|                                                |                              Your base damage:                             |" << '\n';
		std::cout << "|                                                |                                                                            |" << '\n';
		std::cout << "|  Enemy health:                                 |                              Your health:                                  |" << '\n'; // ( 17, 25 ) ( 93, 25 )
		std::cout << "|  [                                          ]  |                              [                                          ]  |" << '\n'; // ( 5, 26 ) ( 82, 26 )
		std::cout << "+------------------------------------------------+----------------------------------------------------------------------------+" << '\n';
		std::cout << "|                                                                                                                             |" << '\n';
		std::cout << "|                                                      What will you do?                                                      |" << '\n';
		std::cout << "|                                                                                                                             |" << '\n';
		std::cout << "|                [  ATTACK  ]               [  DEFEND  ]               [ USE ITEM ]               [ RUN AWAY ]                |" << '\n'; // ( 23, 31 ) ( 50, 31 ) ( 77, 31 ) ( 104, 31 )
		std::cout << "|                                                                                                                             |" << '\n';
		std::cout << "+-----------------------------------------------------------------------------------------------------------------------------+" << '\n'; // ( 0, 33 )
		std::cout                                                                                                                                      << '\n'; // ( 0, 34 )
		std::cout                                                                                                                                      << '\n'; // ( 0, 35 )
		std::cout                                                                                                                                      << '\n'; // ( 0, 36 )
		std::cout                                                                                                                                      << '\n'; // ( 0, 37 )
		std::cout                                                                                                                                      << '\n'; // ( 0, 38 )
		std::cout                                                                                                                                      << '\n'; // ( 0, 39 )
		std::cout                                                                                                                                      << '\n'; // ( 0, 40 )
		std::cout                                                                                                                                      << '\n'; // ( 0, 41 )
		std::cout                                                                                                                                      << '\n'; // ( 0, 42 )
		std::cout                                                                                                                                      << '\n'; // ( 0, 43 )
		std::cout                                                                                                                                      << '\n'; // ( 0, 44 )
		std::cout                                                                                                                                      << '\n'; // ( 0, 45 )
		std::cout                                                                                                                                      << '\n'; // ( 0, 46 )
		std::cout                                                                                                                                      << '\n'; // ( 0, 47 )
		std::cout                                                                                                                                      << '\n'; // ( 0, 48 )
		                                                                                                                                                        // ( 0, 49 )
		console.setCursorPosition( 55, 6 );
		std::cout << enemy1.name;
		console.setCursorPosition( 27, 5 );
		std::cout << enemy1.level;
		console.setCursorPosition( 17, 25 );
		std::cout << enemy1.health;
		printEnemyImage( enemy1.image, console );

		auto desc = enemy1.desc;
		for( auto y = 8; !desc.empty(); ++y )
		{
			console.setCursorPosition( 55, y );
			std::cout << desc.substr( 0, desc.find( '\n' ) );
			if( desc.find( '\n' ) == std::string::npos )
				desc.clear();
			else
				desc.erase( 0, desc.find( '\n' ) + 1 );
		}

		console.setCursorPosition( 93, 25 );
		std::cout << player.getHealth();
		console.setCursorPosition( 93, 22 );
		if( player.getEquippedWeapon().empty() )
			std::cout << "Fists";
		else
			std::cout << player.getEquippedWeapon();
		console.setCursorPosition( 98, 23 );
		std::cout << getPlayerDamage( player );

		printEnemyHealthBar( enemy1, console );
		printPlayerHealthBar( player, console );
	};
	auto printBattleLog = []( const std::vector<std::string>& battleLog, Console& console )
	{
		console.setCursorPosition( 0, 35 );
		std::cout << "                                                                                                                               ";
		for( auto y = 36; y <= 46; y += 2 ) {
			console.setCursorPosition( 0, y );
			std::cout << "                                                                                                                               ";
		}
		for( auto i = decltype( battleLog.size() ){ 0 }; i < battleLog.size() && i < 6; ++i ) {
			console.setCursorPosition( 0, 36 + static_cast<short>( i ) * 2 );
			std::cout << battleLog.at( battleLog.size() - 1 - i );
		}
	};

	std::vector<std::string> battleLog;

	// Print what enemy you're fighting.
	battleLog.push_back( "You are fighting a level " + std::to_string( enemy1.level ) + " " + enemy1.name + "!" );
	console.setCursorPosition( 0, 35 );
	printBattleScreen( enemy1, player, console );
	printBattleLog( battleLog, console );

	bool playersTurn = true;
	auto curButton = 0;
	auto curCycle = 1;
	bool escaped = false;
	auto chargedDamage = 0;
	for( bool loop = true; loop && enemy1.health > 0 && player.getHealth() > 0; ++curCycle )
	{
		// Draw cursor
		if( curCycle == 10 )
		{
			console.setCursorPosition( 23 + ( 27 * curButton ) - 10, 31 );
			std::cout << ">> ";
			console.setCursorPosition( 23 + ( 27 * curButton ) + 7, 31 );
			std::cout << " <<";
			console.setCursorPosition( 127, 49 );
		}
		else if( curCycle == 20 )
		{
			console.setCursorPosition( 23 + ( 27 * curButton ) - 10, 31 );
			std::cout << " >>";
			console.setCursorPosition( 23 + ( 27 * curButton ) + 7, 31 );
			std::cout << "<< ";
			console.setCursorPosition( 127, 49 );
			curCycle = 1;
		}
		Sleep( 48 );

		// Handle input
		if( _kbhit() && playersTurn == true )
		{
			bool playerIsBlocking = false;

			console.setCursorPosition( 23 + ( 27 * curButton ) - 10, 31 );
			std::cout << "   ";
			console.setCursorPosition( 23 + ( 27 * curButton ) + 7, 31 );
			std::cout << "   ";

			const auto ch = _getch();
			if( ch == 0 || ch == 0xE0 )
			{
				auto ch1 = _getch();
				if( ch1 == 75 && curButton > 0 ) // LEFT
				{
					--curButton;
					curCycle = 9;
				}
				else if( ch1 == 77 && curButton < 3 ) // RIGHT
				{
					++curButton;
					curCycle = 9;
				}
			}
			else if( ch == '\r' )
			{
				if( curButton == 0 ) // ATTACK
				{
					// Calculate damage.
					bool crit = false;
					auto damage = getPlayerDamage( player );
					auto dice = rng.getRandomInt( 0, 100 );
					if( dice <= 10 ) // 10% crit chance (double damage)
					{
						damage *= 2;
						crit = true;
					}
					else if( dice <= 40 ) // 30% chance of less damage
						damage = static_cast<int>( round( damage - damage / 3.0f ) );
					else if( dice <= 70 ) // 30% chance of more damage
						damage = static_cast<int>( round( damage + damage / 3.0f ) );
					// 30% chance of no damage difference

					// Print text to battle log.
					battleLog.push_back( "You hit the " + enemy1.name + " with your " + getPlayerWeapon( player ) + " for " + std::to_string( damage ) + " damage!" + ( (crit) ? " (CRITICAL HIT!)" : "" ) );
					printBattleLog( battleLog, console );

					// Set enemy health.
					enemy1.health -= damage;
					if( enemy1.health < 0 )
						enemy1.health = 0;

					// Refresh enemy health.
					console.setCursorPosition( 17, 25 );
					std::cout << "          ";
					console.setCursorPosition( 17, 25 );
					std::cout << enemy1.health;

					// Refresh enemy health bar.
					console.setCursorPosition( 5, 26 );
					std::cout << "                                        ";
					printEnemyHealthBar( enemy1, console );

					// Show enemy hit image for half a second.
					clearEnemyImage( console );
					printEnemyImage( enemy1.hitImage, console );
					console.setCursorPosition( 127, 49 );
					Sleep( 500 );
					clearEnemyImage( console );
					printEnemyImage( enemy1.image, console );

					playersTurn = false;
				}
				else if( curButton == 1 ) // DEFEND
				{
					playerIsBlocking = true;
					battleLog.push_back( "You raise your guard." );
					printBattleLog( battleLog, console );
					playersTurn = false;
				}
				else if( curButton == 2 ) // USE ITEM
				{
					std::string resultInv;
					if( !execInventory( resultInv, true, console, player ) )
					{
						printBattleScreen( enemy1, player, console );
						printBattleLog( battleLog, console );
					}
					else
					{
						std::string resultItem;
						if( !player.useItem( resultInv, resultItem ) )
						{
							console.clearScreen();
							std::cout << "You can't use " << resultInv << "!\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
							console.getEnter();
						}
						else
						{
							auto loc = loadLocationData( resultItem, "gameitemdata.txt", false, true );

							std::map<std::string, int> addedStats;
							for( const auto& stat : loc.addStats )
							{
								auto statName = stat.substr( 0, stat.find( ' ' ) );
								auto value = std::stoi( stat.substr( stat.find( ' ' ) + 1, std::string::npos ) );

								if( statName == "health" )
									player.addHealth( value );
								else if( statName == "strength" )
									player.addStrength( value );
								else if( statName == "intelligence" )
									player.addIntelligence( value );
								else if( statName == "xp" )
									player.addXp( value );

								addedStats[ statName ] = value;
							}

							if( !loc.locationText.empty() )
							{
								console.clearScreen();
								auto nLines = displayLocationText( resultItem, "gameitemdata.txt", 40, console, player, true );
								// Print newlines until we reach the bottom.
								constexpr int screenHeight = 50;
								for( auto i = screenHeight - 2 - nLines; i > 0; --i )
									_putch( '\n' );
								console.getEnter();
							}

							// Show any stat changes to the player.
							for( const auto& stat : addedStats )
								showStatChange( stat.first, stat.second, console );

							auto item = loadItemData( resultInv, "gameitemdata.txt" );
							if( item.isBattleItem && item.damage > 0 )
							{
								playersTurn = false;

								// Calculate damage.
								auto damage = item.damage;
								if( item.damageType == enemy1.damageTypeResistance )
									damage /= 2;
								else if( item.damageType == enemy1.damageTypeWeakness )
									damage *= 2;

								printBattleScreen( enemy1, player, console );
								printBattleLog( battleLog, console );
								Sleep( 1000 );
								battleLog.push_back( "You used " + item.prefix + item.name + " on the " + enemy1.name + " and did " + std::to_string( damage ) + " damage!" );
								printBattleLog( battleLog, console );

								// Set enemy health.
								enemy1.health -= damage;
								if( enemy1.health < 0 )
									enemy1.health = 0;

								// Refresh enemy health.
								console.setCursorPosition( 17, 25 );
								std::cout << "          ";
								console.setCursorPosition( 17, 25 );
								std::cout << enemy1.health;

								// Refresh enemy health bar.
								console.setCursorPosition( 5, 26 );
								std::cout << "                                        ";
								printEnemyHealthBar( enemy1, console );

								// Show enemy hit image for half a second.
								clearEnemyImage( console );
								printEnemyImage( enemy1.hitImage, console );
								console.setCursorPosition( 127, 49 );
								Sleep( 500 );
								clearEnemyImage( console );
								printEnemyImage( enemy1.image, console );
							}
							else
							{
								printBattleScreen( enemy1, player, console );
								printBattleLog( battleLog, console );
							}
						}
					}
				}
				else if( curButton == 3 ) // RUN AWAY
				{
					if( rng.getRandomInt( 0, 100 ) <= enemy1.runChance )
					{
						escaped = true;
						loop = false;
						battleMusicLoop.stopLoop();
						console.clearScreen();
						std::cout << "You managed to escape from the " << enemy1.name << '!';
						console.setCursorPosition( 0, 47 );
						console.getEnter();
					}
					else
					{
						battleLog.push_back( "You failed to escape!" );
						printBattleLog( battleLog, console );
					}

					playersTurn = false;
				}
			}
			
			if( !playersTurn && enemy1.health > 0 && !escaped )
			{
				Sleep( 750 );

				auto damage = enemy1.damage;
				// Calculate damage.
				auto dice = rng.getRandomInt( 0, 90 );
				if( dice <= 30 ) // 33.33% chance of less damage
					damage = static_cast<int>( round( damage + damage / 3.0f ) );
				else if( dice > 30 && dice <= 60 ) // 33.33% chance of more damage
					damage = static_cast<int>( round( damage - damage / 3.0f ) );
				// 33.33% chance of no damage difference

				auto damageBlocked = 0;
				if( playerIsBlocking )
				{
					auto blockDamage = static_cast<int>( round( damage / 2.0f ) );
					damage -= blockDamage;
					damageBlocked = blockDamage;
				}

				if( rng.getRandomInt( 0, 10 ) == 1 )
				{
					chargedDamage = damage * 3;
					battleLog.push_back( "The " + enemy1.name + " is charging a special attack!" );
					printBattleLog( battleLog, console );
				}
				else
				{
					if( chargedDamage > 0 && !playerIsBlocking )
						damage = chargedDamage;

					// Print text to battle log.
					auto tempLog = "The " + enemy1.name + " attacked you for " + std::to_string( damage ) + " damage!";
					if( chargedDamage > 0 )
					{
						chargedDamage = 0;
						tempLog.append( (playerIsBlocking) ? " (You blocked the special attack!)" : " (CRITICAL HIT!)" );
					}
					else if( playerIsBlocking )
						tempLog.append( " (You blocked " + std::to_string( damageBlocked ) + " damage!)" );
					battleLog.push_back( tempLog );
					printBattleLog( battleLog, console );

					// Set player health.
					player.addHealth( -damage );
					if( player.getHealth() < 0 )
						player.setHealth( 0 );
				}

				// Refresh player health.
				console.setCursorPosition( 93, 25 );
				std::cout << "          ";
				console.setCursorPosition( 93, 25 );
				std::cout << player.getHealth();

				// Refresh player health bar.
				console.setCursorPosition( 82, 26 );
				std::cout << "                                        ";
				printPlayerHealthBar( player, console );
				
				playerIsBlocking = false;
				playersTurn = true;
			}
		}
	}

	player.setHealth( playerOriginalHealth );
	if( enemy1.health == 0 )
	{
		// Clear image.
		clearEnemyImage( console );

		console.setCursorPosition( 20, 14 );
		std::cout << "VICTORY!";

		// Refresh player health.
		console.setCursorPosition( 93, 25 );
		std::cout << "          ";
		console.setCursorPosition( 93, 25 );
		std::cout << player.getHealth();

		// Refresh player health bar.
		console.setCursorPosition( 82, 26 );
		std::cout << "                                        ";
		printPlayerHealthBar( player, console );

		Sleep( 2000 );

		player.addMoney( enemy1.reward );
		player.addXp( enemy1.reward * 2 );

		battleLog.clear();
		battleLog.push_back( "Congratulations, you beat the " + enemy1.name + "! You earned: $" + std::to_string( enemy1.reward ) + ", You gained: " + std::to_string( enemy1.reward * 2 ) + " XP!" );
		printBattleLog( battleLog, console );

		console.setCursorPosition( 127, 49 );
	}
	else if( escaped == false )
	{
		Sleep( 2000 );
		battleLog.clear();
		battleLog.push_back( "You failed!" );
		printBattleLog( battleLog, console );
		console.setCursorPosition( 127, 49 );
	}

	if( escaped == false )
	{
		Sleep( 2000 );
		console.setCursorPosition( 0, 47 );
		std::cout << "Press SPACEBAR to continue...\n\n";
		while( _getch() != ' ' );

		if( enemy1.health != 0 )
		{
			auto moneyLoss = enemy1.reward * 2;
			player.addMoney( -moneyLoss );
			if( player.getMoney() < 0 )
			{
				moneyLoss = player.getMoney();
				player.setMoney( 0 );
			}

			console.clearScreen();
			std::cout << "You lost: $" << moneyLoss << '!';
			console.setCursorPosition( 0, 47 );
			console.getEnter();
		}
	}

	battleMusicLoop.stopLoop();
}

void debugPrintLocationData( const std::string& location, const std::string& filename )
{
	std::cout << "----INFO: [" << location << "]----\n";
	const auto loc = loadLocationData( location, filename, false );

	// PRINT OPTIONS NAME
	if( !loc.optionsName.empty() )
		std::cout << "OPTIONS NAME: " << loc.optionsName << "\n\n";
	else
		std::cout << "ERROR: Options name of " << location << " not loaded!\n\n";
	// PRINT DISPLAY NAME
	if( !loc.displayName.empty() )
		std::cout << "DISPLAY NAME: " << loc.displayName << "\n\n";
	else
		std::cout << "ERROR: no Display name found!\n\n";
	// PRINT LOCATION TEXT
	if( !loc.locationText.empty() )
		std::cout << "LOCATION TEXT:\n" << loc.locationText << "\n\n";
	else
		std::cout << "ERROR: Location text of " << location << " not loaded!\n\n";
	// PRINT OPTIONS
	std::cout << "OPTIONS:\n";
	if( !loc.defaultOption.empty() )
		std::cout << loc.defaultOption << "(DEFAULT)\n";
	for( const auto& option : loc.options )
	{
		std::cout << option;
		if( loc.resultLoc.count( option ) )
			std::cout << ":" << loc.resultLoc.at( option );
		if( loc.conds.count( option ) )
			std::cout << ";" << loc.conds.at( option );
		std::cout << '\n';
	}
	std::cout << '\n';
	// PRINT ACTIONS
	std::cout << "ACTIONS:\n";
	for( const auto& cond : loc.addConds )
		std::cout << "Add Condition: \"" << cond << "\"\n";
	for( const auto& item : loc.addItems )
		std::cout << "Add Item: \"" << item << "\"\n";
	for( const auto& stat : loc.addStats )
		std::cout << "Add Stat: \"" << stat << "\"\n";
	for( const auto& cond : loc.removeConds )
		std::cout << "Remove Condition: \"" << cond << "\"\n";
	for( const auto& item : loc.removeItems )
		std::cout << "Remove Item: \"" << item << "\"\n";
	for( const auto& stat : loc.removeStats )
		std::cout << "Remove Stat: \"" << stat << "\"\n";
	std::cout << "---------------------------------------\n\n\n";
}
void debugPrintItemData( const std::string& item )
{
	std::cout << "----INFO: [" << item << "]----\n";
	const auto item1 = loadItemData( item, "gameitemdata.txt" );

	// PRINT ITEM NAME
	if( !item1.name.empty() )
		std::cout << "NAME: " << item1.name << "\n\n";
	else
		std::cout << "ERROR: Name of " << item << " not loaded!\n\n";
	// PRINT ITEM PREFIX
	if( !item1.prefix.empty() )
		std::cout << "PREFIX: " << item1.prefix << "\n\n";
	else
		std::cout << "ERROR: Prefix of " << item << " not loaded!\n\n";
	// PRINT ITEM USES
	std::cout << "USES: " << ( (item1.uses > 0) ? std::to_string( item1.uses ) : "Infinite" ) << "\n\n";
	// PRINT ITEM WEIGHT
	std::cout << "WEIGHT: " << item1.weight << "\n\n";
	// PRINT ITEM WORTH
	std::cout << "WORTH: " << item1.worth << "\n\n";
	// PRINT ITEM DESCRIPTION
	if( !item1.desc.empty() )
		std::cout << "DESCRIPTION:\n" << item1.desc << "\n\n";
	else
		std::cout << "ERROR: Description of " << item << " not loaded!\n\n";
	std::cout << "---------------------------------------\n\n\n";
}
void debugPrintEnemyData( const std::string& enemy )
{
	std::cout << "----INFO: [" << enemy << "]----\n";
	const auto enemy1 = loadEnemyData( enemy, "gameenemydata.txt" );

	// PRINT ENEMY NAME
	if( !enemy1.name.empty() )
		std::cout << "NAME: " << enemy1.name << "\n\n";
	else
		std::cout << "ERROR: Name of " << enemy << " not loaded!\n\n";
	// PRINT ENEMY PREFIX
	if( !enemy1.prefix.empty() )
		std::cout << "PREFIX: " << enemy1.prefix << "\n\n";
	else
		std::cout << "ERROR: Prefix of " << enemy << " not loaded!\n\n";
	// PRINT ENEMY DEFAULT HEALTH
	std::cout << "DEFAULT HEALTH: " << enemy1.health << "\n\n";
	// PRINT ENEMY DEFAULT DAMAGE
	std::cout << "DEFAULT DAMAGE: " << enemy1.damage << "\n\n";
	// PRINT ENEMY RUN CHANCE
	std::cout << "RUN CHANCE: " << enemy1.runChance << "\n\n";
	// PRINT ENEMY MONEY REWARD
	std::cout << "MONEY REWARD: $" << enemy1.reward << "\n\n";
	// PRINT ENEMY IMAGE
	if( !enemy1.image.empty() )
		std::cout << "IMAGE:\n" << enemy1.image << "\n\n";
	else
		std::cout << "ERROR: Image of " << enemy << " not loaded!\n\n";
	// PRINT ENEMY HIT IMAGE
	if( !enemy1.hitImage.empty() )
		std::cout << "HIT IMAGE:\n" << enemy1.hitImage << "\n\n";
	else
		std::cout << "ERROR: Hit image of " << enemy << " not loaded!\n\n";
	// PRINT ENEMY DESCRIPTION
	if( !enemy1.desc.empty() )
		std::cout << "DESCRIPTION:\n" << enemy1.desc << "\n\n";
	else
		std::cout << "ERROR: Description of " << enemy << " not loaded!\n\n";
	std::cout << "---------------------------------------\n\n\n";
}