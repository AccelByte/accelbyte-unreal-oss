// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Interfaces/OnlineAchievementsInterface.h"

class FOnlineAsyncTaskAccelByteQueryUserAchievements : public FOnlineAsyncTaskAccelByte,
                                                       public TSelfPtr<FOnlineAsyncTaskAccelByteQueryUserAchievements,
                                                                ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteQueryUserAchievements(
		FOnlineSubsystemAccelByte* const InABSubsystem,
		FUniqueNetId const& InPlayerId,
		FOnQueryAchievementsCompleteDelegate const& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryAchievements");
	}

private:
	void QueryAchievement();
	void HandleQueryAchievementSuccess(FAccelByteModelsPaginatedUserAchievement const& Result);
	void HandleQueryAchievementError(int32 ErrorCode, FString const& ErrorMessage);

	TSharedPtr<FAccelByteModelsPaginatedUserAchievement> PaginatedUserAchievements;
	FOnQueryAchievementsCompleteDelegate Delegate;
	bool bWasSuccessful;
};
