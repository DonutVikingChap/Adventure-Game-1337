#include "Player.h"
#include "LocationData.h"
#include "Inventory.h"
#include "Console.h"

#include <fstream>

Player::Player()
	:
m_iCharAge( 0 ),
m_sCharName(),
m_sCharSex(),
m_bGameOver( false ),
m_iHealth( 0 ),
m_iStrength( 0 ),
m_iIntelligence( 0 ),
m_iMoney( 0 ),
m_iXp( 0 ),
m_sEquippedWeapon(),
m_Inventory(),
m_ItemConds(),
m_LocConds(),
m_sCurLocation(),
m_sCurFile(),
m_PrevLocations()
{}
int Player::getCharAge() const
{
	return m_iCharAge;
}
std::string Player::getCharName() const
{
	return m_sCharName;
}
std::string Player::getCharSex() const
{
	return m_sCharSex;
}
void Player::setCharAge( const int age )
{
	m_iCharAge = age;
}
void Player::setCharName( const std::string& name )
{
	m_sCharName = name;
}
void Player::setCharSex( const std::string& sex )
{
	m_sCharSex = sex;
}
bool Player::isGameOver() const
{
	return m_bGameOver;
}
int Player::getHealth() const
{
	return m_iHealth;
}
int Player::getStrength() const
{
	return m_iStrength;
}
int Player::getIntelligence() const
{
	return m_iIntelligence;
}
int Player::getMoney() const
{
	return m_iMoney;
}
int Player::getXp() const
{
	return m_iXp;
}
int Player::getLevel() const
{
	return static_cast<int>( m_iXp / 100.0f );
}
void Player::setGameOver( const bool val )
{
	m_bGameOver = val;
}
void Player::setHealth( const int val )
{
	m_iHealth = val;
}
void Player::setStrength( const int val )
{
	m_iStrength = val;
}
void Player::setIntelligence( const int val )
{
	m_iIntelligence = val;
}
void Player::setMoney( const int val )
{
	m_iMoney = val;
}
void Player::setXp( const int val )
{
	m_iXp = val;
}
void Player::addHealth( const int val )
{
	m_iHealth += val;
}
void Player::addStrength( const int val )
{
	m_iStrength += val;
}
void Player::addIntelligence( const int val )
{
	m_iIntelligence += val;
}
void Player::addMoney( const int val )
{
	m_iMoney += val;
}
void Player::addXp( const int val )
{
	m_iXp += val;
}
std::string Player::getEquippedWeapon() const
{
	return m_sEquippedWeapon;
}
void Player::setEquippedWeapon( const std::string& weaponName )
{
	m_sEquippedWeapon = weaponName;
}
Inventory Player::getInventory() const
{
	return m_Inventory;
}
void Player::addItem( const std::string& itemName, const int maxFirstTimeUses )
{
	auto newItem = loadItemData( itemName, "gameitemdata.txt" );

	if( m_Inventory.items.count( itemName ) )
		m_Inventory.items[ itemName ].uses += newItem.uses;
	else
	{
		m_Inventory.items[ itemName ] = newItem;
		if( maxFirstTimeUses != 0 )
			m_Inventory.items[ itemName ].uses = (newItem.uses > maxFirstTimeUses) ? maxFirstTimeUses : newItem.uses;
	}
}
void Player::addItemUses( const std::string& itemName, const int uses )
{
	if( m_Inventory.items.count( itemName ) )
		m_Inventory.items[ itemName ].uses += uses;
}
void Player::removeItem( const std::string& itemName )
{
	m_Inventory.items.erase( itemName );
}
void Player::addWeapon( const std::string& weaponName )
{
	if( !m_Inventory.weapons.count( weaponName ) )
		m_Inventory.weapons[ weaponName ] = loadWeaponData( weaponName, "gameitemdata.txt" );
}
void Player::removeWeapon( const std::string& weaponName )
{
	m_Inventory.weapons.erase( weaponName );
}
bool Player::hasItemCond( const std::string& condName ) const
{
	return m_ItemConds.count( condName ) != 0;
}
bool Player::hasLocCond( const std::string& condName ) const
{
	return m_LocConds.count( condName ) != 0;
}
std::unordered_set<std::string> Player::getItemConds() const
{
	return m_ItemConds;
}
std::unordered_set<std::string> Player::getLocConds() const
{
	return m_LocConds;
}
bool Player::findItemCond( const std::string& searchStr, std::string* result ) const
{
	for( const auto& cond : m_ItemConds )
	{
		if( cond.find( searchStr ) != std::string::npos )
		{
			if( result != nullptr )
				*result = cond;
			return true;
		}
	}
	return false;
}
bool Player::findLocCond( const std::string& searchStr, std::string* result ) const
{
	for( const auto& cond : m_LocConds )
	{
		if( cond.find( searchStr ) != std::string::npos )
		{
			if( result != nullptr )
				*result = cond;
			return true;
		}
	}
	return false;
}
void Player::addItemCond( const std::string& condName )
{
	m_ItemConds.insert( condName );
}
void Player::addLocCond( const std::string& condName )
{
	m_LocConds.insert( condName );
}
void Player::removeItemCond( const std::string& condName )
{
	m_ItemConds.erase( condName );
}
void Player::removeLocCond( const std::string& condName )
{
	m_LocConds.erase( condName );
}
void Player::clearConds()
{
	m_ItemConds.clear();
	m_LocConds.clear();
}
std::string Player::getCurrentLocation() const
{
	return m_sCurLocation;
}
std::string Player::getCurFile() const
{
	return m_sCurFile;
}
bool Player::hasBeenLocation( const std::string& locationName ) const
{
	return m_PrevLocations.count( locationName ) != 0;
}
std::string Player::getPrevLocationsString() const
{
	std::string result;
	for( const auto& loc : m_PrevLocations )
		result.append( loc + "," );
	return result;
}
void Player::setCurrentLocation( const std::string& locationName )
{
	m_sCurLocation = locationName;
}
void Player::setCurFile( const std::string& fileName )
{
	m_sCurFile = fileName;
}
void Player::addPrevLocation( const std::string& locationName )
{
	m_PrevLocations.insert( locationName );
}
bool Player::useItem( const std::string& itemName, std::string& resultLoc )
{
	if( m_Inventory.items.count( itemName ) )
	{
		if( m_Inventory.items[ itemName ].uses > 1 )
			--m_Inventory.items[ itemName ].uses;
		else
			m_Inventory.items.erase( itemName );

		resultLoc = "use" + itemName;
		return true;
	}
	return false;
}
void Player::initializeConds( const std::string& filename )
{
	std::ifstream locFile( "game/data/locations/" + filename );
	if( locFile.is_open() )
	{
		for( std::string line; getline( locFile, line ); )
		{
			if( line.find( "loccond: " ) != std::string::npos )
			{
				auto tempFull = line.substr( line.find( "loccond: " ) + 9, line.length() - line.find( '\n' ) );
				auto tempCond = tempFull.substr( 0, tempFull.find( ' ' ) );
				auto zeroCond = tempCond + " 0";
				if( !this->findLocCond( tempCond ) )
					this->addLocCond( zeroCond );
			}
			else if( line.find( "itemcond: " ) != std::string::npos )
			{
				auto tempFull = line.substr( line.find( "itemcond: " ) + 10, line.length() - line.find( '\n' ) );
				auto tempCond = tempFull.substr( 0, tempFull.find( ' ' ) );
				auto zeroCond = tempCond + " 0";
				if( !this->findItemCond( tempCond ) )
					this->addItemCond( zeroCond );
			}
		}
		locFile.close();
	}
	else
		logError( "Unable to open location data file for condition initialization!" );
}