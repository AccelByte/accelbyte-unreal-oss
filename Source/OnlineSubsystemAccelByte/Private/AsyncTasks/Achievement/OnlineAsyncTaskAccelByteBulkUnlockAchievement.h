// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineAchievementsInterfaceAccelByte.h"
#include "InterfaceModels/OnlineAchievementInterfaceAccelByteModels.h"
#include "Interfaces/OnlineAchievementsInterface.h"

class FOnlineAsyncTaskAccelByteBulkUnlockAchievement
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteBulkUnlockAchievement, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteBulkUnlockAchievement(
		FOnlineSubsystemAccelByte* const Subsystem,
		FUniqueNetId const& UserID,
		bool bIsServer,
		FAccelByteModelsAchievementBulkUnlockRequest const& UnlockRequest,
		FOnBulkAchievementUnlockDelegate const& Delegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteBulkUnlockAchievement");
	}

private:
	void HandleBulkUnlockAchievementSuccess(TArray<FAccelByteModelsAchievementBulkUnlockRespone> const& Response);
	void HandleBulkUnlockAchievementError(int32 ErrorCode, FString const& ErrorMessage);

	// Whether this async task is called by game server or game client.
	bool bIsPerformedByServer = false;

	// Request to unlock achievement
	FAccelByteModelsAchievementBulkUnlockRequest UnlockRequest {};
	
	// Delegate that registered on the constructor & will be called after the task is done (Finalized).
	FOnBulkAchievementUnlockDelegate Delegate;

	// Hold the value of the response if success
	TArray<FAccelByteModelsAchievementBulkUnlockRespone> BulkUnlockResponse{};
};
