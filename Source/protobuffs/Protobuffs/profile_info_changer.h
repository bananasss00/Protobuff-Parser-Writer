#pragma once
#include "ProtoParse.h"
#include "ProtobuffMessages.h"

#define _gc2ch MatchmakingGC2ClientHello
#define _pci PlayerCommendationInfo
#define _pri PlayerRankingInfo

static std::string profile_info_changer(void *pubDest, uint32_t *pcubMsgSize) {
	ProtoWriter msg((void*)((DWORD)pubDest + 8), *pcubMsgSize - 8, _gc2ch::MAX_FIELD);
	
	//replace commends
	auto _commendation = msg.has(_gc2ch::commendation) ? msg.get(_gc2ch::commendation).String() : std::string("");
	ProtoWriter commendation(_commendation, _pci::MAX_FIELD);
	commendation.replace(_pci::cmd_friendly, 1000);
	commendation.replace(_pci::cmd_teaching, 2000);
	commendation.replace(_pci::cmd_leader, 3000);
	msg.replace(_gc2ch::commendation, commendation.serialize());
	
	//replace ranks
	auto _ranking = msg.has(_gc2ch::ranking) ? msg.get(_gc2ch::ranking).String() : std::string("");
	ProtoWriter ranking(_ranking, _pri::MAX_FIELD);
	ranking.replace(_pri::rank_id, 9);
	ranking.replace(_pri::wins, 999);
	msg.replace(_gc2ch::ranking, ranking.serialize());
	
	//replace private level
	msg.replace(_gc2ch::player_level, 40);
	
	return msg.serialize();
}