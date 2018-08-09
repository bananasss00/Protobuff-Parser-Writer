#pragma once
#include "ProtoParse.h"
#include "ProtobuffMessages.h"
#include <ctime>

struct wskin
{
	int wId;
	int paintkit;
};

std::vector<wskin> g_skins = {
	{ 508,622 },
	{ 5027,10007 },
	{ 1,527 },
	{ 2,544 },
	{ 3,387 },
	{ 4,586 },
	{ 32,338 },
	{ 36,404 },
	{ 61,653 },
	{ 63,270 },
	{ 30,652 },
	{ 7,656 },
	{ 8,305 },
	{ 10,260 },
	{ 13,629 },
	{ 14,401 },
	{ 16,632 },
	{ 60,383 },
	{ 17,433 },
	{ 19,283 },
	{ 24,688 },
	{ 25,616 },
	{ 26,306 },
	{ 27,198 },
	{ 28,483 },
	{ 29,434 },
	{ 33,481 },
	{ 34,262 },
	{ 35,450 },
	{ 39,311 },
	{ 38,232 },
	{ 11,628 },
	{ 9,344 },
	{ 40,538 }
};

#define START_MUSICKIT_INDEX 1500000
#define START_ITEM_INDEX     2000000

static void fix_null_inventory(CMsgSOCacheSubscribed& cache);
static void clear_equip_state(SubscribedType& object);
static void apply_medals(SubscribedType& object);
static void apply_music_kits(SubscribedType& object);
static void add_all_items(SubscribedType& object);
static void add_item(SubscribedType& object, int index, ItemDefinitionIndex itemIndex, int rarity, int paintKit, int seed, float wear, std::string name);
static TeamID GetAvailableClassID(int definition_index);
static int GetSlotID(int definition_index);
static std::vector<uint32_t> music_kits = { 3, 4, 5, 6, 7, 8 };


template<typename T>
inline std::string get_4bytes(T value)
{
	return std::string{ reinterpret_cast<const char*>( reinterpret_cast<void*>(&value) ), 4 };
}

template<typename T>
inline std::string make_econ_item_attribute(int def_index, T value)
{
	CSOEconItemAttribute attribute;
	attribute.add_def_index(def_index);
	attribute.add_value_bytes(get_4bytes(value));
	return attribute.serialize();
}

inline std::string make_equipped_state(int team, int slot)
{
	CSOEconItemEquipped equipped_state;
	equipped_state.add_new_class(team);
	equipped_state.add_new_slot(slot);
	return equipped_state.serialize();
}

static std::string inventory_changer(void *pubDest, uint32_t *pcubMsgSize) {
	CMsgClientWelcome msg((void*)((DWORD)pubDest + 8), *pcubMsgSize - 8);
	if (!msg.has_outofdate_subscribed_caches())
		return msg.serialize();

	CMsgSOCacheSubscribed cache = msg.get_outofdate_subscribed_caches<CMsgSOCacheSubscribed>();
	// If not have items in inventory, Create null inventory
	fix_null_inventory(cache); 
	// Add custom items
	auto objects = cache.getAll_objects();
	for (size_t i = 0; i < objects.size(); i++)
	{
		SubscribedType object(objects[i].String());

		if (!object.has_type_id())
			continue;

		switch (object.get_type_id().Int32())
		{
			case 1: // Inventory
			{
				if (true) //g_Options.skins_packet_clear_default_items
					object.clear_object_data();

				clear_equip_state(object);
				apply_medals(object);
				apply_music_kits(object);
				add_all_items(object);
				cache.replace_objects(object.serialize(), i);
			}
			break;
		}
	}
	msg.replace_outofdate_subscribed_caches(cache.serialize(), 0);
	
	return msg.serialize();
}

