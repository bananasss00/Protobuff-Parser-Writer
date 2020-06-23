#include "Protobuffs.h"
#include "main.h"
#include "Protobuffs\profile_info_changer.h"
#include "Protobuffs\inventory_changer.h"


#define CAST(cast, address, add) reinterpret_cast<cast>((uint32_t)address + (uint32_t)add)

void Protobuffs::WritePacket(std::string packet, void* thisPtr, void* oldEBP, void *pubDest, uint32_t cubDest, uint32_t *pcubMsgSize)
{
	if ((uint32_t)packet.size() <= cubDest - 8)
	{
		memcpy((void*)((DWORD)pubDest + 8), (void*)packet.data(), packet.size());
		*pcubMsgSize = packet.size() + 8;
	}
	else if (g_pMemAlloc)
	{
		auto memPtr = *CAST(void**, thisPtr, 0x18);
		auto memPtrSize = *CAST(uint32_t*, thisPtr, 0x1C);
		auto newSize = (memPtrSize - cubDest) + packet.size() + 8;

		auto memory = g_pMemAlloc->Realloc(memPtr, newSize + 4);

		*CAST(void**, thisPtr, 0x18) = memory;
		*CAST(uint32_t*, thisPtr, 0x1C) = newSize;
		*CAST(void**, oldEBP, -0x14) = memory;

		memcpy(CAST(void*, memory, 0x1C), (void*)packet.data(), packet.size());

		*pcubMsgSize = packet.size() + 8;
	}
}

static bool onceChanger = false;

void Protobuffs::ReceiveMessage(void* thisPtr, void* oldEBP, uint32_t messageType, void *pubDest, uint32_t cubDest, uint32_t *pcubMsgSize)
{
	if (messageType == k_EMsgGCCStrike15_v2_MatchmakingGC2ClientHello)
	{
		auto packet = profile_info_changer(pubDest, pcubMsgSize);
		WritePacket(packet, thisPtr, oldEBP, pubDest, cubDest, pcubMsgSize);
	}
  else if (messageType == k_EMsgGCCStrike15_v2_ClientGCRankUpdate)
  {
    CMsgGCCStrike15_v2_ClientGCRankUpdate msg((void*)((DWORD)pubDest + 8), *pcubMsgSize - 8);

    auto ranking = msg.ranking().get();

    if (ranking.rank_type_id().get() == 7) // wingman
    {
      ranking.rank_id().set(9);
      ranking.wins().set(999);
      msg.ranking().set(ranking);
      auto packet = msg.serialize();
      WritePacket(packet, thisPtr, oldEBP, pubDest, cubDest, pcubMsgSize);
    }
  }
	else if (messageType == k_EMsgGCClientWelcome)
	{
		auto packet = inventory_changer(pubDest, pcubMsgSize);
		WritePacket(packet, thisPtr, oldEBP, pubDest, cubDest, pcubMsgSize);
		
		if (!onceChanger) // idk why. all stuff not applied from first attempt
		{
		  SendClientHello();
		  SendMatchmakingClient2GCHello();
		  SendClientGcRankUpdate();
		  onceChanger = true;
		}
	}
	printf(".GC Receive: %d\n", messageType);
}

bool Protobuffs::PreSendMessage(uint32_t &unMsgType, void* pubData, uint32_t &cubData)
{
	uint32_t MessageType = unMsgType & 0x7FFFFFFF;

	if (MessageType == k_EMsgGCAdjustItemEquippedState)
	{
		return inventory_changer_presend(pubData, cubData);
	}
	printf(".GC Sent: %d\n", MessageType);
	return true;
}

///////////////////////////////////
/******** Custom Messages ********/
///////////////////////////////////
bool Protobuffs::SendClientHello()
{
	CMsgClientHello msg;
	msg.client_session_need().set(1);
	auto packet = msg.serialize();

	void* ptr = malloc(packet.size() + 8);

	if (!ptr)
		return false;

	((uint32_t*)ptr)[0] = k_EMsgGCClientHello | ((DWORD)1 << 31);
	((uint32_t*)ptr)[1] = 0;

	memcpy((void*)((DWORD)ptr + 8), (void*)packet.data(), packet.size());
	bool result = g_SteamGameCoordinator->GCSendMessage(k_EMsgGCClientHello | ((DWORD)1 << 31), ptr, packet.size() + 8) == k_EGCResultOK;
	free(ptr);

	return result;
}

bool Protobuffs::SendMatchmakingClient2GCHello()
{
	ProtoWriter msg(0);
	auto packet = msg.serialize();
	void* ptr = malloc(packet.size() + 8);

	if (!ptr)
		return false;

	((uint32_t*)ptr)[0] = k_EMsgGCCStrike15_v2_MatchmakingClient2GCHello | ((DWORD)1 << 31);
	((uint32_t*)ptr)[1] = 0;

	memcpy((void*)((DWORD)ptr + 8), (void*)packet.data(), packet.size());
	bool result = g_SteamGameCoordinator->GCSendMessage(k_EMsgGCCStrike15_v2_MatchmakingClient2GCHello | ((DWORD)1 << 31), ptr, packet.size() + 8) == k_EGCResultOK;
	free(ptr);

	return result;
}

bool Protobuffs::SendClientGcRankUpdate()
{
  MatchmakingGC2ClientHello::PlayerRankingInfo rank_wingman;
  rank_wingman.rank_type_id().set(7); // 6 - mm, 7 - wingman

  CMsgGCCStrike15_v2_ClientGCRankUpdate msg;
  msg.ranking().set(rank_wingman);

  auto packet = msg.serialize();

	void* ptr = malloc(packet.size() + 8);

	if (!ptr)
		return false;

  ((uint32_t*)ptr)[0] = k_EMsgGCCStrike15_v2_ClientGCRankUpdate | ((DWORD)1 << 31);
	((uint32_t*)ptr)[1] = 0;

	memcpy((void*)((DWORD)ptr + 8), (void*)packet.data(), packet.size());
	bool result = g_SteamGameCoordinator->GCSendMessage(k_EMsgGCCStrike15_v2_ClientGCRankUpdate | ((DWORD)1 << 31), ptr, packet.size() + 8) == k_EGCResultOK;
	free(ptr);

  return result;
}

bool Protobuffs::EquipWeapon(int weaponid, int classid, int slotid)
{
	CMsgAdjustItemEquippedState msg;
	msg.item_id().set(START_ITEM_INDEX + weaponid);
	msg.new_class().set(classid);
	msg.new_slot().set(slotid);
	msg.swap().set(true);
	auto packet = msg.serialize();

	void* ptr = malloc(packet.size() + 8);

	if (!ptr)
		return false;

	((uint32_t*)ptr)[0] = k_EMsgGCAdjustItemEquippedState | ((DWORD)1 << 31);
	((uint32_t*)ptr)[1] = 0;

	memcpy((void*)((DWORD)ptr + 8), (void*)packet.data(), packet.size());
	bool result = g_SteamGameCoordinator->GCSendMessage(k_EMsgGCAdjustItemEquippedState | ((DWORD)1 << 31), ptr, packet.size() + 8) == k_EGCResultOK;
	free(ptr);

	return result;
}

void Protobuffs::EquipAll()
{
	for (auto& x : g_skins) {
		auto def_index = (ItemDefinitionIndex)x.wId;
		EquipWeapon(def_index, TEAM_COUNTER_TERRORIST, GetSlotID(def_index));
		EquipWeapon(def_index, TEAM_TERRORIST, GetSlotID(def_index));
	}
}