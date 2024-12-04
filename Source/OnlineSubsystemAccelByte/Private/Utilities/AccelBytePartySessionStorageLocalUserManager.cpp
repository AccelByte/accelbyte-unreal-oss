// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Utilities/AccelBytePartySessionStorageLocalUserManager.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"

bool FAccelBytePartySessionStorageLocalUserManager::TryGetAccelByteUserIDFromUniqueNetIdPtr(FUniqueNetIdPtr UserNetId, FString& OutputResultAccelByteUserID)
{
	if (UserNetId == nullptr)
	{
		return false;
	}

	if (!UserNetId->IsValid())
	{
		return false;
	}

	const FUniqueNetIdAccelByteUserPtr CastUserAB = FUniqueNetIdAccelByteUser::CastChecked(UserNetId.ToSharedRef());
	if (!CastUserAB.IsValid())
	{
		return false;
	}

	const FString& AccelByteUserID = CastUserAB->GetAccelByteId();
	
	if (AccelByteUserID.IsEmpty())
	{
		return false;
	}

	OutputResultAccelByteUserID = AccelByteUserID;
	return true;
}

FAccelByteModelsV2PartySessionStorageReservedData FAccelBytePartySessionStorageLocalUserManager::ExtractCacheToWriteToPartyStorage(FUniqueNetIdPtr User)
{
	FString AccelByteUserID{};

	FAccelByteModelsV2PartySessionStorageReservedData Output;
	if (!TryGetAccelByteUserIDFromUniqueNetIdPtr(User, AccelByteUserID))
	{
		return Output;
	}
	Output.PastSessionIDs = PastSessionManager.GetPastSessionIDs(User);
	return Output;
}

const uint32& FAccelBytePastSessionRecordManager::GetMaxStoredSessionIdCount()
{
	return this->MaxStoredSessionIdCount;
}

void FAccelBytePastSessionRecordManager::SetMaxStoredSessionIdCount(uint32 MaxCount)
{
	MaxStoredSessionIdCount = MaxCount;
}

TArray<FString> FAccelBytePastSessionRecordManager::GetPastSessionIDs(FUniqueNetIdPtr User)
{
	FString AccelByteUserID{};
	if (!FAccelBytePartySessionStorageLocalUserManager::TryGetAccelByteUserIDFromUniqueNetIdPtr(User, AccelByteUserID))
	{
		return {};
	}

	if (!PastSessionIDs.Contains(AccelByteUserID))
	{
		return {};
	}

	return PastSessionIDs[AccelByteUserID];
}

void FAccelBytePastSessionRecordManager::InsertPastSessionID(FUniqueNetIdPtr User, const FString& SessionID)
{
	TArray<FString> SessionIDs;
	SessionIDs.Add(SessionID);
	InsertPastSessionID(User, SessionIDs);
};

void AppendArrayAsUniqueEntry(TArray<FString>& Source, const TArray<FString>& Addition)
{
	TSet<FString> Target = TSet<FString>(Source);
	TSet<FString> Copy = TSet<FString>(Addition);
	Target.Append(Copy);
	Source = Target.Array();
}

void FAccelBytePastSessionRecordManager::InsertPastSessionID(FUniqueNetIdPtr User, TArray<FString>& SessionIDs)
{
	FString AccelByteUserID{};
	if (!FAccelBytePartySessionStorageLocalUserManager::TryGetAccelByteUserIDFromUniqueNetIdPtr(User, AccelByteUserID))
	{
		return;
	}

	// Add the entry of UserID first if it doesn't exist
	if (!PastSessionIDs.Contains(AccelByteUserID))
	{
		auto Tuple = TTuple<FString, TArray<FString>>(AccelByteUserID, TArray<FString>());
		PastSessionIDs.Add(Tuple);
	}

	AppendArrayAsUniqueEntry(PastSessionIDs[AccelByteUserID], SessionIDs);

	while ((uint64)PastSessionIDs[AccelByteUserID].Num() > (uint64)MaxStoredSessionIdCount &&
		MaxStoredSessionIdCount != 0)
	{
		PastSessionIDs[AccelByteUserID].RemoveAt(0);
	}
};

void FAccelBytePastSessionRecordManager::RemoveSpecificCachedPastSessionIDs(FUniqueNetIdPtr User, const FString& SessionID)
{
	FString AccelByteUserID{};
	if (!FAccelBytePartySessionStorageLocalUserManager::TryGetAccelByteUserIDFromUniqueNetIdPtr(User, AccelByteUserID))
	{
		return;
	}

	if (PastSessionIDs.Contains(AccelByteUserID))
	{
		if (PastSessionIDs[AccelByteUserID].Contains(SessionID))
		{
			PastSessionIDs[AccelByteUserID].Remove(SessionID);
		}
	}
}

void FAccelBytePastSessionRecordManager::ResetCachedPastSessionIDs(FUniqueNetIdPtr User)
{
	FString AccelByteUserID{};
	if (!FAccelBytePartySessionStorageLocalUserManager::TryGetAccelByteUserIDFromUniqueNetIdPtr(User, AccelByteUserID))
	{
		return;
	}

	if (PastSessionIDs.Contains(AccelByteUserID))
	{
		PastSessionIDs[AccelByteUserID].Empty();
	}
};

void FAccelBytePastSessionRecordManager::ResetAllCachedPastSessionIDs()
{
	PastSessionIDs.Empty();
};