static bool inventory_changer_presend(void* pubData, uint32_t &cubData)
{
	CMsgAdjustItemEquippedState msg((void*)((DWORD)pubData + 8), cubData - 8);
	// Change music kit check
	if (msg.has_item_id()
		&& msg.get_new_class().UInt32() == 0
		|| msg.get_new_slot().UInt32() == 54)
	{
		auto ItemIndex = msg.get_item_id().UInt64() - START_MUSICKIT_INDEX;

		if (ItemIndex > 38 || ItemIndex < 3)
			return true;

		/*g_Options.skins_packets_musci_kit*/auto skins_packets_musci_kit = msg.get_new_slot().UInt32() == 0xFFFF ? 0 : ItemIndex - 2;

		return false;
	}
	// Change weapon skin check
	if (!msg.has_item_id()
		|| !msg.has_new_class()
		|| !msg.has_new_slot())
		return true;

	return false;
}

static void fix_null_inventory(CMsgSOCacheSubscribed& cache)
{
	bool inventory_exist = false;
	auto objects = cache.getAll_objects();
	for (size_t i = 0; i < objects.size(); i++)
	{
		SubscribedType object(objects[i].String());
		if (!object.has_type_id())
			continue;
		if (object.get_type_id().Int32() != 1)
			continue;
		inventory_exist = true;
		break;
	}
	if (!inventory_exist)
	{
		int cache_size = objects.size();
		SubscribedType null_object;
		null_object.add_type_id(1);

		cache.add_objects( null_object.serialize() );
	}
}

static void clear_equip_state(SubscribedType& object)
{
	auto object_data = object.getAll_object_data();
	for (size_t j = 0; j < object_data.size(); j++)
	{
		CSOEconItem item( object_data[j].String() );

		if (!item.has_equipped_state())
			continue;

		// create NOT equiped state for item
		auto null_equipped_state = make_equipped_state(0, 0);
		
		// unequip all
		auto equipped_state = item.getAll_equipped_state();
		for (size_t k = 0; k < equipped_state.size(); k++)
			item.replace_equipped_state(null_equipped_state, k);

		object.replace_object_data(item.serialize(), j);
	}
}

std::vector<uint32_t> packets_medals = { 1372, 958, 957, 956, 955 };
int packets_equipped_medal = 874;

static void apply_medals(SubscribedType& object)
{
	uint32_t steamid = g_SteamUser->GetSteamID().GetAccountID();

	CSOEconItem medal;
	medal.add_account_id(steamid);
	medal.add_origin(9);
	medal.add_rarity(6);
	medal.add_quantity(1);
	medal.add_quality(4);
	medal.add_level(1);

	// Time acquired attribute
	medal.add_attribute(make_econ_item_attribute(222, (uint32_t)std::time(0)));

	int i = 10000;
	for (uint32_t MedalIndex : packets_medals)
	{
		medal.add_def_index(MedalIndex);
		medal.add_inventory(i);
		medal.add_id(i);
		object.add_object_data(medal.serialize());
		i++;
	}

	if (packets_equipped_medal)
	{
		medal.add_def_index(packets_equipped_medal);
		medal.add_inventory(i);
		medal.add_id(i);
		medal.add_equipped_state(make_equipped_state(0, 55));
		object.add_object_data(medal.serialize());
	}
}

static void apply_music_kits(SubscribedType& object)
{
	uint32_t steamid = g_SteamUser->GetSteamID().GetAccountID();

	CSOEconItem music_kit;
	music_kit.add_account_id(steamid);
	music_kit.add_origin(9);
	music_kit.add_rarity(6);
	music_kit.add_quantity(1);
	music_kit.add_quality(4);
	music_kit.add_level(1);
	music_kit.add_flags(0);
	music_kit.add_def_index(1314);

	// Time acquired attribute
	music_kit.add_attribute(make_econ_item_attribute(75, (uint32_t)std::time(0)));

	int selected_musickit_gui = 16;
	for (int i = 3; i <= 38; ++i)
	{
		if (selected_musickit_gui != i)
		{
			music_kit.add_attribute(make_econ_item_attribute(166, i)); // Music kit id
			music_kit.add_inventory(START_MUSICKIT_INDEX + i);
			music_kit.add_id(START_MUSICKIT_INDEX + i);
			object.add_object_data( music_kit.serialize() );
		}
	}

	if (selected_musickit_gui)
	{
		music_kit.add_attribute(make_econ_item_attribute(166, selected_musickit_gui)); // Music kit id
		music_kit.add_inventory(START_MUSICKIT_INDEX + selected_musickit_gui);
		music_kit.add_id(START_MUSICKIT_INDEX + selected_musickit_gui);
		music_kit.add_equipped_state(make_equipped_state(0, 54));
		object.add_object_data(music_kit.serialize());
	}
}

