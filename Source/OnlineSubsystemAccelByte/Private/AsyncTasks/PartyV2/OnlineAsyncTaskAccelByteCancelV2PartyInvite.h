// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteCancelV2PartyInvite
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteCancelV2PartyInvite, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteCancelV2PartyInvite(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FUniqueNetId& InInvitee);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteCancelV2PartyInvite");
	}

private:
	/** Name of the session. */
	FName SessionName;

	/** ID of invitee. */
	TSharedRef<const FUniqueNetIdAccelByteUser> InviteeId;

	/** Handler for cancel party invite success. */
	void OnCancelPartyInviteSuccess();

	/** Handler for cancel party invite fail. */
	void OnCancelPartyInviteError(const int32 ErrorCode, const FString& ErrorMessage);

	/** Text representing the error that occurred in the request, if one did. */
	FText ErrorText;

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;
};
