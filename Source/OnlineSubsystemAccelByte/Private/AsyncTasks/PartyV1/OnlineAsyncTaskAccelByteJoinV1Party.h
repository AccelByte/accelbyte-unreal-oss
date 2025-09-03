// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once
#if 1 // MMv1 Deprecation

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlinePartyInterfaceAccelByte.h"
#include "Models/AccelByteUserModels.h"
#include "Models/AccelByteLobbyModels.h"
#include "OnlineAsyncTaskAccelByteQueryV1PartyInfo.h"
#include "OnlineUserCacheAccelByte.h"

struct FAccelByteModelsUserStats;
/**
 * Async task to join a party from join info with an invite token
 */
class FOnlineAsyncTaskAccelByteJoinV1Party
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteJoinV1Party, ESPMode::ThreadSafe>
{
public:

	/**
	 * Constructor for joining a party through an invite
	 */
	FOnlineAsyncTaskAccelByteJoinV1Party(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const IOnlinePartyJoinInfo& InOnlinePartyJoinInfo, const FOnJoinPartyComplete& InDelegate);
	
	/**
	 * Constructor for joining a party through a party code
	 */
	FOnlineAsyncTaskAccelByteJoinV1Party(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InPartyCode, const FOnJoinPartyComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Tick() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteJoinParty");
	}

private:

	/** Information on the party that the user is joining */
	FOnlinePartyJoinInfoAccelByte OnlinePartyJoinInfo;

	/** Optional party code used to join a party, will be used depending on what path this task is taking */
	FString PartyCode;

	/** Delegate to be fired after party join is complete */
	FOnJoinPartyComplete Delegate;

	/* Used to track error code to be sent through FOnJoinPartyComplete*/
	int32 NotApprovedReason;

	/** Status on the completion of the join party task, forwarded to delegate fired */
	EJoinPartyCompletionResult CompletionResult;

	/** Array of party member IDs to their display names, used in Finalize to construct member instances */
	TArray<FAccelByteUserInfoRef> PartyMemberInfo;

	/** Information on the party joined from the backend, used in Finalize to set up party object */
	FAccelByteModelsPartyJoinResponse PartyInfo;

	/**
	 * Party data instance that will be initialized to the party object created in Finalize. We always want to have an
	 * instance of this associated with a party, even if there is no data to read.
	 */
	TSharedRef<FOnlinePartyData> PartyData;

	/** ID of the party that we joined, used to fire delegates */
	TSharedPtr<const FOnlinePartyIdAccelByte> JoinedPartyId;

	FAccelByteModelsBulkUserStatusNotif MemberStatuses;

	/** Delegate handler for when our request to check whether we are in a party gets a response */
	void OnGetPartyInfoResponse(const FAccelByteModelsInfoPartyResponse& Result);

	/** Delegate handler for when our request to accept an invite and join a party gets a response */
	void OnJoinPartyResponse(const FAccelByteModelsPartyJoinResponse& Result);

	/** Delegate handler for when our request to query all party info completes */
	void OnQueryPartyInfoComplete(bool bIsSuccessful, const FAccelBytePartyInfo& Result);

	/** Delegate handler for when we get a response back from the backend about leaving the party specified in this task */
	void OnLeavePartyResponse(const FAccelByteModelsLeavePartyResponse& Result);

	/** Gets a string representation of the information we are using to join a party, or blank if none is valid. Used for debug printing */
	FString GetJoinInfoString();

	/** Gets a formatted string to use for errors relating to party join */
	FString GetPartyInfoForError();

	void OnGetUserPresenceComplete(const FAccelByteModelsBulkUserStatusNotif& Statuses);
};
#endif