#ifndef PLAYER_H
#define PLAYER_H
#pragma once

#include "Inventory.h"

#include <unordered_set>

class Player final
{
public:
	Player();

	int getCharAge() const;
	std::string getCharName() const;
	std::string getCharSex() const;
	void setCharAge( const int age );
	void setCharName( const std::string& name );
	void setCharSex( const std::string& sex );

	bool isGameOver() const;
	int getHealth() const;
	int getStrength() const;
	int getIntelligence() const;
	int getMoney() const;
	int getXp() const;
	int getLevel() const;
	void setGameOver( const bool val );
	void setHealth( const int val );
	void setStrength( const int val );
	void setIntelligence( const int val );
	void setMoney( const int val );
	void setXp( const int val );
	void addHealth( const int val );
	void addStrength( const int val );
	void addIntelligence( const int val );
	void addMoney( const int val );
	void addXp( const int val );

	std::string getEquippedWeapon() const;
	void setEquippedWeapon( const std::string& weapon );

	Inventory getInventory() const;
	void addItem( const std::string& itemName, const int maxFirstTimeUses = 0 );
	void addItemUses( const std::string& itemName, const int uses );
	void removeItem( const std::string& itemName );
	void addWeapon( const std::string& weaponName );
	void removeWeapon( const std::string& weaponName );

	bool hasItemCond( const std::string& condName ) const;
	bool hasLocCond( const std::string& condName ) const;
	std::unordered_set<std::string> getItemConds() const;
	std::unordered_set<std::string> getLocConds() const;
	bool findItemCond( const std::string& searchStr, std::string* result = nullptr ) const;
	bool findLocCond( const std::string& searchStr, std::string* result = nullptr ) const;
	void addItemCond( const std::string& condName );
	void addLocCond( const std::string& condName );
	void removeItemCond( const std::string& condName );
	void removeLocCond( const std::string& condName );
	void clearConds();

	std::string getCurrentLocation() const;
	std::string getCurFile() const;
	bool hasBeenLocation( const std::string& locationName ) const;
	std::string getPrevLocationsString() const;
	void setCurrentLocation( const std::string& locationName );
	void setCurFile( const std::string& fileName );
	void addPrevLocation( const std::string& locationName );

	bool useItem( const std::string& itemName, std::string& resultLoc );
	void initializeConds( const std::string& filename );

private:
	int	m_iCharAge;
	std::string	m_sCharName;
	std::string	m_sCharSex;

	bool m_bGameOver;
	int	m_iHealth;
	int	m_iStrength;
	int	m_iIntelligence;
	int	m_iMoney;
	int m_iXp;

	std::string	m_sEquippedWeapon;
	Inventory m_Inventory;

	std::unordered_set<std::string> m_ItemConds;
	std::unordered_set<std::string> m_LocConds;

	std::string	m_sCurLocation;
	std::string m_sCurFile;
	std::unordered_set<std::string> m_PrevLocations;
};

struct Enemy final
{
	Enemy() : level( 0 ), health( 0 ), damage( 0 ), runChance( 0 ), reward( 0 ), damageTypeWeakness(), damageTypeResistance(), prefix(), name(), image(), hitImage(), desc() {}

	int level;
	int health;
	int damage;
	int runChance;
	int reward;
	std::string damageTypeWeakness;
	std::string damageTypeResistance;
	std::string prefix;
	std::string name;
	std::string image;
	std::string hitImage;
	std::string desc;
};

#endif