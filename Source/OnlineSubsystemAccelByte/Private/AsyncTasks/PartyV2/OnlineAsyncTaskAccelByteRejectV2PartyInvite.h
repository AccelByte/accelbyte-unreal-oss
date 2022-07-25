// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../OnlineAsyncTaskAccelByte.h"
#include "Models/AccelByteSessionModels.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionSettings.h"

/**
 * Async Task to reject a party session invite
 */
class FOnlineAsyncTaskAccelByteRejectV2PartyInvite : public FOnlineAsyncTaskAccelByte
{
public:
	FOnlineAsyncTaskAccelByteRejectV2PartyInvite(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnlineSessionSearchResult& InInvitedSession);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteRejectPartyInvite");
	}

private:
	/** Session that we were invited to that we want to reject the invite of */
	FOnlineSessionSearchResult InvitedSession;

	void OnRejectPartyInviteSuccess();
	void OnRejectPartyInviteError(int32 ErrorCode, const FString& ErrorMessage);
};

