#ifndef INVENTORY_H
#define INVENTORY_H
#pragma once

#include <string>
#include <unordered_map>

struct Item
{
	Item() : codeName(), prefix(), name(), desc(), weight( 0.0f ), worth( 0 ), healthCond( 0 ), strengthCond( 0 ), intelligenceCond( 0 ) {}
	virtual ~Item() = default;

	std::string codeName;
	std::string prefix;
	std::string name;
	std::string desc;
	float weight;
	int worth;
	int healthCond;
	int strengthCond;
	int intelligenceCond;
};

struct ItemConsumable final : public Item
{
	ItemConsumable() : damageType(), isBattleItem( false ), weight( 0.0f ), damage( 0 ), uses( 0 ) {}

	std::string damageType;
	bool isBattleItem;
	float weight;
	int damage;
	int uses;
};

struct Weapon final : public Item
{
	Weapon() : damage( 0 ) {}

	int damage;
};

struct Inventory final
{
	Inventory() : items(), weapons() {}

	std::unordered_map<std::string, ItemConsumable> items;
	std::unordered_map<std::string, Weapon> weapons;
};

struct Shop final
{
	Shop() : buyMultiplier( 0 ), name(), items(), itemPrices(), nItems(), weapons(), weaponPrices(), nWeapons() {}

	double buyMultiplier;
	std::string name;
	std::vector<ItemConsumable> items;
	std::vector<int> itemPrices;
	std::vector<int> nItems;
	std::vector<Weapon> weapons;
	std::vector<int> weaponPrices;
	std::vector<int> nWeapons;
};

#endif