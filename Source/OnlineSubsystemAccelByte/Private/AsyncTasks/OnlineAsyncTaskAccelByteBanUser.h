// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"

/**
 * Ban User to EQU8 service.
 *
 * For dedicated sessions, this requires the permission "ADMIN:NAMESPACE:{namespace}:ACTION" action=4 (UPDATE) on your server OAuth client.
 */
class FOnlineAsyncTaskAccelByteBanUser : public FOnlineAsyncTaskAccelByte
{
public:

	/** Constructor to setup the RegisterPlayers task */
	FOnlineAsyncTaskAccelByteBanUser(FOnlineSubsystemAccelByte* const InABInterface, const FString &InUserId, int32 InActionId, const FString &InMessage);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteBanUser");
	}

private:

	/** User ID to ban */
	FString UserId;

	/** Action ID to send to EQU8 service */
	int32 ActionId;

	/** Message to ban user */
	FString Message;
	
	void OnSuccess();
	void OnFailed(int code, const FString &message);
};
