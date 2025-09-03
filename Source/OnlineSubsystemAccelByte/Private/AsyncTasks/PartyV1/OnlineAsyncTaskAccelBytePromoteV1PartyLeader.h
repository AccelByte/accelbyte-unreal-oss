// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once
#if 1 // MMv1 Deprecation

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlinePartyInterfaceAccelByte.h"

/**
 * Task to promote a member of a party to leader
 */
class FOnlineAsyncTaskAccelBytePromoteV1PartyLeader
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelBytePromoteV1PartyLeader, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelBytePromoteV1PartyLeader(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnlinePartyId& InPartyId, const FUniqueNetId& InTargetMemberId, const FOnPromotePartyMemberComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelBytePromotePartyLeader");
	}

private:

	/** ID of the party that the user is attempting to promote someone as leader of */
	TSharedRef<const FOnlinePartyIdAccelByte> PartyId;

	/** ID of the member that the user is attempting to promote */
	TSharedRef<const FUniqueNetIdAccelByteUser> TargetMemberId;

	/** Delegate to be fired after finishing the task to promote a party member */
	FOnPromotePartyMemberComplete Delegate;

	/** Result of the task to promote a party member to leader, sent to delegate */
	EPromoteMemberCompletionResult CompletionResult;

	/**
	 * Delegate handler for when we get a response from the backend on promoting a member of a party to leader
	 */
	void OnPromotePartyMemberResponse(const FAccelByteModelsPartyPromoteLeaderResponse& Result);

};

#endif