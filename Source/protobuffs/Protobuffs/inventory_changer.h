#pragma once
#include "ProtoWriter.h"
#include "ProtobuffMessages.h"

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

static void fix_null_inventory(ProtoWriter& cache);
static void clear_equip_state(ProtoWriter& object);
static void apply_medals(ProtoWriter& object);
static void apply_music_kits(ProtoWriter& object);
static void add_all_items(ProtoWriter& object);
static void add_item(ProtoWriter& object, int index, ItemDefinitionIndex itemIndex, int rarity, int paintKit, int seed, float wear, std::string name);
static TeamID GetAvailableClassID(int definition_index);
static int GetSlotID(int definition_index);
static std::vector<uint32_t> music_kits = { 3, 4, 5, 6, 7, 8 };

static std::string inventory_changer(void *pubDest, uint32_t *pcubMsgSize) {
	ProtoWriter msg((void*)((DWORD)pubDest + 8), *pcubMsgSize - 8, 11);
	if (msg.getAll(CMsgClientWelcome::outofdate_subscribed_caches).empty())
		return msg.serialize();

	ProtoWriter cache(msg.get(CMsgClientWelcome::outofdate_subscribed_caches).String(), 4);
	// If not have items in inventory, Create null inventory
	fix_null_inventory(cache); 
	// Add custom items
	auto objects = cache.getAll(CMsgSOCacheSubscribed::objects);
	for (int i = 0; i < objects.size(); i++)
	{
		ProtoWriter object(objects[i].String(), 2);

		if (!object.has(SubscribedType::type_id))
			continue;

		switch (object.get(SubscribedType::type_id).Int32())
		{
			case 1: // Inventory
			{
				if (true) //g_Options.skins_packet_clear_default_items
					object.clear(SubscribedType::object_data);

				clear_equip_state(object);
				apply_medals(object);
				apply_music_kits(object);
				add_all_items(object);
				cache.replace(Field(CMsgSOCacheSubscribed::objects, TYPE_STRING, object.serialize()), i);
			}
			break;
		}
	}
	msg.replace(Field(CMsgClientWelcome::outofdate_subscribed_caches, TYPE_STRING, cache.serialize()), 0);
	
	return msg.serialize();
}

static bool inventory_changer_presend(void* pubData, uint32_t &cubData)
{
	ProtoWriter msg((void*)((DWORD)pubData + 8), cubData - 8, 19);
	// Change music kit check
	if (msg.has(CMsgAdjustItemEquippedState::item_id)
		&& msg.get(CMsgAdjustItemEquippedState::new_class).UInt32() == 0
		|| msg.get(CMsgAdjustItemEquippedState::new_slot).UInt32() == 54)
	{
		int ItemIndex = /*(uint32_t)*/msg.get(CMsgAdjustItemEquippedState::item_id).UInt64() - START_MUSICKIT_INDEX;

		if (ItemIndex > 38 || ItemIndex < 3)
			return true;

		/*g_Options.skins_packets_musci_kit*/auto skins_packets_musci_kit = msg.get(CMsgAdjustItemEquippedState::new_slot).UInt32() == 65535 ? 0 : ItemIndex - 2;

		return false;
	}
	// Change weapon skin check
	if (!msg.has(CMsgAdjustItemEquippedState::item_id)
		|| !msg.has(CMsgAdjustItemEquippedState::new_class)
		|| !msg.has(CMsgAdjustItemEquippedState::new_slot))
		return true;

	return false;
}

static void fix_null_inventory(ProtoWriter& cache)
{
	bool inventory_exist = false;
	auto objects = cache.getAll(CMsgSOCacheSubscribed::objects);
	for (int i = 0; i < objects.size(); i++)
	{
		ProtoWriter object(objects[i].String(), 2);
		if (!object.has(SubscribedType::type_id))
			continue;
		if (object.get(SubscribedType::type_id).Int32() != 1)
			continue;
		inventory_exist = true;
		break;
	}
	if (!inventory_exist)
	{
		int cache_size = objects.size();
		ProtoWriter null_object(2);
		null_object.add(Field(SubscribedType::type_id, TYPE_INT32, (int64_t)1));

		cache.add(Field(CMsgSOCacheSubscribed::objects, TYPE_STRING, null_object.serialize()));
	}
}

