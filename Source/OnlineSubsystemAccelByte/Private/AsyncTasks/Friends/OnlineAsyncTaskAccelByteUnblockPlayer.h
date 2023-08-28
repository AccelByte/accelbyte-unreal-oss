// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteLobbyModels.h"

/**
 * Task for unblocking a player on the AccelByte backend.
 */
class FOnlineAsyncTaskAccelByteUnblockPlayer
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteUnblockPlayer, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteUnblockPlayer(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FUniqueNetId& InPlayerId);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUnblockPlayer");
	}

private:

	/** Id of the user that we wish to unblock */
	TSharedRef<const FUniqueNetIdAccelByteUser> PlayerId;

	/** String representing errors encountered while executing this task, passed to delegate */
	FString ErrorStr;

	/** Delegate handler for when the request to unblock a player is complete */
	void OnUnblockPlayerResponse(const FAccelByteModelsUnblockPlayerResponse& Result);

};

