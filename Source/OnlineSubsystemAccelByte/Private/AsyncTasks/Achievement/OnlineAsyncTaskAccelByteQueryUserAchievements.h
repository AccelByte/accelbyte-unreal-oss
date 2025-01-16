// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "InterfaceModels/OnlineAchievementInterfaceAccelByteModels.h"
#include "Interfaces/OnlineAchievementsInterface.h"
#include "InterfaceModels/OnlineUserInterfaceAccelByteModels.h"

class FOnlineAsyncTaskAccelByteQueryUserAchievements
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteQueryUserAchievements, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteQueryUserAchievements(
		FOnlineSubsystemAccelByte* const InABSubsystem,
		FUniqueNetId const& InPlayerId,
		FOnQueryAchievementsCompleteDelegate const& InDelegate);

	FOnlineAsyncTaskAccelByteQueryUserAchievements(
		FOnlineSubsystemAccelByte* const InABSubsystem,
		FUniqueNetId const& InPlayerId,
		FAccelByteQueryAchievementsParameters const& InRequestParameters,
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
	void QueryAchievement(int32 Offset, int32 Limit);
	void HandleQueryAchievementSuccess(FAccelByteModelsPaginatedUserAchievement const& Result);
	void HandleQueryAchievementError(int32 ErrorCode, FString const& ErrorMessage);

	TArray<FAccelByteModelsUserAchievement> UserAchievements{};
	FOnQueryAchievementsCompleteDelegate Delegate;

	FAccelByteQueryAchievementsParameters RequestParameters = {};
};
