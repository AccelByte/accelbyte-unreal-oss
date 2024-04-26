// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineAchievementsInterfaceAccelByte.h"
#include "Models/AccelByteAchievementModels.h"

class FOnlineAsyncTaskAccelByteSendPSNEvents
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteSendPSNEvents, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteSendPSNEvents(FOnlineSubsystemAccelByte* const InABSubsystem
		, const FAccelByteModelsAchievementBulkCreatePSNEventRequest& InRequest
		, const FOnSendPSNEventsCompleteDelegate& InCompletionDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteSendPSNEvents");
	}

private:
	FAccelByteModelsAchievementBulkCreatePSNEventRequest Request{};

	FAccelByteModelsAchievementBulkCreatePSNEventResponse Response{};

	FOnSendPSNEventsCompleteDelegate Delegate{};

	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(CreatePSNEvent, FAccelByteModelsAchievementBulkCreatePSNEventResponse)

};