static void clear_equip_state(ProtoWriter& object)
{
	auto object_data = object.getAll(SubscribedType::object_data);
	for (int j = 0; j < object_data.size(); j++)
	{
		ProtoWriter item(object_data[j].String(), 19);

		if (item.getAll(CSOEconItem::equipped_state).empty())
			continue;

		// create NOT equiped state for item
		ProtoWriter null_equipped_state(2);
		null_equipped_state.replace(Field(CSOEconItemEquipped::new_class, TYPE_UINT32, (int64_t)0));
		null_equipped_state.replace(Field(CSOEconItemEquipped::new_slot, TYPE_UINT32, (int64_t)0));
		// unequip all
		auto equipped_state = item.getAll(CSOEconItem::equipped_state);
		for (int k = 0; k < equipped_state.size(); k++)
			item.replace(Field(CSOEconItem::equipped_state, TYPE_STRING, null_equipped_state.serialize()), k);

		object.replace(Field(SubscribedType::object_data, TYPE_STRING, item.serialize()), j);
	}
}

std::vector<uint32_t> packets_medals = { 1372, 958, 957, 956, 955 };
int packets_equipped_medal = 874;

static void apply_medals(ProtoWriter& object)
{
	uint32_t steamid = g_SteamUser->GetSteamID().GetAccountID();

	ProtoWriter medal(19);
	medal.add(Field(CSOEconItem::account_id, TYPE_UINT32, (int64_t)steamid));
	medal.add(Field(CSOEconItem::origin, TYPE_UINT32, (int64_t)9));
	medal.add(Field(CSOEconItem::rarity, TYPE_UINT32, (int64_t)6));
	medal.add(Field(CSOEconItem::quantity, TYPE_UINT32, (int64_t)1));
	medal.add(Field(CSOEconItem::quality, TYPE_UINT32, (int64_t)4));
	medal.add(Field(CSOEconItem::level, TYPE_UINT32, (int64_t)1));

	ProtoWriter time_acquired_attribute(3);
	time_acquired_attribute.add(Field(CSOEconItemAttribute::def_index, TYPE_UINT32, (int64_t)222));
	time_acquired_attribute.add(Field(CSOEconItemAttribute::value_bytes, TYPE_STRING, std::string("\x00\x00\x00\x00")));
	medal.add(Field(CSOEconItem::attribute, TYPE_STRING, time_acquired_attribute.serialize()));

	int i = 10000;
	for (uint32_t MedalIndex : packets_medals)
	{
		medal.add(Field(CSOEconItem::def_index, TYPE_UINT32, (int64_t)MedalIndex));
		medal.add(Field(CSOEconItem::inventory, TYPE_UINT32, (int64_t)i));
		medal.add(Field(CSOEconItem::id, TYPE_UINT64, (int64_t)i));
		object.add(Field(SubscribedType::object_data, TYPE_STRING, medal.serialize()));
		i++;
	}

	if (packets_equipped_medal)
	{
		medal.add(Field(CSOEconItem::def_index, TYPE_UINT32, (int64_t)packets_equipped_medal));
		medal.add(Field(CSOEconItem::inventory, TYPE_UINT32, (int64_t)i));
		medal.add(Field(CSOEconItem::id, TYPE_UINT64, (int64_t)i));

		ProtoWriter equipped_state(2);
		equipped_state.add(Field(CSOEconItemEquipped::new_class, TYPE_UINT32, (int64_t)0));
		equipped_state.add(Field(CSOEconItemEquipped::new_slot, TYPE_UINT32, (int64_t)55));
		medal.add(Field(CSOEconItem::equipped_state, TYPE_STRING, equipped_state.serialize()));
		object.add(Field(SubscribedType::object_data, TYPE_STRING, medal.serialize()));
	}
}

