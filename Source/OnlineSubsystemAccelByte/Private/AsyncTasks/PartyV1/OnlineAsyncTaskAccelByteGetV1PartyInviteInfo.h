// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteLobbyModels.h"
#include "OnlineUserCacheAccelByte.h"

/**
 * Fill out information about your async task here.
 */
class FOnlineAsyncTaskAccelByteGetV1PartyInviteInfo
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetV1PartyInviteInfo, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGetV1PartyInviteInfo(FOnlineSubsystemAccelByte* const InABInterface, const TSharedRef<const FUniqueNetIdAccelByteUser>& InUserId, const FAccelByteModelsPartyGetInvitedNotice& InNotification);

	virtual void Initialize() override;
	virtual void Finalize() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetPartyInviteInfo");
	}

private:

	/** Notification for the invite we received */
	FAccelByteModelsPartyGetInvitedNotice Notification;

	/** Information on the user that we retrieved either from cache or the backend */
	TSharedPtr<FAccelByteUserInfo> NotificationSenderInfo;

	/** Delegate handler for when we complete a query for joined party member information */
	void OnQueryNotificationSenderComplete(bool bIsSuccessful, TArray<TSharedRef<FAccelByteUserInfo>> UsersQueried);

};

