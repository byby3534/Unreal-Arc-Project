// Copyright

#include "Game/GameInitialization/CMNpcWorldSubsystem.h"
#include "Npc/CMNpcBase.h"

void UCMNpcWorldSubsystem::RegisterNpc(const FName& NpcId, ACMNpcBase* NpcActor)
{
	if (!NpcActor)
	{
		return;
	}

	FCMNpcCacheEntry& Entry = NpcCacheMap.FindOrAdd(NpcId);
	Entry.NpcId = NpcId;
	Entry.NpcActor = NpcActor;
}

void UCMNpcWorldSubsystem::UnregisterNpc(const FName& NpcId)
{
	NpcCacheMap.Remove(NpcId);
}

ACMNpcBase* UCMNpcWorldSubsystem::GetNpcById(const FName& NpcId) const
{
	if (const FCMNpcCacheEntry* Found = NpcCacheMap.Find(NpcId))
	{
		return Found->NpcActor.Get();
	}
	return nullptr;
}

void UCMNpcWorldSubsystem::GetAllNpcEntries(TArray<FCMNpcCacheEntry>& OutEntries) const
{
	OutEntries.Reset();
	NpcCacheMap.GenerateValueArray(OutEntries);
}

