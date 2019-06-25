#pragma once
#include "ProtoParse.h"
#include "ProtobuffMessages.h"

static std::string profile_info_changer(void *pubDest, uint32_t *pcubMsgSize) {
	MatchmakingGC2ClientHello msg((void*)((DWORD)pubDest + 8), *pcubMsgSize - 8);
	
	//replace commends
  MatchmakingGC2ClientHello::PlayerCommendationInfo commendations;
  commendations.cmd_friendly().set(1000);
  commendations.cmd_teaching().set(2000);
  commendations.cmd_leader().set(3000);
  msg.commendation().set(commendations);

	//replace ranks
  MatchmakingGC2ClientHello::PlayerRankingInfo ranking;
  ranking.account_id().set(g_SteamUser->GetSteamID().GetAccountID());
	ranking.rank_id().set(9);
	ranking.wins().set(999);
  msg.ranking().set(ranking);

	//replace private level
	msg.player_level().set(40);
	
	return msg.serialize();
}