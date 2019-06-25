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

static void clear_equip_state(CMsgClientWelcome::SubscribedType& object);
static void apply_medals(CMsgClientWelcome::SubscribedType& object);
static void apply_music_kits(CMsgClientWelcome::SubscribedType& object);
static void add_all_items(CMsgClientWelcome::SubscribedType& object);
static void add_item(CMsgClientWelcome::SubscribedType& object, int index, ItemDefinitionIndex itemIndex, int rarity, int paintKit, int seed, float wear, std::string name);
static TeamID GetAvailableClassID(int definition_index);
static int GetSlotID(int definition_index);
static std::vector<uint32_t> music_kits = { 3, 4, 5, 6, 7, 8 };


template<typename T>
inline std::string get_4bytes(T value)
{
	return std::string{ reinterpret_cast<const char*>( reinterpret_cast<void*>(&value) ), 4 };
}

template<typename T>
inline CSOEconItemAttribute make_econ_item_attribute(int def_index, T value)
{
	CSOEconItemAttribute attribute;
	attribute.def_index().set(def_index);
	attribute.value_bytes().set(get_4bytes(value));
	return attribute;
}

inline CSOEconItemEquipped make_equipped_state(int team, int slot)
{
	CSOEconItemEquipped equipped_state;
	equipped_state.new_class().set(team);
	equipped_state.new_slot().set(slot);
	return equipped_state;
}

static std::string inventory_changer(void *pubDest, uint32_t *pcubMsgSize) {
	CMsgClientWelcome msg((void*)((DWORD)pubDest + 8), *pcubMsgSize - 8);
	if (!msg.outofdate_subscribed_caches().has())
		  return msg.serialize();

	auto cache = msg.outofdate_subscribed_caches().get();

  static auto fix_null_inventory = [&cache]()
  {
    auto objects = cache.objects().get_all();
    auto it = std::find_if(objects.begin(), objects.end(), [](decltype(objects.front()) o)
    {
      return o.type_id().has() && o.type_id().get() == 1;
    });

    // inventory not exist, need create
    if (it == objects.end()) 
	  {
      CMsgClientWelcome::SubscribedType null_object;
		  null_object.type_id().set(1);
		  cache.objects().add(null_object);
	  }
  };

  // If not have items in inventory, Create null inventory
  fix_null_inventory();

	// Add custom items
	auto objects = cache.objects().get_all();
	for (size_t i = 0; i < objects.size(); i++) {
		auto object = objects[i];

		if (!object.type_id().has())
			continue;

		switch (object.type_id().get())
		{
			case 1: // Inventory
			{
				if (true) //g_Options.skins_packet_clear_default_items
					object.object_data().clear();

				clear_equip_state(object);
				apply_medals(object);
				apply_music_kits(object);
				add_all_items(object);
				cache.objects().set(object, i);
			}
			break;
		}
	}
	msg.outofdate_subscribed_caches().set(cache);
	
	return msg.serialize();
}

static bool inventory_changer_presend(void* pubData, uint32_t &cubData)
{
	CMsgAdjustItemEquippedState msg((void*)((DWORD)pubData + 8), cubData - 8);
	// Change music kit check
	if (msg.item_id().has() && (msg.new_class().get() == 0 || msg.new_slot().get() == 54))
	{
		auto ItemIndex = msg.item_id().get() - START_MUSICKIT_INDEX;

		  if (ItemIndex > 38 || ItemIndex < 3)
			  return true;

		  /*g_Options.skins_packets_musci_kit = */msg.new_slot().get() == 0xFFFF ? 0 : ItemIndex - 2;

		return false;
	}
	// Change weapon skin check
	if (!msg.item_id().has() || !msg.new_class().get() || !msg.new_slot().get())
		return true;

	return false;
}


static void clear_equip_state(CMsgClientWelcome::SubscribedType& object)
  {
	  auto object_data = object.object_data().get_all();
	  for (size_t j = 0; j < object_data.size(); j++)
	  {
		  auto item = object_data[j];

		  if (!item.equipped_state().has())
			  continue;

		  // create NOT equiped state for item
		  auto null_equipped_state = make_equipped_state(0, 0);
		  
		  // unequip all
		  auto equipped_state = item.equipped_state().get_all();
		  for (size_t k = 0; k < equipped_state.size(); k++)
			  item.equipped_state().set(null_equipped_state, k);

		  object.object_data().set(item, j);
	  }
  }

std::vector<uint32_t> packets_medals = { 1372, 958, 957, 956, 955 };
int packets_equipped_medal = 874;

