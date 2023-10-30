// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Models/AccelByteSessionModels.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionSettings.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

/**
 * Async Task to reject a party session invite
 */
class FOnlineAsyncTaskAccelByteRejectV2PartyInvite
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteRejectV2PartyInvite, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteRejectV2PartyInvite(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnlineSessionSearchResult& InInvitedSession, const FOnRejectSessionInviteComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteRejectV2PartyInvite");
	}

private:
	/** Session that we were invited to that we want to reject the invite of */
	FOnlineSessionSearchResult InvitedSession{};

	/** Delegate fired when we finish rejecting an invite */
	FOnRejectSessionInviteComplete Delegate{};

	FVoidHandler OnRejectPartyInviteSuccessDelegate;
	FErrorHandler OnRejectPartyInviteErrorDelegate;
	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES(RejectPartyInvite);
};

