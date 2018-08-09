#pragma once
#include "ProtoParse.h"
#include "ProtobuffMessages.h"

#define _gc2ch MatchmakingGC2ClientHello
#define _pci PlayerCommendationInfo
#define _pri PlayerRankingInfo

static std::string profile_info_changer(void *pubDest, uint32_t *pcubMsgSize) {
	MatchmakingGC2ClientHello msg((void*)((DWORD)pubDest + 8), *pcubMsgSize - 8);
	
	//replace commends
	PlayerCommendationInfo commendation_new( msg.get_commendation().String() );
	commendation_new.replace_cmd_friendly(1000);
	commendation_new.replace_cmd_teaching(2000);
	commendation_new.replace_cmd_leader(3000);
	msg.replace_commendation(commendation_new.serialize());
	
	//replace ranks
	PlayerRankingInfo ranking_new(msg.get_ranking().String());
	ranking_new.replace_rank_id(9);
	ranking_new.replace_wins(999);
	msg.replace_ranking(ranking_new.serialize());
	
	//replace private level
	msg.replace_player_level(40);
	
	return msg.serialize();
}