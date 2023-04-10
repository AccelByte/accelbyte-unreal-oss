// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteQueryAchievement : public FOnlineAsyncTaskAccelByte,
                                                  public TSelfPtr<
	                                                  FOnlineAsyncTaskAccelByteQueryAchievement, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteQueryAchievement(
		FOnlineSubsystemAccelByte* const Subsystem,
		FUniqueNetId const& InPlayerId,
		FOnQueryAchievementsCompleteDelegate const& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryAchievement");
	}

private:
	void QueryAchievement();
	void HandleQueryAchievementSuccess(FAccelByteModelsPaginatedPublicAchievement const& Response);
	void HandleQueryAchievementError(int32 ErrorCode, FString const& ErrorMessage);

	TSharedPtr<FAccelByteModelsPaginatedPublicAchievement> PaginatedPublicAchievementPtr;
	FOnQueryAchievementsCompleteDelegate Delegate;
	bool bWasSuccessful;
};
