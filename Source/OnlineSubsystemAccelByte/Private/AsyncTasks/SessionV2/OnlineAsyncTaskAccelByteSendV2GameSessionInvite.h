// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlinePartyInterfaceAccelByte.h"

/**
 * Async Task for sending an invite to a user to join a game related session
 */
class FOnlineAsyncTaskAccelByteSendV2GameSessionInvite
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteSendV2GameSessionInvite, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteSendV2GameSessionInvite(FOnlineSubsystemAccelByte* const InABInterface
		, const FUniqueNetId& InLocalUserId
		, const FName& InSessionName
		, const FUniqueNetId& InRecipientId);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteSendV2GameSessionInvite");
	}

private:
	/** Name of the session to send an invite for */
	FName SessionName;

	/** ID of the user that will receive the invite as an AccelByte ID */
	TSharedRef<const FUniqueNetIdAccelByteUser> RecipientId;

	FString SessionId;

	FVoidHandler OnSendGameSessionInviteSuccessDelegate;
	void OnSendGameSessionInviteSuccess();

	FErrorHandler OnSendGameSessionInviteErrorDelegate;
	void OnSendGameSessionInviteError(int32 ErrorCode, const FString& ErrorMessage);
};