static void apply_music_kits(ProtoWriter& object)
{
	uint32_t steamid = g_SteamUser->GetSteamID().GetAccountID();

	ProtoWriter music_kit(19);
	music_kit.add(Field(CSOEconItem::account_id, TYPE_UINT32, (int64_t)steamid));
	music_kit.add(Field(CSOEconItem::origin, TYPE_UINT32, (int64_t)9));
	music_kit.add(Field(CSOEconItem::rarity, TYPE_UINT32, (int64_t)6));
	music_kit.add(Field(CSOEconItem::quantity, TYPE_UINT32, (int64_t)1));
	music_kit.add(Field(CSOEconItem::quality, TYPE_UINT32, (int64_t)4));
	music_kit.add(Field(CSOEconItem::level, TYPE_UINT32, (int64_t)1));
	music_kit.add(Field(CSOEconItem::flags, TYPE_UINT32, (int64_t)0));
	music_kit.add(Field(CSOEconItem::def_index, TYPE_UINT32, (int64_t)1314));

	ProtoWriter time_acquired_attribute(3);
	time_acquired_attribute.add(Field(CSOEconItemAttribute::def_index, TYPE_UINT32, (int64_t)75));
	time_acquired_attribute.add(Field(CSOEconItemAttribute::value_bytes, TYPE_STRING, std::string("\x00\x00\x00\x00")));
	music_kit.add(Field(CSOEconItem::attribute, TYPE_STRING, time_acquired_attribute.serialize()));

	int selected_musickit_gui = 16;
	for (int i = 3; i <= 38; ++i)
	{
		if (selected_musickit_gui != i)
		{
			uint32_t musikkit_id_value = i;
			auto kit_id = std::string{ reinterpret_cast<const char*>((void*)&musikkit_id_value), 4 };
			ProtoWriter musikkit_id(3);
			musikkit_id.add(Field(CSOEconItemAttribute::def_index, TYPE_UINT32, (int64_t)166));
			musikkit_id.add(Field(CSOEconItemAttribute::value_bytes, TYPE_STRING, kit_id));
			music_kit.add(Field(CSOEconItem::attribute, TYPE_STRING, musikkit_id.serialize()));

			music_kit.add(Field(CSOEconItem::inventory, TYPE_UINT32, (int64_t)(START_MUSICKIT_INDEX + i)));
			music_kit.add(Field(CSOEconItem::id, TYPE_UINT64, (int64_t)(START_MUSICKIT_INDEX + i)));
			object.add(Field(SubscribedType::object_data, TYPE_STRING, music_kit.serialize()));
		}
	}

	if (selected_musickit_gui)
	{
		uint32_t musikkit_id_value = selected_musickit_gui;
		auto kit_id = std::string{ reinterpret_cast<const char*>((void*)&musikkit_id_value), 4 };
		ProtoWriter musikkit_id(3);
		musikkit_id.add(Field(CSOEconItemAttribute::def_index, TYPE_UINT32, (int64_t)166));
		musikkit_id.add(Field(CSOEconItemAttribute::value_bytes, TYPE_STRING, kit_id));
		music_kit.add(Field(CSOEconItem::attribute, TYPE_STRING, musikkit_id.serialize()));

		music_kit.add(Field(CSOEconItem::inventory, TYPE_UINT32, (int64_t)(START_MUSICKIT_INDEX + selected_musickit_gui)));
		music_kit.add(Field(CSOEconItem::id, TYPE_UINT64, (int64_t)(START_MUSICKIT_INDEX + selected_musickit_gui)));

		ProtoWriter equipped_state(2);
		equipped_state.add(Field(CSOEconItemEquipped::new_class, TYPE_UINT32, (int64_t)0));
		equipped_state.add(Field(CSOEconItemEquipped::new_slot, TYPE_UINT32, (int64_t)54));
		music_kit.add(Field(CSOEconItem::equipped_state, TYPE_STRING, equipped_state.serialize()));

		object.add(Field(SubscribedType::object_data, TYPE_STRING, music_kit.serialize()));
	}
}

