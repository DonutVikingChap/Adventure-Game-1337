#ifndef LOCATION_DATA_H
#define LOCATION_DATA_H
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

// Forward declaration.
class Player;
class Console;
class RNG;
struct ItemConsumable;
struct Weapon;
struct Enemy;
struct Shop;

struct Location final
{
	std::string displayName;
	std::string optionsName;
	std::string locationText;
	std::string defaultOption;
	std::string shop;
	std::vector<int> battleLevel;
	std::vector<int> battleChanceLevel;
	std::vector<int> battleChancePercentage;
	std::vector<std::string> battleEnemy;
	std::vector<std::string> battleChanceEnemy;
	std::vector<std::string> addItems;
	std::vector<std::string> addWeapons;
	std::vector<std::string> addConds;
	std::vector<std::string> addStats;
	std::vector<std::string> removeItems;
	std::vector<std::string> removeWeapons;
	std::vector<std::string> removeConds;
	std::vector<std::string> removeStats;
	std::unordered_set<std::string> options;
	std::unordered_map<std::string, std::string> resultLoc;
	std::unordered_map<std::string, std::string> conds;
};

bool saveGame( const std::string& fileName, Player& player );
bool loadGame( const std::string& fileName, Player& player );
bool deleteSaveGame( const std::string& fileName );
void gameOver( Console& console, Player& player );

Location loadLocationData( const std::string& location, const std::string& filename, const bool overRideData, const bool dataFile = false );
ItemConsumable loadItemData( const std::string& item, const std::string& filename );
Weapon loadWeaponData( const std::string& weapon, const std::string& filename );
Enemy loadEnemyData( const std::string& enemy, const std::string& filename );
Shop loadShopData( const std::string& shop, const std::string& filename );

int displayLocationText( const std::string& location, const std::string& filename, const int delay, Console& console, Player& player, const bool dataFile = false ); // Returns the number of lines in the text.
std::string handleLocationInput( const std::string& location, const std::string& filename, const short line, Console& console, Player& player, RNG& rng );
std::string execLocation( const std::string& location, const std::string& filename, Console& console, Player& player, RNG& rng, const bool dataFile = false );

void execShop( const std::string& shop, Console& console, Player& player );
void execBattle( const std::string& enemy, const int level, Console& console, Player& player, RNG& rng );

void debugPrintLocationData( const std::string& location, const std::string& filename );
void debugPrintItemData( const std::string& item );
void debugPrintEnemyData( const std::string& enemy );

#endif