static void add_all_items(SubscribedType& object)
{
	int l = 1;
	for (auto& x : g_skins) {
		add_item(object, l, (ItemDefinitionIndex)x.wId, 6, x.paintkit, 0, FLT_MIN, "unknowncheats.me");
		l++;
	}
}

static void add_item(SubscribedType& object, int index, ItemDefinitionIndex itemIndex, int rarity, int paintKit, int seed, float wear, std::string name)
{
	uint32_t steamid = g_SteamUser->GetSteamID().GetAccountID();

	CSOEconItem item;
	item.add_id(START_ITEM_INDEX + itemIndex);
	item.add_account_id(steamid);
	item.add_def_index(itemIndex);
	item.add_inventory(START_ITEM_INDEX + index);
	item.add_origin(24);
	item.add_quantity(1);
	item.add_level(1);
	item.add_style(0);
	item.add_flags(0);
	item.add_in_use(true);
	item.add_original_id(0);
	item.add_rarity(rarity);
	item.add_quality(0);

	if (name.size() > 0)
		item.add_custom_name(name);

	// Add equipped state for both teams
	TeamID avalTeam = GetAvailableClassID(itemIndex);

	if (avalTeam == TeamID::TEAM_SPECTATOR || avalTeam == TeamID::TEAM_TERRORIST) {
		item.add_equipped_state( make_equipped_state(TEAM_TERRORIST, GetSlotID(itemIndex)) );
	}
	if (avalTeam == TeamID::TEAM_SPECTATOR || avalTeam == TeamID::TEAM_COUNTER_TERRORIST) {
		item.add_equipped_state( make_equipped_state(TEAM_COUNTER_TERRORIST, GetSlotID(itemIndex)) );
	}

	// Add CSOEconItemAttribute's
	item.add_attribute(make_econ_item_attribute(6, float(paintKit)));
	item.add_attribute(make_econ_item_attribute(7, float(seed)));
	item.add_attribute(make_econ_item_attribute(8, float(wear)));

	// Time acquired attribute
	item.add_attribute(make_econ_item_attribute(180, (uint32_t)std::time(0)));

	// Stickers
	for (int j = 0; j < 4; j++)
	{
		item.add_attribute(make_econ_item_attribute(113 + 4 * j, uint32_t(289 + j))); // Sticker Kit
		item.add_attribute(make_econ_item_attribute(114 + 4 * j, float(0.001f)));     // Sticker Wear
		item.add_attribute(make_econ_item_attribute(115 + 4 * j, float(1.f)));        // Sticker Scale
		item.add_attribute(make_econ_item_attribute(116 + 4 * j, float(0.f)));        // Sticker Rotation
	}

	object.add_object_data(item.serialize());
}