static void add_all_items(ProtoWriter& object)
{
	int l = 1;
	for (auto& x : g_skins) {
		add_item(object, l, (ItemDefinitionIndex)x.wId, 6, x.paintkit, 0, FLT_MIN, "");
		l++;
	}
}

static void add_item(ProtoWriter& object, int index, ItemDefinitionIndex itemIndex, int rarity, int paintKit, int seed, float wear, std::string name)
{
	uint32_t steamid = g_SteamUser->GetSteamID().GetAccountID();

	ProtoWriter item(19);
	item.add(Field(CSOEconItem::id, TYPE_UINT64, (int64_t)(START_ITEM_INDEX + index)));
	item.add(Field(CSOEconItem::account_id, TYPE_UINT32, (int64_t)steamid));
	item.add(Field(CSOEconItem::def_index, TYPE_UINT32, (int64_t)itemIndex));
	item.add(Field(CSOEconItem::inventory, TYPE_UINT32, (int64_t)(START_ITEM_INDEX + index)));
	item.add(Field(CSOEconItem::origin, TYPE_UINT32, (int64_t)24));
	item.add(Field(CSOEconItem::quantity, TYPE_UINT32, (int64_t)1));
	item.add(Field(CSOEconItem::level, TYPE_UINT32, (int64_t)1));
	item.add(Field(CSOEconItem::style, TYPE_UINT32, (int64_t)0));
	item.add(Field(CSOEconItem::flags, TYPE_UINT32, (int64_t)0));
	item.add(Field(CSOEconItem::in_use, TYPE_BOOL, (int64_t)true));
	item.add(Field(CSOEconItem::original_id, TYPE_UINT64, (int64_t)0));
	item.add(Field(CSOEconItem::rarity, TYPE_UINT32, (int64_t)rarity));
	item.add(Field(CSOEconItem::quality, TYPE_UINT32, (int64_t)0));

	if (name.size() > 0)
		item.add(Field(CSOEconItem::custom_name, TYPE_STRING, name));

	// Equip new skins
	{
		TeamID avalTeam = GetAvailableClassID(itemIndex);

		if (avalTeam == TeamID::TEAM_SPECTATOR || avalTeam == TeamID::TEAM_TERRORIST) {
			ProtoWriter equipped_state(2);
			equipped_state.add(Field(CSOEconItemEquipped::new_class, TYPE_UINT32, (int64_t)TEAM_TERRORIST));
			equipped_state.add(Field(CSOEconItemEquipped::new_slot, TYPE_UINT32, (int64_t)GetSlotID(itemIndex)));
			item.add(Field(CSOEconItem::equipped_state, TYPE_STRING, equipped_state.serialize()));
		}
		if (avalTeam == TeamID::TEAM_SPECTATOR || avalTeam == TeamID::TEAM_COUNTER_TERRORIST) {
			ProtoWriter equipped_state(2);
			equipped_state.add(Field(CSOEconItemEquipped::new_class, TYPE_UINT32, (int64_t)TEAM_COUNTER_TERRORIST));
			equipped_state.add(Field(CSOEconItemEquipped::new_slot, TYPE_UINT32, (int64_t)GetSlotID(itemIndex)));
			item.add(Field(CSOEconItem::equipped_state, TYPE_STRING, equipped_state.serialize()));
		}
	}
	// Paint Kit
	float _PaintKitAttributeValue = (float)paintKit;
	auto PaintKitAttributeValue = std::string{ reinterpret_cast<const char*>((void*)&_PaintKitAttributeValue), 4 };
	ProtoWriter PaintKitAttribute(3);
	PaintKitAttribute.add(Field(CSOEconItemAttribute::def_index, TYPE_UINT32, (int64_t)6));
	PaintKitAttribute.add(Field(CSOEconItemAttribute::value_bytes, TYPE_STRING, PaintKitAttributeValue));
	item.add(Field(CSOEconItem::attribute, TYPE_STRING, PaintKitAttribute.serialize()));

	// Paint Seed
	float _SeedAttributeValue = (float)seed;
	auto SeedAttributeValue = std::string{ reinterpret_cast<const char*>((void*)&_SeedAttributeValue), 4 };
	ProtoWriter SeedAttribute(3);
	SeedAttribute.add(Field(CSOEconItemAttribute::def_index, TYPE_UINT32, (int64_t)7));
	SeedAttribute.add(Field(CSOEconItemAttribute::value_bytes, TYPE_STRING, SeedAttributeValue));
	item.add(Field(CSOEconItem::attribute, TYPE_STRING, SeedAttribute.serialize()));

	// Paint Wear
	float _WearAttributeValue = wear;
	auto WearAttributeValue = std::string{ reinterpret_cast<const char*>((void*)&_WearAttributeValue), 4 };
	ProtoWriter WearAttribute(3);
	WearAttribute.add(Field(CSOEconItemAttribute::def_index, TYPE_UINT32, (int64_t)8));
	WearAttribute.add(Field(CSOEconItemAttribute::value_bytes, TYPE_STRING, WearAttributeValue));
	item.add(Field(CSOEconItem::attribute, TYPE_STRING, WearAttribute.serialize()));

	// Stickers
	for (int j = 0; j < 4; j++)
	{
		// Sticker Kit
		ProtoWriter StickerKitAttribute(3);
		StickerKitAttribute.add(Field(CSOEconItemAttribute::def_index, TYPE_UINT32, (int64_t)(113 + 4 * j)));
		StickerKitAttribute.add(Field(CSOEconItemAttribute::value_bytes, TYPE_STRING, "\x00\x00\x00\x00"));
		item.add(Field(CSOEconItem::attribute, TYPE_STRING, StickerKitAttribute.serialize()));
		// Sticker Wear
		float _StickerWearAttributeValue = 0.001f;
		auto StickerWearAttributeValue = std::string{ reinterpret_cast<const char*>((void*)&_StickerWearAttributeValue), 4 };
		ProtoWriter StickerWearAttribute(3);
		StickerWearAttribute.add(Field(CSOEconItemAttribute::def_index, TYPE_UINT32, (int64_t)(114 + 4 * j)));
		StickerWearAttribute.add(Field(CSOEconItemAttribute::value_bytes, TYPE_STRING, StickerWearAttributeValue));
		item.add(Field(CSOEconItem::attribute, TYPE_STRING, StickerWearAttribute.serialize()));
		// Sticker Scale
		float _StickerScaleAttributeValue = 1.f;
		auto StickerScaleAttributeValue = std::string{ reinterpret_cast<const char*>((void*)&_StickerScaleAttributeValue), 4 };
		ProtoWriter StickerScaleAttribute(3);
		StickerScaleAttribute.add(Field(CSOEconItemAttribute::def_index, TYPE_UINT32, (int64_t)(115 + 4 * j)));
		StickerScaleAttribute.add(Field(CSOEconItemAttribute::value_bytes, TYPE_STRING, StickerScaleAttributeValue));
		item.add(Field(CSOEconItem::attribute, TYPE_STRING, StickerScaleAttribute.serialize()));
		// Sticker Rotation
		float _StickerRotationAttributeValue = 0.f;
		auto StickerRotationAttributeValue = std::string{ reinterpret_cast<const char*>((void*)&_StickerRotationAttributeValue), 4 };
		ProtoWriter StickerRotationAttribute(3);
		StickerRotationAttribute.add(Field(CSOEconItemAttribute::def_index, TYPE_UINT32, (int64_t)(116 + 4 * j)));
		StickerRotationAttribute.add(Field(CSOEconItemAttribute::value_bytes, TYPE_STRING, StickerRotationAttributeValue));
		item.add(Field(CSOEconItem::attribute, TYPE_STRING, StickerRotationAttribute.serialize()));
	}
	object.add(Field(SubscribedType::object_data, TYPE_STRING, item.serialize()));
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