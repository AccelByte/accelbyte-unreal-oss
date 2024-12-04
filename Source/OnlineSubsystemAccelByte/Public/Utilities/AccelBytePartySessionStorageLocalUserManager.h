// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Set.h"
#include "OnlineSubsystemAccelByteTypes.h"

/*
 * This struct collect user's past game session and cache it locally.
 * This user's past game session record will be propagated to the party using reserved party storage.
 * Then, the party leader could read party member's past session record and ask the matchmaking service to EXCLUDE those sessions from backfill matchmaking.
 * 
 * Caution:
 * This entry is modified by the OSS automatically. But if there is a specific need, the record can be modified to remove specific sessions or add specific session.
 */
struct ONLINESUBSYSTEMACCELBYTE_API FAccelBytePastSessionRecordManager
{
	TArray<FString> GetPastSessionIDs(FUniqueNetIdPtr UserNetId);
	void InsertPastSessionID(FUniqueNetIdPtr UserNetId, const FString& SessionID);
	void InsertPastSessionID(FUniqueNetIdPtr UserNetId, TArray<FString>& SessionID);
	void RemoveSpecificCachedPastSessionIDs(FUniqueNetIdPtr UserNetId, const FString& SessionID);
	void ResetCachedPastSessionIDs(FUniqueNetIdPtr UserNetId);
	void ResetAllCachedPastSessionIDs();

	const uint32& GetMaxStoredSessionIdCount();
	void SetMaxStoredSessionIdCount(uint32 MaxCount);

private:
	uint32 MaxStoredSessionIdCount = 5;
	TMap<FString /*AccelByteUserID*/, TArray<FString /*SessionID*/>> PastSessionIDs{};
};

/*
 * Member of FOnlineSessionV2AccelByte OSS Interface.
 * This struct collect current local users' entry that will be send & overwritten to its reserved slot in the Party Session's Storage.
 * Each user is eligible to write it's own reserved storage.
 * Currently, the reserved storage only consists of users' past session record
 */
struct ONLINESUBSYSTEMACCELBYTE_API FAccelBytePartySessionStorageLocalUserManager
{
	static bool TryGetAccelByteUserIDFromUniqueNetIdPtr(FUniqueNetIdPtr UserNetId, FString& OutputResultAccelByteUserID);

	/*
	 * Manage user's past game session record.
	 */
	FAccelBytePastSessionRecordManager PastSessionManager{};

	/*
	 * Extract user's cached past session from PastSessionManager automatically into a struct that ready to send to backend service.
	 * 
	 * @param UserNetId The user that want to retrieve the game session record.
	 * @return The struct that ready to sent.
	 */
	FAccelByteModelsV2PartySessionStorageReservedData ExtractCacheToWriteToPartyStorage(FUniqueNetIdPtr UserNetId);
};