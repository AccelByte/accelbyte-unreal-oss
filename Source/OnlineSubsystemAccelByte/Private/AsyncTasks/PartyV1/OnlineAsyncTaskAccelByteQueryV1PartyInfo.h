// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineUserCacheAccelByte.h"
#include "Interfaces/OnlinePartyInterface.h"

struct FAccelBytePartyInfo
{
	/**
	 * Array containing information on each member of the party
	 */
	TArray<TSharedRef<FAccelByteUserInfo>> MemberInfo;

	/**
	 * Instance of data associated with this specific party
	 */
	TSharedRef<FOnlinePartyData> PartyData;

	FAccelBytePartyInfo()
		: MemberInfo()
		, PartyData(MakeShared<FOnlinePartyData>())
	{
	}
};

/**
 * Delegate fired once the FOnlineAsyncTaskAccelByteQueryPartyInfo task completes.
 * 
 * @param bWasSuccessful Boolean representing whether the task completed successfully or not
 * @param PartyInfo Structure containing the party information that was queried
 */
DECLARE_DELEGATE_TwoParams(FOnQueryPartyInfoComplete, bool /*bWasSuccessful*/, const FAccelBytePartyInfo& /*PartyInfo*/);

/**
 * Task to get basic information relating to a party, such as party member information and party storage.
 */
class FOnlineAsyncTaskAccelByteQueryV1PartyInfo
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteQueryV1PartyInfo, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteQueryV1PartyInfo(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FString& InPartyId, const TArray<FString>& InMembers, const FOnQueryPartyInfoComplete& InDelegate);

	FOnlineAsyncTaskAccelByteQueryV1PartyInfo(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId, const FString& InPartyId, const TArray<FString>& InMembers, const FOnQueryPartyInfoComplete& InDelegate);

    virtual void Initialize() override;
    virtual void Tick() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryPartyInfo");
	}

private:

	/**
	 * ID of the party that we wish to query extra information for
	 */
	FString PartyId;

	/**
	 * Array of IDs representing members of the party, should come from the party info or party join responses.
	 */
	TArray<FString> Members;

	/**
	 * Delegate fired once we get information for this party
	 */
	FOnQueryPartyInfoComplete Delegate;

	/**
	 * Flag denoting when we get a response back for querying basic party member info
	 */
	FThreadSafeBool bHasRetrievedPartyMemberInfo = false;

	/**
	 * Flag denoting when we get a response back for querying party storage
	 */
	FThreadSafeBool bHasRetrievedPartyStorage = false;

	/**
	 * Counter to determine if we have attempted to get customization information for all party members
	 */
	FThreadSafeCounter CustomizationQueriesRemaining;

	/**
	 * Counter to determine if we have attempted to get stats information for all party members
	 */
	FThreadSafeCounter StatsQueriesRemaining;

	/**
	* Counter to determine if we have attempted to get stats information for all party members
	*/
	FThreadSafeCounter ProgressionQueriesRemaining;

	/**
	* Counter to determine if we have attempted to get stats information for all party members
	*/
	FThreadSafeCounter DailyPlayStreakQueriesRemaining;

	/**
	* Counter to determine if we have attempted to get stats information for all party members
	*/
	FThreadSafeCounter RanksQueriesRemaining;

	/**
	 * Party info structure used to return to delegate
	 */
	FAccelBytePartyInfo PartyInfo;

	/**
	 * Check whether we have completed all work for our task
	 */
	bool HasFinishedAsyncWork();

	/** Delegate handler for when our request to query all party members completes */
	void OnQueryPartyMembersComplete(bool bIsSuccessful, TArray<TSharedRef<FAccelByteUserInfo>> UsersQueried);

	/** Delegate handler for when a request to get party storage data succeeds */
	void OnGetPartyStorageSuccess(const FAccelByteModelsPartyDataNotif& Result);

	/** Delegate handler for when a request to get party storage data fails */
	void OnGetPartyStorageError(int32 ErrorCode, const FString& ErrorMessage);
	
	/** Timeout handler */
	virtual void OnTaskTimedOut() override;
};

