// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "InterfaceModels/OnlineAchievementInterfaceAccelByteModels.h"
#include "Interfaces/OnlineAchievementsInterface.h"

class FOnlineAsyncTaskAccelByteQueryAchievement
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteQueryAchievement, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteQueryAchievement(
		FOnlineSubsystemAccelByte* const Subsystem,
		FUniqueNetId const& InPlayerId,
		FOnQueryAchievementsCompleteDelegate const& InDelegate);

	FOnlineAsyncTaskAccelByteQueryAchievement(
		FOnlineSubsystemAccelByte* const Subsystem,
		FUniqueNetId const& InPlayerId,
		FAccelByteQueryAchievementDescriptionParameters const& InRequestParameters,
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
	void QueryAchievement(int32 Offset, int32 Limit);
	void HandleQueryAchievementSuccess(FAccelByteModelsPaginatedPublicAchievement const& Response);
	void HandleQueryAchievementError(int32 ErrorCode, FString const& ErrorMessage);

	TArray<FAccelByteModelsPublicAchievement> PublicAchievements{};
	FOnQueryAchievementsCompleteDelegate Delegate;

	FAccelByteQueryAchievementDescriptionParameters RequestParameters = {};
};
