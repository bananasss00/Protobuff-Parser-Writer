#pragma once
#include "ProtoParse.h"
#include "ProtobuffMessages.h"

#define _gc2ch MatchmakingGC2ClientHello
#define _pci PlayerCommendationInfo
#define _pri PlayerRankingInfo

static std::string profile_info_changer(void *pubDest, uint32_t *pcubMsgSize) {
	MatchmakingGC2ClientHello msg((void*)((DWORD)pubDest + 8), *pcubMsgSize - 8);
	
	//replace commends
	auto commendation_new = msg.get_commendation<PlayerCommendationInfo>();
	commendation_new.replace_cmd_friendly(1000);
	commendation_new.replace_cmd_teaching(2000);
	commendation_new.replace_cmd_leader(3000);
	msg.replace_commendation(commendation_new.serialize());
	
	//replace ranks
	auto ranking_new = msg.get_ranking<PlayerRankingInfo>();
	ranking_new.replace_rank_id(9);
	ranking_new.replace_wins(999);
	msg.replace_ranking(ranking_new.serialize());
	
	//replace private level
	msg.replace_player_level(40);
	
	return msg.serialize();
}