static TeamID GetAvailableClassID(int definition_index)
{
	switch (definition_index)
	{
	case WEAPON_KNIFE_BAYONET:
	case WEAPON_KNIFE_FLIP:
	case WEAPON_KNIFE_GUT:
	case WEAPON_KNIFE_KARAMBIT:
	case WEAPON_KNIFE_M9_BAYONET:
	case WEAPON_KNIFE_TACTICAL:
	case WEAPON_KNIFE_FALCHION:
	case WEAPON_KNIFE_SURVIVAL_BOWIE:
	case WEAPON_KNIFE_BUTTERFLY:
	case WEAPON_KNIFE_PUSH:
	case WEAPON_ELITE:
	case WEAPON_P250:
	case WEAPON_CZ75A:
	case WEAPON_DEAGLE:
	case WEAPON_REVOLVER:
	case WEAPON_MP7:
	case WEAPON_UMP45:
	case WEAPON_P90:
	case WEAPON_BIZON:
	case WEAPON_SSG08:
	case WEAPON_AWP:
	case WEAPON_NOVA:
	case WEAPON_XM1014:
	case WEAPON_M249:
	case WEAPON_NEGEV:
	case GLOVE_STUDDED_BLOODHOUND:
	case GLOVE_SPORTY:
	case GLOVE_SLICK:
	case GLOVE_LEATHER_WRAP:
	case GLOVE_MOTORCYCLE:
	case GLOVE_SPECIALIST:
		return TEAM_SPECTATOR;

	case WEAPON_GLOCK:
	case WEAPON_AK47:
	case WEAPON_MAC10:
	case WEAPON_G3SG1:
	case WEAPON_TEC9:
	case WEAPON_GALILAR:
	case WEAPON_SG553:
	case WEAPON_SAWEDOFF:
		return TEAM_TERRORIST;

	case WEAPON_AUG:
	case WEAPON_FAMAS:
	case WEAPON_MAG7:
	case WEAPON_FIVESEVEN:
	case WEAPON_USP_SILENCER:
	case WEAPON_HKP2000:
	case WEAPON_MP9:
	case WEAPON_M4A1_SILENCER:
	case WEAPON_M4A1:
	case WEAPON_SCAR20:
		return TEAM_COUNTER_TERRORIST;

	default:
		return TEAM_UNASSIGNED;
	}
}

static int GetSlotID(int definition_index)
{
	switch (definition_index)
	{
	case WEAPON_KNIFE_BAYONET:
	case WEAPON_KNIFE_FLIP:
	case WEAPON_KNIFE_GUT:
	case WEAPON_KNIFE_KARAMBIT:
	case WEAPON_KNIFE_M9_BAYONET:
	case WEAPON_KNIFE_TACTICAL:
	case WEAPON_KNIFE_FALCHION:
	case WEAPON_KNIFE_SURVIVAL_BOWIE:
	case WEAPON_KNIFE_BUTTERFLY:
	case WEAPON_KNIFE_PUSH:
		return 0;
	case WEAPON_USP_SILENCER:
	case WEAPON_HKP2000:
	case WEAPON_GLOCK:
		return 2;
	case WEAPON_ELITE:
		return 3;
	case WEAPON_P250:
		return 4;
	case WEAPON_TEC9:
	case WEAPON_CZ75A:
	case WEAPON_FIVESEVEN:
		return 5;
	case WEAPON_DEAGLE:
	case WEAPON_REVOLVER:
		return 6;
	case WEAPON_MP9:
	case WEAPON_MAC10:
		return 8;
	case WEAPON_MP7:
		return 9;
	case WEAPON_UMP45:
		return 10;
	case WEAPON_P90:
		return 11;
	case WEAPON_BIZON:
		return 12;
	case WEAPON_FAMAS:
	case WEAPON_GALILAR:
		return 14;
	case WEAPON_M4A1_SILENCER:
	case WEAPON_M4A1:
	case WEAPON_AK47:
		return 15;
	case WEAPON_SSG08:
		return 16;
	case WEAPON_SG553:
	case WEAPON_AUG:
		return 17;
	case WEAPON_AWP:
		return 18;
	case WEAPON_G3SG1:
	case WEAPON_SCAR20:
		return 19;
	case WEAPON_NOVA:
		return 20;
	case WEAPON_XM1014:
		return 21;
	case WEAPON_SAWEDOFF:
	case WEAPON_MAG7:
		return 22;
	case WEAPON_M249:
		return 23;
	case WEAPON_NEGEV:
		return 24;
	case GLOVE_STUDDED_BLOODHOUND:
	case GLOVE_SPORTY:
	case GLOVE_SLICK:
	case GLOVE_LEATHER_WRAP:
	case GLOVE_MOTORCYCLE:
	case GLOVE_SPECIALIST:
		return 41;
	default:
		return -1;
	}
}