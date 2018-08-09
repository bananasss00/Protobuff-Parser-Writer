#pragma once

#define k_EMsgGCCStrike15_v2_MatchmakingGC2ClientReserve 9107
#define k_EMsgGCClientWelcome 4004
#define k_EMsgGCClientHello 4006
#define k_EMsgGCAdjustItemEquippedState 1059
#define k_EMsgGCCStrike15_v2_MatchmakingClient2GCHello 9109
#define k_EMsgGCCStrike15_v2_MatchmakingGC2ClientHello 9110

//FORMAT: constexpr static Tag field_name = {field_id, field_type};

make_struct(CMsgClientHello, 8)
	make_field(client_session_need, 3, TYPE_UINT32)
};


// ProfileChanger
make_struct(MatchmakingGC2ClientHello, 20)
	make_field(ranking, 7, TYPE_STRING)
	make_field(commendation, 8, TYPE_STRING)
	make_field(player_level, 17, TYPE_INT32)
};
make_struct(PlayerCommendationInfo, 4)
	make_field(cmd_friendly, 1, TYPE_UINT32)
	make_field(cmd_teaching, 2, TYPE_UINT32)
	make_field(cmd_leader, 4, TYPE_UINT32)
};
make_struct(PlayerRankingInfo, 6)
	make_field(rank_id, 2, TYPE_UINT32)
	make_field(wins, 4, TYPE_UINT32)
};



// InvChanger
make_struct(SubscribedType, 2)
	make_field(type_id, 1, TYPE_INT32)
	make_field(object_data, 2, TYPE_STRING)
};
make_struct(CMsgSOCacheSubscribed, 4)
	make_field(objects, 2, TYPE_STRING)
};
make_struct(CMsgClientWelcome, 11)
	make_field(outofdate_subscribed_caches, 3, TYPE_STRING)
};
make_struct(CSOEconItem, 19)
	make_field(id, 1, TYPE_UINT64)
	make_field(account_id, 2, TYPE_UINT32)
	make_field(inventory, 3, TYPE_UINT32)
	make_field(def_index, 4, TYPE_UINT32)
	make_field(quantity, 5, TYPE_UINT32)
	make_field(level, 6, TYPE_UINT32)
	make_field(quality, 7, TYPE_UINT32)
	make_field(flags, 8, TYPE_UINT32)
	make_field(origin, 9, TYPE_UINT32)
	make_field(custom_name, 10, TYPE_STRING)
	make_field(attribute, 12, TYPE_STRING)
	make_field(in_use, 14, TYPE_BOOL)
	make_field(style, 15, TYPE_UINT32)
	make_field(original_id, 16, TYPE_UINT64)
	make_field(equipped_state, 18, TYPE_STRING)
	make_field(rarity, 19, TYPE_UINT32)
};
make_struct(CMsgAdjustItemEquippedState, 4)
	make_field(item_id, 1, TYPE_UINT64)
	make_field(new_class, 2, TYPE_UINT32)
	make_field(new_slot, 3, TYPE_UINT32)
	make_field(swap, 4, TYPE_BOOL)
};
make_struct(CSOEconItemEquipped, 2)
	make_field(new_class, 1, TYPE_UINT32)
	make_field(new_slot, 2, TYPE_UINT32)
};
make_struct(CSOEconItemAttribute, 3)
	make_field(def_index, 1, TYPE_UINT32)
	make_field(value, 2, TYPE_UINT32)
	make_field(value_bytes, 3, TYPE_STRING)
};