// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteLobbyModels.h"
#include "OnlinePartyInterfaceAccelByte.h"

/**
 * Fill out information about your async task here.
 */
class FOnlineAsyncTaskAccelByteKickV1PartyMember : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteKickV1PartyMember, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteKickV1PartyMember(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnlinePartyId& InPartyId, const FUniqueNetId& InTargetMemberId, const FOnKickPartyMemberComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteKickPartyMember");
	}

private:

	/** ID of the party that we wish to kick a member from */
	TSharedRef<const FOnlinePartyIdAccelByte> PartyId;
	
	/** ID of the user that we wish to attempt to kick from our party */
	TSharedRef<const FUniqueNetIdAccelByteUser> TargetMemberId;

	/** Delegate fired after our task to kick a member finishes */
	FOnKickPartyMemberComplete Delegate;

	/** Result of the task to kick a party member, sent to delegate */
	EKickMemberCompletionResult CompletionResult;

	/** Delegate handler for when we get a response back from the backend about kicking a party member */
	void OnKickPartyMemberResponse(const FAccelByteModelsKickPartyMemberResponse& Result);
};