static void apply_medals(CMsgClientWelcome::SubscribedType& object)
{
	uint32_t steamid = g_SteamUser->GetSteamID().GetAccountID();

	CSOEconItem medal;
	medal.account_id().set(steamid);
	medal.origin().set(9);
	medal.rarity().set(6);
	medal.quantity().set(1);
	medal.quality().set(4);
	medal.level().set(1);

	// Time acquired attribute
	medal.attribute().set(make_econ_item_attribute(222, (uint32_t)std::time(0)));

	int i = 10000;
	for (uint32_t MedalIndex : packets_medals)
	{
		medal.def_index().set(MedalIndex);
		medal.inventory().set(i);
		medal.id().set(i);
		object.object_data().add(medal);
		i++;
	}

	if (packets_equipped_medal)
	{
		medal.def_index().set(packets_equipped_medal);
		medal.inventory().set(i);
		medal.id().set(i);
		medal.equipped_state().set(make_equipped_state(0, 55));
		object.object_data().add(medal);
	}
}

static void apply_music_kits(CMsgClientWelcome::SubscribedType& object)
{
	uint32_t steamid = g_SteamUser->GetSteamID().GetAccountID();

	CSOEconItem music_kit;
	music_kit.account_id().set(steamid);
	music_kit.origin().set(9);
	music_kit.rarity().set(6);
	music_kit.quantity().set(1);
	music_kit.quality().set(4);
	music_kit.level().set(1);
	music_kit.flags().set(0);
	music_kit.def_index().set(1314);

	// Time acquired attribute
	music_kit.attribute().add(make_econ_item_attribute(75, (uint32_t)std::time(0)));

	int selected_musickit_gui = 16;
	for (int i = 3; i <= 38; ++i)
	{
		if (selected_musickit_gui != i)
		{
			music_kit.attribute().add(make_econ_item_attribute(166, i)); // Music kit id
			music_kit.inventory().set(START_MUSICKIT_INDEX + i);
			music_kit.id().set(START_MUSICKIT_INDEX + i);
			object.object_data().add( music_kit );
		}
	}

	if (selected_musickit_gui)
	{
		music_kit.attribute().add(make_econ_item_attribute(166, selected_musickit_gui)); // Music kit id
		music_kit.inventory().set(START_MUSICKIT_INDEX + selected_musickit_gui);
		music_kit.id().set(START_MUSICKIT_INDEX + selected_musickit_gui);
		music_kit.equipped_state().set(make_equipped_state(0, 54));
		object.object_data().add( music_kit );
	}
}

static void add_all_items(CMsgClientWelcome::SubscribedType& object)
{
	int l = 1;
	for (auto& x : g_skins) {
		add_item(object, l, (ItemDefinitionIndex)x.wId, 6, x.paintkit, 0, FLT_MIN, "unknowncheats.me");
		l++;
	}
}

static void add_item(CMsgClientWelcome::SubscribedType& object, int index, ItemDefinitionIndex itemIndex, int rarity, int paintKit, int seed, float wear, std::string name)
{
	uint32_t steamid = g_SteamUser->GetSteamID().GetAccountID();

	CSOEconItem item;
	item.id().set(START_ITEM_INDEX + itemIndex);
	item.account_id().set(steamid);
	item.def_index().set(itemIndex);
	item.inventory().set(START_ITEM_INDEX + index);
	item.origin().set(24);
	item.quantity().set(1);
	item.level().set(1);
	item.style().set(0);
	item.flags().set(0);
	item.in_use().set(true);
	item.original_id().set(0);
	item.rarity().set(rarity);
	item.quality().set(0);

	if (name.size() > 0)
		item.custom_name().set(name);

	// Add equipped state for both teams
	TeamID avalTeam = GetAvailableClassID(itemIndex);

	if (avalTeam == TeamID::TEAM_SPECTATOR || avalTeam == TeamID::TEAM_TERRORIST) {
		item.equipped_state().add( make_equipped_state(TEAM_TERRORIST, GetSlotID(itemIndex)) );
	}
	if (avalTeam == TeamID::TEAM_SPECTATOR || avalTeam == TeamID::TEAM_COUNTER_TERRORIST) {
		item.equipped_state().add( make_equipped_state(TEAM_COUNTER_TERRORIST, GetSlotID(itemIndex)) );
	}

	// Add CSOEconItemAttribute's
	item.attribute().add(make_econ_item_attribute(6, float(paintKit)));
	item.attribute().add(make_econ_item_attribute(7, float(seed)));
	item.attribute().add(make_econ_item_attribute(8, float(wear)));

	// Time acquired attribute
	item.attribute().add(make_econ_item_attribute(180, (uint32_t)std::time(0)));

	// Stickers
	for (int j = 0; j < 4; j++)
	{
		item.attribute().add(make_econ_item_attribute(113 + 4 * j, uint32_t(289 + j))); // Sticker Kit
		item.attribute().add(make_econ_item_attribute(114 + 4 * j, float(0.001f)));     // Sticker Wear
		item.attribute().add(make_econ_item_attribute(115 + 4 * j, float(1.f)));        // Sticker Scale
		item.attribute().add(make_econ_item_attribute(116 + 4 * j, float(0.f)));        // Sticker Rotation
	}

	object.object_data().add(item);
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