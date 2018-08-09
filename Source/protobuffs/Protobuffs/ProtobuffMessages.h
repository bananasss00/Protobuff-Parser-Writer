#pragma once

#define k_EMsgGCCStrike15_v2_MatchmakingGC2ClientReserve 9107
#define k_EMsgGCClientWelcome 4004
#define k_EMsgGCClientHello 4006
#define k_EMsgGCAdjustItemEquippedState 1059
#define k_EMsgGCCStrike15_v2_MatchmakingClient2GCHello 9109
#define k_EMsgGCCStrike15_v2_MatchmakingGC2ClientHello 9110

//FORMAT: constexpr static Tag field_name = {field_id, field_type};

struct CMsgClientHello {
	constexpr static size_t MAX_FIELD = 8;
	constexpr static Tag client_session_need = {3, TYPE_UINT32};
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
struct SubscribedType {
	constexpr static size_t MAX_FIELD = 2;
	constexpr static Tag type_id = {1, TYPE_INT32};
	constexpr static Tag object_data = {2, TYPE_STRING};
};

struct CMsgSOCacheSubscribed {
	constexpr static size_t MAX_FIELD = 4;
	constexpr static Tag objects = {2, TYPE_STRING};
};

struct CMsgClientWelcome {
	constexpr static size_t MAX_FIELD = 11;
	constexpr static Tag outofdate_subscribed_caches = {3, TYPE_STRING};
};

struct CSOEconItem {
	constexpr static size_t MAX_FIELD = 19;
	constexpr static Tag id = {1, TYPE_UINT64};
	constexpr static Tag account_id = {2, TYPE_UINT32};
	constexpr static Tag inventory = {3, TYPE_UINT32};
	constexpr static Tag def_index = {4, TYPE_UINT32};
	constexpr static Tag quantity = {5, TYPE_UINT32};
	constexpr static Tag level = {6, TYPE_UINT32};
	constexpr static Tag quality = {7, TYPE_UINT32};
	constexpr static Tag flags = {8, TYPE_UINT32};
	constexpr static Tag origin = {9, TYPE_UINT32};
	constexpr static Tag custom_name = {10, TYPE_STRING};
	constexpr static Tag attribute = {12, TYPE_STRING};
	constexpr static Tag in_use = {14, TYPE_BOOL};
	constexpr static Tag style = {15, TYPE_UINT32};
	constexpr static Tag original_id = {16, TYPE_UINT64};
	constexpr static Tag equipped_state = {18, TYPE_STRING};
	constexpr static Tag rarity = {19, TYPE_UINT32};
};

struct CMsgAdjustItemEquippedState {
	constexpr static size_t MAX_FIELD = 4;
	constexpr static Tag item_id = {1, TYPE_UINT64};
	constexpr static Tag new_class = {2, TYPE_UINT32};
	constexpr static Tag new_slot = {3, TYPE_UINT32};
	constexpr static Tag swap = {4, TYPE_BOOL};
};

struct CSOEconItemEquipped {
	constexpr static size_t MAX_FIELD = 2;
	constexpr static Tag new_class = {1, TYPE_UINT32};
	constexpr static Tag new_slot = {2, TYPE_UINT32};
};

struct CSOEconItemAttribute {
	constexpr static size_t MAX_FIELD = 3;
	constexpr static Tag def_index = {1, TYPE_UINT32};
	constexpr static Tag value = {2, TYPE_UINT32};
	constexpr static Tag value_bytes = {3, TYPE_STRING};
};