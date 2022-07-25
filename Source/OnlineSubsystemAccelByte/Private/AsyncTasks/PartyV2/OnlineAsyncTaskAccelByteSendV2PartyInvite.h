// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlinePartyInterfaceAccelByte.h"

/**
 * Async Task for sending an invite to a user to join a party related session
 */
class FOnlineAsyncTaskAccelByteSendV2PartyInvite : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteSendV2PartyInvite(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FUniqueNetId& InRecipientId);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteSendV2PartyInvite");
	}

private:
	/** Name of the session to send an invite for */
	FName SessionName;

	/** ID of the user that will receive the invite as an AccelByte ID */
	TSharedRef<const FUniqueNetIdAccelByteUser> RecipientId;

	void OnSendPartyInviteSuccess();
	void OnSendPartyInviteError(int32 ErrorCode, const FString& ErrorMessage);
};

