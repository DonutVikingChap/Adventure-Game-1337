#ifndef UI_H
#define UI_H
#pragma once

#include <string>
#include <unordered_set>

// Forward declaration.
class Console;
class Player;
class RNG;
struct ItemConsumable;
struct Weapon;

int getInputInt( const bool eraseWhenDone, const short line, Console& console );
bool getInputUntilIntRange( int& result, const int min, const int max, const bool retry, const bool eraseWhenDone, const short line, Console& console );
std::string getInputString( const int maxLen, const bool makeLower, const bool allowNumbers, const bool allowSpecialChars, const bool eraseWhenDone, const short line, Console& console );
bool getInputUntilStrings( std::string& result, const std::unordered_set<std::string>& allowedStrings, const int maxLen, const bool makeLower, const bool allowNumbers, const bool allowSpecialChars, const bool retry, const bool eraseWhenDone, const short line, Console& console );

std::unordered_set<std::string> getStandardCommands();
std::unordered_set<std::string> getSynonyms( const std::string& word );

bool mainMenu( Console& console, Player& player, RNG& rng );
void charCreation( Console& console, Player& player, RNG& rng );
void statMenu( Console& console, Player& player, RNG& rng );
void confirmMenu( Console& console, Player& player, RNG& rng );
void helpMenu( Console& console );
void saveMenu( Console& console, Player& player );
void loadMenu( Console& console, Player& player, RNG& rng );
void showStatMenu( Console& console, Player& player );
void showStatChange( const std::string& statType, const int value, Console& console );
bool execInventory( std::string& resultLocation, const bool allowBattleItems, Console& console, Player& player );

void debugPrintAllLocationData();
void debugPrintAllItemData();
void debugPrintAllEnemyData();

#endif