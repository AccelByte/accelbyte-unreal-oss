// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlinePartyInterfaceAccelByte.h"

/**
 * Fill out information about your async task here.
 */
class FOnlineAsyncTaskAccelByteAddJoinedPartyMember : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteAddJoinedPartyMember(FOnlineSubsystemAccelByte* const InABInterface, const TSharedRef<const FUniqueNetIdAccelByteUser>& InLocalUserId, const TSharedRef<FOnlinePartyAccelByte>& InParty, const FString& InJoinedAccelByteId);

	virtual void Initialize() override;
	virtual void Tick() override;
	virtual void Finalize() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteAddJoinedPartyMember");
	}

	bool HasFinishedAsyncWork();

private:

	/** Reference to the local party object that the user joined */
	TSharedRef<FOnlinePartyAccelByte> Party;

	/** ID of the user that joined this party */
	FString JoinedAccelByteId;

	/** Information about the user that joined this party */
	TSharedPtr<FAccelByteUserInfo> JoinedPartyMember;

	/** Flag representing whether we have gotten basic member info for the user yet */
	FThreadSafeBool bHasRetrievedMemberInfo = false;

	/** Flag representing whether we have gotten statistics info for the user yet */
	FThreadSafeBool bHasRetrievedMemberStats = false;

	/** Flag representing whether we have gotten customization info for the user yet */
	FThreadSafeBool bHasRetrievedMemberCustomizations = false;

	/** Flag representing whether we have gotten user progression info for the user yet */
	FThreadSafeBool bHasRetrievedMemberProgression = false;

	/** Flag representing whether we have gotten user daily play streak info for the user yet */
	FThreadSafeBool bHasRetrievedMemberDailyPlayStreak = false;
	
	/** Flag representing whether we have gotten user rank info for the user yet */
	FThreadSafeBool bHasRetrievedMemberRanks = false;

	/** Delegate handler for when we complete a query for joined party member information */
	void OnQueryJoinedPartyMemberComplete(bool bIsSuccessful, TArray<TSharedRef<FAccelByteUserInfo>> UsersQueried);

};

